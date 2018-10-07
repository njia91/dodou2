#ifndef DODOU2_HELPERS_H
#define DODOU2_HELPERS_H

#include <stdio.h>
#include <memory.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    char* serverPort;
    char* serverName;
    uint8_t* nameServerIP;
    char* nameServerPort;
} serverInputArgs;

void fillInAddrInfo(struct addrinfo **addrInfo, const int port, const char *IPAddress, int socketType, int flags);

#endif //DODOU2_HELPERS_H
