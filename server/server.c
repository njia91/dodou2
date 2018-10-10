#include "server.h"


void freeParticipant(void *id) {
  free((char *)id);
}

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

      pduParticipants participants;
      participants.ids = calloc(1, sizeof(char *));
      participants.ids[0] = calloc(1, sizeof(char) * strlen(clientID));
      memcpy(participants.ids[0], clientID, strlen(clientID));
      participants.noOfIds = 1;
      participants.opCode = PARTICIPANTS;


      // TODO: save client
      //dll_insert(clientID, 0, participantsList);

      // Send participants list to client
      size_t dataSize;
      uint8_t *data = pduCreator_participants(&participants, &dataSize);
      facade_write(socket_fd, data, dataSize);


      free(participants.ids[0]);
      free(participants.ids);
      free(clientID);
    } else if (p->opCode == MESS) {
      fprintf(stdout, "Received a message\n");
      // TODO: Implement message handling
    } else if (p->opCode == PLEAVE) {
      fprintf(stdout, "Participant leaved\n");
      // TODO: Implement leaving handling
    } else {
      fprintf(stderr, "Received unhandled message with OP Code: %d\n", p->opCode);
    }
  }
}

void server_main(int argc, char **argv) {
  serverInputArgs args;

  parseServerArgs(argc, argv, &args);

  int nameServerSocket = establishConnectionWithNs(args);

  fprintf(stdout, "Socket value: %d\n", nameServerSocket);

  registerToServer(nameServerSocket, args);

  participantsList = dll_empty();
  dll_setMemoryHandler(participantsList, &freeParticipant);


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