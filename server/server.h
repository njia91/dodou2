#ifndef DODOU2_SERVER_H
#define DODOU2_SERVER_H

#include "pduReader.h"
#include "nameServerConnection.h"
#include "helpers.h"

void parseServerArgs(int argc, char** argv, serverInputArgs* args);

void server_main(int argc, char** argv);

void fillInAddrInfo(struct addrinfo** addrInfo, const int port, const char* IPAddress, int flags);

#endif //DODOU2_SERVER_H
