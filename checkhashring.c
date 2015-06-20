#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PROTOTYPES 1
#include "rfc1321/md5c.c"
#undef  PROTOTYPES

#define MAX_NODE_COUNT 200
#define NUM_OF_HASHES 40
#define NUM_PER_HASH  4

static char *node_array[MAX_NODE_COUNT];
static int   node_count;

struct server_item {
    char *hostport;
};

struct continuum_item {
    uint32_t index;    // server index
    uint32_t point;    // point on the ketama continuum
};

static void hash_md5(const char *key, size_t key_length, unsigned char *result)
{
    MD5_CTX ctx;

    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char *)key, key_length);
    MD5Final(result, &ctx);
}

uint32_t ketama_hash(const char *key, size_t key_length)
{
    unsigned char digest[16];

    hash_md5(key, key_length, digest);
    return (uint32_t)((digest[3] << 24)
                     |(digest[2] << 16)
                     |(digest[1] << 8)
                     | digest[0]);
}

static int continuum_item_cmp(const void *t1, const void *t2)
{
    const struct continuum_item *ct1 = t1, *ct2 = t2;

    if (ct1->point == ct2->point)      return 0;
    else if (ct1->point  > ct2->point) return  1;
    else                               return -1;
}

#define MAX_SERVER_ITEM_COUNT 100
static int ketama_continuum_generate(const struct server_item *servers, int num_servers,
                                     struct continuum_item **continuum, int *continuum_len)
{
    char host[MAX_SERVER_ITEM_COUNT+10] = "";
    int host_len;
    int pp, hh, ss, nn;
    unsigned char digest[16];

    // 40 hashes, 4 numbers per hash = 160 points per server
    int points_per_server = NUM_OF_HASHES * NUM_PER_HASH;

    *continuum = calloc(points_per_server * num_servers, sizeof(struct continuum_item));
    if (*continuum == NULL) {
        fprintf(stderr, "calloc failed: continuum\n");
        return -1;
    }

    for (ss=0, pp=0; ss<num_servers; ss++) {
        for (hh=0; hh<NUM_OF_HASHES; hh++) {
            host_len = snprintf(host, MAX_SERVER_ITEM_COUNT+10, "%s-%u",
                                servers[ss].hostport, hh);
            hash_md5(host, host_len, digest);
            for (nn=0; nn<NUM_PER_HASH; nn++, pp++) {
                (*continuum)[pp].index = ss;
                (*continuum)[pp].point = ((uint32_t) (digest[3 + nn * NUM_PER_HASH] & 0xFF) << 24)
                                       | ((uint32_t) (digest[2 + nn * NUM_PER_HASH] & 0xFF) << 16)
                                       | ((uint32_t) (digest[1 + nn * NUM_PER_HASH] & 0xFF) <<  8)
                                       | (           (digest[0 + nn * NUM_PER_HASH] & 0xFF)      );
            }
        }
    }

    qsort(*continuum, pp, sizeof(struct continuum_item), continuum_item_cmp);
    *continuum_len = pp;
    return 0;
}

static void server_item_free(struct server_item *servers, int num_servers)
{
    int i;
    for (i=0; i<num_servers; i++) {
        free(servers[i].hostport);
    }
}

static int server_item_populate(char **server_list, int num_servers,
                                struct server_item **servers)
{
    char *hostport;
    char  buf[128];
    char *tok = NULL;
    int i;

    *servers = calloc(num_servers, sizeof(struct server_item));
    if (*servers == NULL) {
        fprintf(stderr, "calloc failed: servers\n");
        return -1;
    }

    // filter characters after dash(-)
    for (i=0; i<num_servers; i++) {
        strcpy(buf, server_list[i]); 
        tok = strtok(buf, "-");
        if (tok == NULL || (hostport = strdup(tok)) == NULL) {
            fprintf(stderr, "invalid server token\n");
            server_item_free(*servers, i);
            free(*servers);
            *servers = NULL;
        }
        (*servers)[i].hostport = hostport;
    }
    return 0;
}

static void _print_node_list(struct server_item *servers, int num_servers)
{
    int i;
    fprintf(stderr, "cluster node list: count=%d\n", num_servers);
    for (i = 0; i < num_servers; i++) {
        fprintf(stderr, "node[%02d]: %s (org=%s)\n", i, servers[i].hostport, node_array[i]);
    }
}

static void _print_continuum(struct continuum_item *continuum, int num_continuum)
{
    int i;
    fprintf(stderr, "cluster continuum: count=%d\n", num_continuum);
    for (i = 0; i < num_continuum; i++) {
        fprintf(stderr, "continuum[%04d]: hash=%08x, sidx=%02d\n",
                i, continuum[i].point, continuum[i].index);
    }
}

static void _check_continuum(struct continuum_item *continuum, int num_continuum)
{
    int i;
    fprintf(stderr, "check cluster continuum begin: count=%d\n", num_continuum);
    for (i = 1; i < num_continuum; i++) {
        if (continuum[i].point < continuum[i-1].point) {
            fprintf(stderr, "hash[%d]=%08x is smaller than the previous hash=%08x\n",
                    i, continuum[i].point, continuum[i-1].point);
            break;
        }
        if (continuum[i].point == continuum[i-1].point) {
            fprintf(stderr, "hash[%d]=%08x is equal to the previous hash.\n",
                    i, continuum[i].point);
            break;
        } 
    }
    fprintf(stderr, "check cluster continuum end: %s\n",
            (i == num_continuum ? "success" : "fail")); 
}

int main(int argc, char **argv)
{
    FILE *fp;
    char *filename = "./arcus_node_list";
    char *hostport;
    char  buffer[256];
    struct server_item *servers = NULL;
    struct continuum_item *continuum = NULL;
    int i, num_continuum = 0;

    fp = fopen(filename, "r");
    assert(fp);

    node_count = 0;
    while ((fgets(buffer, sizeof(buffer), fp)) != NULL) {
        /* fprintf(stderr, "LINE[%03d]: %s\n", node_count+1, buffer); */
        buffer[strlen(buffer)-1] = '\0'; /* remove new line character*/    
        hostport = strdup(buffer);
        assert(hostport);
        node_array[node_count++] = hostport;
    }

    fclose(fp);

    if (server_item_populate(node_array, node_count, &servers) != 0) {
        fprintf(stderr, "server_item_populate failed.\n");
        exit(-1);
    }

    if (ketama_continuum_generate(servers, node_count, &continuum, &num_continuum) != 0) {
        fprintf(stderr, "ketama_continuum_generate failed.\n");
        exit(-1);
    }

    _print_node_list(servers, node_count);
    _print_continuum(continuum, num_continuum);
    _check_continuum(continuum, num_continuum);

    server_item_free(servers, node_count);
    free(servers);
    free(continuum);

    for (i = 0; i < node_count; i++)
        free(node_array[i]);
    node_count = 0;

    return 0;
}
