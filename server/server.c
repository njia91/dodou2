#include "server.h"

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

/**
 * Add a client to the participants list
 * @param socket_fd The clients socket
 * @param clientID The clients ID
 */
void addToParticipantsList(int socket_fd, char *clientID) {
  participant par;
  par.clientID = calloc(strlen(clientID) + 1, sizeof(char));
  memcpy(par.clientID, clientID, strlen(clientID));
  par.socket_fd = socket_fd;

  fprintf(stdout, "%s added to participants\n", par.clientID);
  participantList[currentFreeParticipantSpot] = par;
  currentFreeParticipantSpot++;
}

/**
 * When a client have joined the server, this can be used to send a list
 * of all current participants to the client.
 * @param socket_fd The socket of the client who should receive the participants list
 */
void sendParticipantsListToClient(int socket_fd) {
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
}

/**
 * Will do the following:
 * - Send the participants list to the new client
 * - Check if the server is full. If it is, notify the new client and close the connection
 * - If the server is not full, add the new client to the participants list
 * - Notify other participants that a new client have joined
 * @param join The join message
 * @param socket_fd The new clients socket
 * @return If everything was ok
 */
bool handleJoin(pduJoin *join, int socket_fd) {
  fprintf(stdout, "Received JOIN message\n");

  char* clientID = calloc(join->idSize + 1, sizeof(char));
  memcpy(clientID, join->id, join->idSize);

  sendParticipantsListToClient(socket_fd);

  if (currentFreeParticipantSpot >= UINT8_MAX) {
    char messageString[] = "Server full";
    pduMess mess;
    mess.opCode = MESS;
    mess.message = calloc(strlen(messageString), sizeof(char));
    memcpy(mess.message, messageString, strlen(messageString));
    mess.messageSize = (uint16_t) strlen(messageString);
    mess.idSize = 0;
    mess.timeStamp = 0;
    mess.id = NULL;
    // TODO: add mess.timeStamp;

    size_t bufferSize;
    // Tell new client server is full
    uint8_t *buffer = pduCreator_mess(&mess, &bufferSize);
    facade_write(socket_fd, buffer, bufferSize);

    free(mess.message);
    free(buffer);

    buffer = pduCreator_quit(&bufferSize);
    // Tell client to close connection
    facade_write(socket_fd, buffer, bufferSize);
    free(buffer);

    free(clientID);
    close(socket_fd);

    return true;
  }

  addToParticipantsList(socket_fd, clientID);

  notifyClientsNewClientJoined(socket_fd, clientID);

  free(clientID);
  return true;
}

/**
 * Will notify all participants that a new client have joined the server.
 * Will not notify the new client.
 * @param socket_fd Socket of the new client
 * @param clientID ID of the new client
 */
void notifyClientsNewClientJoined(int socket_fd, char *clientID) {
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
  free(buffer);
}

/**
 * Will send a message to everyone except one
 * @param mess The message to send
 * @param socket_fd The client's socket to not send to
 * @return If everything is ok
 */
bool handleMess(pduMess *mess, int socket_fd) {
  fprintf(stdout, "Received a message\n");

  // Add more info to the message
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      mess->idSize = (uint8_t) strlen(participantList[i].clientID);
      memcpy(mess->id, participantList[i].clientID, mess->idSize);
      struct timeval time;
      gettimeofday(&time, NULL); // TODO: wrong time
      mess->timeStamp = (uint32_t) time.tv_usec;
    }
  }

  // Send the message to everyone except the sender
  size_t bufferSize;
  uint8_t *buffer = pduCreator_mess(mess, &bufferSize);
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd != participantList[i].socket_fd) {
      facade_write(participantList[i].socket_fd, buffer, bufferSize);
    }
  }
  free(buffer);
  return true;
}

/**
 * Notify all clients in the participants that a client have left.
 * Except for the leaving client, it does not receive a message.
 * @param socket_fd The socket of the leaving client
 * @return If all is ok
 */
bool handleQuit(int socket_fd) {
  // Prepare a leave message
  pduPLeave leave;
  leave.opCode = PLEAVE;
  struct timeval time;
  gettimeofday(&time, NULL);
  leave.timeStamp = (uint32_t) time.tv_usec; // TODO: This is wrong, use better timestamp

  // Find the id of the leaving client
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      fprintf(stdout, "%s have left the chat\n", participantList[i].clientID);
      // Add the client id to the message
      leave.idSize = (uint8_t) strlen(participantList[i].clientID);
      leave.id = calloc(leave.idSize, sizeof(char));
      memcpy(leave.id, participantList[i].clientID, leave.idSize);
      // Remove the client from the participants list
      free(participantList[i].clientID);
      participantList[i] = participantList[--currentFreeParticipantSpot];
      break;
    }
  }

  size_t bufferSize;
  uint8_t *buffer = pduCreator_pLeave(&leave, &bufferSize);
  free(leave.id);

  // Notify all in participants list that a client have left
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    facade_write(participantList[i].socket_fd, buffer, bufferSize);
  }

  free(buffer);
  // TODO: Remove from epoll as well??

  close(socket_fd);
  return true;
}

