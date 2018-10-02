#ifndef DODOU2_SERVER_H
#define DODOU2_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sysCall_facade.h>

#include "pduCommon.h"
#include "pduCreator.h"

typedef struct {
    char* serverPort;
    char* serverName;
    uint8_t* nameServerIP;
    char* nameServerPort;
} serverInputArgs;

void parseServerArgs(int argc, char** argv, serverInputArgs* args);

int establishConnectionWithNs(serverInputArgs *args);

void server_main(int argc, char** argv);

void fillInAddrInfo(struct addrinfo** addrInfo, const int port, const char* IPAddress, int flags);

#endif //DODOU2_SERVER_H
