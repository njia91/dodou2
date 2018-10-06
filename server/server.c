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

void server_main(int argc, char** argv) {
    serverInputArgs args;

    parseServerArgs(argc, argv, &args);

    int nameServerSocket = establishConnectionWithNs(&args);

    fprintf(stdout, "Socket value: %d\n", nameServerSocket);

    registerToServer(nameServerSocket, args);

    bool running = true;
    while (running) {
        if (!gotACKResponse(nameServerSocket)) {
            registerToServer(nameServerSocket, args);
            fprintf(stdout, "Lost contact with name server, connecting again\n");
        } else {
            fprintf(stdout, "Still connected to server\n");
        }
        sleep(8);
    }

    free(args.nameServerIP);
    free(args.serverName);
    free(args.nameServerPort);
}