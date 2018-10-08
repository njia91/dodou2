#include "server.h"

void parseServerArgs(int argc, char **argv, serverInputArgs *args) {
  if (argc <= 4) {
    fprintf(stderr, "Too few or too many Arguments \n"
                    "[Port] [Server name] [Nameserver IP Adress] [Nameserver Port]\n");
    exit(EXIT_FAILURE);
  }
  args->serverPort = argv[1];
  args->serverName = calloc(strlen(argv[2]), sizeof(char));
  memcpy(args->serverName, argv[2], strlen(argv[2]));

  args->nameServerIP = calloc(strlen(argv[3]) + 1, sizeof(uint8_t));
  strcpy((char *) args->nameServerIP, argv[3]);
  args->nameServerPort = calloc(PORT_LENGTH, sizeof(uint8_t));
  memcpy(args->nameServerPort, argv[4], PORT_LENGTH - 1);
}

bool processSocketData(int socket_fd, void *args) {
  bool allOk = true;
  serverData *sData = (serverData *)args;

  if (socket_fd == sData->server_fd) {
    // Server socket
    fprintf(stdout, "ServerFD have info...\n");
    int client_fd = listenForIncomingConnection(socket_fd);
    facade_setToNonBlocking(client_fd);
    fprintf(stdout, "ServerFD info read, new socket created:%d\n", client_fd);
    // Add Client
    struct epoll_event ev_client;
    ev_client.data.fd = client_fd;
    ev_client.events = EPOLLIN | EPOLLONESHOT;
    int result = facade_epoll_ctl(sData->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client);
    fprintf(stdout, "Added client to epoll: %d\n", result);
  } else {
    // Client socket
    genericPdu *p = getDataFromSocket(socket_fd);
    if (p == NULL){
      return false;
    }

    if (p->opCode == JOIN) {
      pduJoin *join = (pduJoin *)p;
      fprintf(stdout, "Client ID length: %d\n", join->idSize);
      char* clientID = calloc(join->idSize, sizeof(char));
      memcpy(clientID, join->id, join->idSize);
      fprintf(stdout, "Client ID: %s\n", clientID);
      free(clientID);

      // TODO: save client

      // TODO: send PARTICIPANTS

    }

    fprintf(stdout, "OP Code from client: %d\n", p->opCode);
  }
}

void server_main(int argc, char **argv) {
  serverInputArgs args;

  parseServerArgs(argc, argv, &args);

  int nameServerSocket = establishConnectionWithNs(args);

  fprintf(stdout, "Socket value: %d\n", nameServerSocket);

  registerToServer(nameServerSocket, args);


  int epoll_fd = epoll_create1(0);


  int server_fd = setupServerSocket(args);

  // Setup Epoll and add server
  struct epoll_event ev_server;
  ev_server.data.fd = server_fd;
  ev_server.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev_server);


  facade_setToNonBlocking(server_fd);

  serverData sData;
  sData.server_fd = server_fd;
  sData.numOfActiveFds = 1; // Just the server
  sData.epoll_fd = epoll_fd;

  readerInfo rInfo;
  rInfo.args = &sData;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;
  rInfo.numOfActiveFds = 1; // Only counting the Server socket.

  pthread_t receivingThread;
  int ret = pthread_create(&receivingThread, NULL, waitForIncomingMessages, (void *)&rInfo);
  if (ret){
    fprintf(stderr, "Unable to create a pthread. Error: %d\n", ret);
  }

  bool running = true;
  while (running) {
    if (gotACKResponse(nameServerSocket)) {
      fprintf(stdout, "Still connected to server\n");
    } else {
      registerToServer(nameServerSocket, args);
      fprintf(stdout, "Lost contact with name server, connecting again\n");
    }
    sleep(8);
  }
  pthread_join(receivingThread, NULL);

  free(args.nameServerIP);
  free(args.serverName);
  free(args.nameServerPort);
}