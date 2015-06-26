#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HOST_NM_LENG 128 /* name length */
#define HOST_IP_LENG 128 /* ip length */

int main(int argc, char **argv)
{
    char hostnm[HOST_NM_LENG];
    char hostip[HOST_IP_LENG];
    int  port, i;
    int  count;

    if (argc != 5) {
        fprintf(stderr, "./gennodelist <hostname> <IP> <port> <count>\n");
        return -1;
    }

    strncpy(hostnm, argv[1], HOST_NM_LENG);
    strncpy(hostip, argv[2], HOST_IP_LENG);
    port = atoi(argv[3]);
    count = atoi(argv[4]);

    for (i = 0; i < count; i++) {
        printf("%s:%d-%s\n", hostip, port+i, hostnm);
    }
    return 0;
}
