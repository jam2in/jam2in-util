#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static char *local_hostname = "localhost";
static char *local_ip = "127.0.0.1";

int main(int argc, char **argv)
{
    char      filenam[128];
    char      svccode[128];
    char      content[128];
    FILE      *fp;
    int       start; /* start port number */
    int       count; /* node count */
    int       i, rc;
    
    if (argc != 3) {
      fprintf(stderr, "Usage) genarcuslocalconf <port> <count>\n");
      return -1;
    }
    start = atoi(argv[1]);
    count = atoi(argv[2]);

    sprintf(svccode, "%dnode", count);
    sprintf(filenam, "./local.%s.json", svccode);

    fp = fopen(filenam, "w");
    assert(fp); 

    sprintf(content, "{\n");
    rc = fputs(content, fp); assert(rc != 0); 
    sprintf(content, "\t\"serviceCode\": \"%s\",\n", svccode);
    rc = fputs(content, fp); assert(rc != 0); 
    sprintf(content, "\t\"servers\": [\n");
    rc = fputs(content, fp); assert(rc != 0); 
    for (i = 0; i < (count-1); i++) {
        sprintf(content, "\t\t{ \"hostname\": \"%s\", \"ip\": \"%s\", \"config\": { \"port\": \"%d\" } },\n",
                local_hostname, local_ip, start+i);
        rc = fputs(content, fp); assert(rc != 0); 
    }
    sprintf(content, "\t\t{ \"hostname\": \"%s\", \"ip\": \"%s\", \"config\": { \"port\": \"%d\" } }\n",
            local_hostname, local_ip, start+i);
    rc = fputs(content, fp); assert(rc != 0); 
    sprintf(content, "\t],\n");
    rc = fputs(content, fp); assert(rc != 0); 
    sprintf(content, "\t\"config\": { \"threads\" : \"1\", \"memlimit\" : \"10\", \"connections\" : \"10\"}\n");
    rc = fputs(content, fp); assert(rc != 0); 
    sprintf(content, "}\n");
    rc = fputs(content, fp); assert(rc != 0); 

    fclose(fp);
    return 0;
}