/**
 *
 * @param sData
 * @return
 */
bool readInputFromUser(serverData *sData) {
  size_t buffSize = 0;
  char *buffer = NULL;
  bool active = true;
  ssize_t ret = 0;
  fflush(stdin);

  if (getline(&buffer, &buffSize, stdin) == -1){
    fprintf(stderr, "Failed to read data from user: %s\n", strerror(errno));
    active = false;
  } else {
    if (strcmp(buffer, "QUIT\n") == 0){
      // TODO: Notify all clients that serer is shutting down.
      // TODO: Shut down server gracefully.
      // TODO: Notify other thread that we are shutting down.
//      active = false;
//      uint8_t *pduBuffer = pduCreator_quit(&buffSize);
//      ret = facade_write(sData->server_fd, pduBuffer, buffSize);
//      free(pduBuffer);
//      if (ret != buffSize){
//        fprintf(stderr, "Unable to write all data to socket.\n");
//      }
    } else {
      // TODO: Allow server to send system messages to all clients
      // TODO: Allow server to kick clients??
      // TODO: More nice functionality?
//      // Prepare Message PDU
//      pduMess mess;
//      size_t size = 0;
//      mess.opCode = MESS;
//      mess.id = NULL;
//      mess.idSize =  0;
//      mess.message = (uint8_t *) buffer;
//      mess.messageSize = (uint16_t) strlen(buffer);
//      mess.timeStamp = 0;
//
//      uint8_t *packet = pduCreator_mess(&mess, &size);
//      // Send package
//      ret = facade_write(sData->server_fd, packet, size);
//      free(packet);
//      if (ret != size){
//        fprintf(stderr, "%s: Unable to write all data to socket. Size %zd  ret %zd\n",__func__, size, ret);
//        fprintf(stderr, "Errno : %s \n", strerror(errno));
//        if (errno == EBADF){
//          active = false;
//        }
//      }
    }
  }
  free(buffer);
  return active;
}

/**
 * Processes the data that is read from a socket
 * @param socket_fd The socket to read data from
 * @param args Important data
 * @return If all is ok
 */
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
    int result = facade_epoll_ctl(sData->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client); // TODO: Handle result, set allOk = false
    sData->numOfActiveFds++;
    fprintf(stdout, "Added client to epoll: %d, numberOfEpoll:%d\n", client_fd, sData->numOfActiveFds);
  }  else if (socket_fd == STDIN_FILENO){
    // Input from the server terminal
    allOk = readInputFromUser(sData);
    if (!allOk){
      eventfd_t e = TERMINATE;
      write(sData->commonEventFd, &e, sizeof(eventfd_t));
    }
  } else {
    // Client socket
    genericPdu *p = getDataFromSocket(socket_fd);
    if (p == NULL) {
      // Client have disconnected unexpectedly
      handleQuit(socket_fd);
      return false;
    }
    // Its a message, process it
    if (p->opCode == JOIN) {
      allOk = handleJoin((pduJoin *)p, socket_fd);
    } else if (p->opCode == MESS) {
      allOk = handleMess((pduMess *)p, socket_fd);
    } else if (p->opCode == QUIT) {
      allOk = handleQuit(socket_fd);
    } else {
      fprintf(stderr, "Received unhandled message with OP Code: %d\n", p->opCode);
    }

    deletePdu(p);
  }
  return allOk;
}

void server_main(int argc, char **argv) {
  serverInputArgs args;

  parseServerArgs(argc, argv, &args);

  int nameServerSocket = establishConnectionWithNs(args);

  fprintf(stdout, "Socket value: %d\n", nameServerSocket);

  registerToServer(nameServerSocket, args);

  currentFreeParticipantSpot = 0;

  int epoll_fd = epoll_create1(0);
  int event_fd;

  int server_fd = setupServerSocket(args);

  // Setup Epoll and add server
  struct epoll_event ev_server;
  ev_server.data.fd = server_fd;
  ev_server.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev_server);

  // Add stdin
  struct epoll_event ev_stdin;
  ev_stdin.data.fd = STDIN_FILENO;
  ev_stdin.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev_stdin);

  // Add inter-thread communication FD.
  event_fd = eventfd(0, O_NONBLOCK);
  struct epoll_event ev_ITC;
  ev_ITC.data.fd = event_fd;
  ev_ITC.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &ev_ITC);


  facade_setToNonBlocking(server_fd);

  sData.commonEventFd = event_fd;
  sData.server_fd = server_fd;
  sData.numOfActiveFds = 1; // Just the server
  sData.epoll_fd = epoll_fd;

  readerInfo rInfo;
  rInfo.args = &sData;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;
  rInfo.numOfActiveFds = 1; // Only counting the Server socket.


  fflush(stdin);

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