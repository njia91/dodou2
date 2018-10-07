#include "helpers.h"

void fillInAddrInfo(struct addrinfo **addrInfo, const int port, const char *IPAddress, int socketType, int flags) {
    char portId[15];
    sprintf(portId, "%d", port);
    struct addrinfo info;
    memset(&info,0,sizeof(info));
    info.ai_family = AF_UNSPEC;
    info.ai_socktype = socketType;
    info.ai_protocol = 0;
    info.ai_flags = flags;

    if (getaddrinfo(IPAddress, portId , &info, addrInfo) != 0) {
        fprintf(stderr, "Failed to get address info\n");
        exit(EXIT_FAILURE);
    }
}