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
  args->serverName = calloc(strlen(argv[2]) + 1, sizeof(char));
  memcpy(args->serverName, argv[2], strlen(argv[2]));

  args->nameServerIP = calloc(strlen(argv[3]) + 1, sizeof(uint8_t));
  strcpy((char *) args->nameServerIP, argv[3]);
  args->nameServerPort = calloc(PORT_LENGTH, sizeof(uint8_t));
  memcpy(args->nameServerPort, argv[4], PORT_LENGTH - 1);
}

void handleJoin(pduJoin *join, int socket_fd) {
  fprintf(stdout, "Received JOIN message\n");
  char* clientID = calloc(join->idSize + 1, sizeof(char));
  memcpy(clientID, join->id, join->idSize);

  participant par;
  par.clientID = calloc(strlen(clientID) + 1, sizeof(char));
  memcpy(par.clientID, clientID, strlen(clientID));
  par.socket_fd = socket_fd;

  fprintf(stdout, "%s added to participants\n", par.clientID);
  participantList[currentFreeParticipantSpot] = par;
  currentFreeParticipantSpot++;

  uint8_t numberOfParticipants = (uint8_t)(currentFreeParticipantSpot);
  pduParticipants participants;
  participants.opCode = PARTICIPANTS;
  participants.ids = calloc(numberOfParticipants, sizeof(char *));
  participants.noOfIds = numberOfParticipants;
  for (uint8_t i = 0; i < currentFreeParticipantSpot; i++) {
    participants.ids[i] = calloc(strlen(participantList[i].clientID) + 1, sizeof(char));
    memcpy(participants.ids[i], participantList[i].clientID, strlen(participantList[i].clientID));
  }

  // Send participants list to client
  size_t dataSize;
  uint8_t *data = pduCreator_participants(&participants, &dataSize);
  fprintf(stdout, "Sending participants, noP:%d\n", participants.noOfIds);
  facade_write(socket_fd, data, dataSize);

  for (uint8_t i = 0; i < currentFreeParticipantSpot; i++) {
    free(participants.ids[i]);
  }
  free(participants.ids);

  free(data);

  pduPJoin pJoin;
  pJoin.opCode = PJOIN;
  pJoin.idSize = (uint8_t) strlen(clientID);
  pJoin.id = calloc(pJoin.idSize, sizeof(char));
  memcpy(pJoin.id, clientID, pJoin.idSize);
  struct timeval time;
  gettimeofday(&time, NULL); // TODO: wrong time
  pJoin.timeStamp = (uint32_t) time.tv_usec;

  size_t bufferSize;
  uint8_t *buffer = pduCreator_pJoin(&pJoin, &bufferSize);

  for (uint8_t i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd != participantList[i].socket_fd) {
      facade_write(participantList[i].socket_fd, buffer, bufferSize);
    }
  }

  free(pJoin.id);
  free(join->id);
  free(buffer);

  free(clientID);
}

void handleMess(pduMess *mess, int socket_fd) {
  fprintf(stdout, "Received a message\n");

  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      mess->idSize = (uint8_t) strlen(participantList[i].clientID);
      //mess->id = calloc(mess->idSize, sizeof(char));
      memcpy(mess->id, participantList[i].clientID, mess->idSize);
      struct timeval time;
      gettimeofday(&time, NULL); // TODO: wrong time
      mess->timeStamp = (uint32_t) time.tv_usec;
    }
  }

  size_t bufferSize;
  uint8_t *buffer = pduCreator_mess(mess, &bufferSize);

  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd != participantList[i].socket_fd) {
      facade_write(participantList[i].socket_fd, buffer, bufferSize);
    }
  }
  free(mess->id);
  free(mess->message);
  free(mess);
  free(buffer);
}

void handleQuit(pduQuit *quit, int socket_fd) {
  pduPLeave leave;
  leave.opCode = PLEAVE;
  struct timeval time;
  gettimeofday(&time, NULL);
  leave.timeStamp = (uint32_t) time.tv_usec;

  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      fprintf(stdout, "%s have left the chat\n", participantList[i].clientID);

      leave.idSize = (uint8_t) strlen(participantList[i].clientID);
      leave.id = calloc(leave.idSize, sizeof(char));
      memcpy(leave.id, participantList[i].clientID, leave.idSize);
      // Remove from list
      free(participantList[i].clientID);
      participantList[i] = participantList[--currentFreeParticipantSpot];
      break;
    }
  }

  size_t bufferSize;
  uint8_t *buffer = pduCreator_pLeave(&leave, &bufferSize);

  free(leave.id);

  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    facade_write(participantList[i].socket_fd, buffer, bufferSize);
  }

  free(buffer);
  // TODO: Remove from epoll as well
  close(socket_fd);
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
    sData->numOfActiveFds++;
    fprintf(stdout, "Added client to epoll: %d, numberOfEpoll:%d\n", client_fd, sData->numOfActiveFds);
  } else {
    // Client socket
    genericPdu *p = getDataFromSocket(socket_fd);
    if (p == NULL){
      return false;
    }

    if (p->opCode == JOIN) {
      handleJoin((pduJoin *)p, socket_fd);
    } else if (p->opCode == MESS) {
      handleMess((pduMess *)p, socket_fd);
    } else if (p->opCode == QUIT) {
      handleQuit((pduQuit *)p, socket_fd);
    } else {
      fprintf(stderr, "Received unhandled message with OP Code: %d\n", p->opCode);
    }

    free(p);
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

  currentFreeParticipantSpot = 0;

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