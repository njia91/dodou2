#include <pduReader.h>
#include "server.h"

void parseServerArgs(int argc, char** argv, serverInputArgs* args) {
    if (argc <= 4) {
        fprintf(stderr, "Too few or too many Arguments \n"
                        "[Port] [Server name] [Nameserver IP Adress] [Nameserver Port]\n");
        exit(EXIT_FAILURE);
    }
    args->serverPort = argv[1];
    args->serverName = calloc(strlen(argv[2]), sizeof(char));
    memcpy(args->serverName, argv[2], strlen(argv[2]));

    args->nameServerIP = calloc(strlen(argv[3]) + 1, sizeof(uint8_t));
    strcpy((char *)args->nameServerIP, argv[3]);
    args->nameServerPort = calloc(PORT_LENGTH, sizeof(uint8_t));
    memcpy(args->nameServerPort, argv[4], PORT_LENGTH - 1);

}
void fillInAddrInfo(struct addrinfo** addrInfo, const int port, const char* IPAddress, int flags) {
    char portId[15];
    sprintf(portId, "%d", port);
    struct addrinfo info;
    memset(&info,0,sizeof(info));
    info.ai_family = AF_UNSPEC;
    info.ai_socktype = SOCK_STREAM;
    info.ai_protocol = 0;
    info.ai_flags = flags;

    if (getaddrinfo(IPAddress, portId , &info, addrInfo) != 0) {
        fprintf(stderr, "Failed to get address info\n");
        exit(EXIT_FAILURE);
    }

}
int establishConnectionWithNs(serverInputArgs* args) {
    int nameserver_fd = 0;
    struct addrinfo* res = 0;

    fillInAddrInfo(&res, atoi(args->nameServerPort), args->nameServerIP, AI_ADDRCONFIG);

    nameserver_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (nameserver_fd == -1) {
        freeaddrinfo(res);
        fprintf(stderr, "FAILED TO CREATE SOCKET\n");
    }


    // Used to prioritize packets from this socket.
    int optval = 6;
    if (setsockopt(nameserver_fd, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(int)) == -1) {
        close(nameserver_fd);
        fprintf(stderr, "Failed to prioritize packets\n");
        exit(EXIT_FAILURE);
    }

    // Try to connect to Server. Retry if unsuccessful.
    while(connect(nameserver_fd, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "Unable to connect, retrying in 1 sec...");

        sleep(1);
    }
    freeaddrinfo(res);

    return nameserver_fd;/*




    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_protocol=0;
    hints.ai_flags=AI_ADDRCONFIG;

    int ret = facade_getAddrinfo((char *) args->nameServerIP, args->nameServerPort, &hints, &res);

    if (ret != 0) {
        fprintf(stderr, "%s \n",gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    nameserver_fd = facade_createSocket(&res);

    if (nameserver_fd == -1){
        fprintf(stderr, "Unable to setup socket to nameserver.\n");
        exit(EXIT_FAILURE);
    }

    ret = facade_connect(nameserver_fd, &res);
    if (ret == -1){
        fprintf(stderr, "Could not connect to NameServer %s \n", strerror(errno));
    }

    facade_freeaddrinfo(res);
    return nameserver_fd;*/
}

void server_main(int argc, char** argv) {
    serverInputArgs args;

    parseServerArgs(argc, argv, &args);

    int socket = establishConnectionWithNs(&args);

    fprintf(stdout, "Socket value: %d\n", socket);

    pduReg registerMessage;
    registerMessage.opCode = REG;
    registerMessage.serverName = (uint8_t*) args.serverName;
    registerMessage.serverNameSize = (uint8_t) strlen(args.serverName);
    registerMessage.tcpPort = (uint16_t) atoi(args.serverPort);

    size_t registerBufferSize;
    uint8_t* registerBuffer = pduCreator_reg(&registerMessage, &registerBufferSize);


    fprintf(stdout, "Sending REG to nameserver (%s) (%d)\n", registerBuffer, (int)registerBufferSize);
//    facade_write(socket, registerBuffer, registerBufferSize);
    send(socket, registerBuffer, registerBufferSize, 0);

    fprintf(stdout, "Receiving ACK from nameserver... ");

    char buffer[1024] = {0};
    ssize_t valread = read(socket, buffer, 1024);
    //genericPdu* ackData = getDataFromSocket(socket);

   // if (ackData->opCode == ACK) {
     //   fprintf(stdout, "Success!!\n");

    //}

}