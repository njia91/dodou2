#include "nameServerConnection.h"

int establishConnectionWithNs(serverInputArgs* args) {
    int nameServer_fd = 0;
    struct addrinfo* res = 0;

    fillInAddrInfo(&res, atoi(args->nameServerPort), args->nameServerIP, AI_ADDRCONFIG);

    nameServer_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (nameServer_fd == -1) {
        freeaddrinfo(res);
        fprintf(stderr, "FAILED TO CREATE SOCKET\n");
    }


    // Used to prioritize packets from this socket.
    int optval = 6;
    if (setsockopt(nameServer_fd, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(int)) == -1) {
        close(nameServer_fd);
        fprintf(stderr, "Failed to prioritize packets\n");
        exit(EXIT_FAILURE);
    }

    // Try to connect to Server. Retry if unsuccessful.
    while(connect(nameServer_fd, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "Unable to connect, retrying in 1 sec...");

        sleep(1);
    }
    freeaddrinfo(res);

    return nameServer_fd;
}

void registerToServer(int sock, serverInputArgs args) {
    pduReg registerMessage;
    registerMessage.opCode = REG;
    registerMessage.serverName = (uint8_t*) args.serverName;
    registerMessage.serverNameSize = (uint8_t) strlen(args.serverName);
    registerMessage.tcpPort = (uint16_t) atoi(args.serverPort);
    registerMessage.ipAddress[0] = 90;
    registerMessage.ipAddress[1] = 231;
    registerMessage.ipAddress[2] = 78;
    registerMessage.ipAddress[3] = 221;

    size_t registerBufferSize;
    uint8_t* registerBuffer = pduCreator_reg(&registerMessage, &registerBufferSize);

    fprintf(stdout, "Sending REG to name server\n");
    ssize_t res = facade_write(sock, registerBuffer, registerBufferSize);
    free(registerBuffer);
}

bool gotACKResponse(int nameServerSocket) {
    genericPdu* data = getUdpDataFromSocket(nameServerSocket);

    if (data->opCode == ACK) {
        pduAck *ack = (pduAck *) data;

        pduAlive aliveMessage;
        aliveMessage.opCode = ALIVE;
        aliveMessage.noOfClients = 0;
        aliveMessage.id = ack->id;

        free(data);

        size_t aliveBufferSize;
        uint8_t *aliveBuffer = pduCreator_alive(&aliveMessage, &aliveBufferSize);

        ssize_t res = facade_write(nameServerSocket, aliveBuffer, aliveBufferSize);
        free(aliveBuffer);
    } else if (data->opCode == NOTREQ) {
        return false;
    }
    return true;
}