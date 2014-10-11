#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define SYSLOGD_PORT    514     // UDP syslog port #

int main()
{
    char      zkip[50] = "127.0.0.1"; /* ZK server IP */
    char      myip[50] = "";
    char      host[100] = "";
    char     *hostp=NULL;
    int       sock, flags, rc=0;
    socklen_t socklen=16;
    struct in_addr     inaddr;
    struct sockaddr_in saddr;
    struct sockaddr_in myaddr;
    struct hostent     *hp;

    fprintf(stderr, "\n");
    fprintf(stderr, "get hostname using gethostname()\n");
    fprintf(stderr, "================================\n");

    rc = gethostname((char *) &host, sizeof(host));
    if (rc == 0) {
        fprintf(stderr, "hostname: %s\n", host);
    } else {
        fprintf(stderr, "cannot get hostname: %s\n", strerror(errno));
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "get hostname using gethostbyaddr()\n");
    fprintf(stderr, "================================\n");

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        fprintf(stderr, "socket() failed: %s\n", strerror(errno));
        _exit(-1);
    }
    do {
        if (!inet_aton(zkip, &inaddr)) {
            fprintf(stderr, "Invalid zkip : %s\n", zkip);
            rc=-1; break;
        }
        saddr.sin_family = AF_INET;
        saddr.sin_addr   = inaddr;
        saddr.sin_port   = htons(SYSLOGD_PORT); // syslogd port. what if this is not open? XXX

        flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
        rc = connect(sock, (struct sockaddr*) &saddr, sizeof(struct sockaddr_in));
        fcntl(sock, F_SETFL, flags);
        if (rc != 0) {
            fprintf(stderr, "cannot connect : %s\n", strerror(errno));
            break;
        }

        if (getsockname(sock, (struct sockaddr *)&myaddr, &socklen)) {
            fprintf(stderr, "getsockname failed: %s\n", strerror(errno));
            rc=-1; break;
        }
        if (1) {
            inet_ntop(AF_INET, &myaddr.sin_addr, myip, sizeof(myip));
            fprintf(stderr, "local IP is %s\n", myip);
        }
        hp = gethostbyaddr((char*)&myaddr.sin_addr.s_addr, sizeof(myaddr.sin_addr.s_addr), AF_INET);
        if (hp) {
            hostp = strdup(hp->h_name);
            fprintf(stderr, "hostname: %s\n", hostp);
        } else {
            fprintf(stderr, "cannot get hostname using gethostbyaddr\n");
        } 
    } while(0);
    close(sock);

    return 0;
}
