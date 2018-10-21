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
    mess.timeStamp = getCurrentTime();
    mess.id = NULL;

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

bool sendDataFromServer(uint8_t *data, size_t dataSize) {
  ssize_t ret = 0;
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    ret = facade_write(participantList[i].socket_fd, data, dataSize);
  }
  free(data);
  if (ret != dataSize) {
    fprintf(stderr, "%s: Unable to write all data to socket. Size %zd  ret %zd\n",__func__, dataSize, ret);
    fprintf(stderr, "Errno : %s \n", strerror(errno));
    if (errno == EBADF) {
      return false;
    }
  }
  return true;
}

bool sendQuitFromServer() {
  size_t quitDataSize;
  uint8_t *quitData = pduCreator_quit(&quitDataSize);
  return sendDataFromServer(quitData, quitDataSize);
}

bool sendMessageFromServer(pduMess *mess) {
  size_t messDataSize;
  uint8_t *messData = pduCreator_mess(mess, &messDataSize);
  free(mess->message);
  return sendDataFromServer(messData, messDataSize);
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
      mess->timeStamp = getCurrentTime();
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
  leave.timeStamp = getCurrentTime();

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
  return true;
}

/**
 *
 * @param sData
 * @return
 */
bool readInputFromUser(serverData *sData) {
  size_t inputBufferSize = 0;
  char *inputBuffer = NULL;
  bool active = true;
  fflush(stdin);

  if (getline(&inputBuffer, &inputBufferSize, stdin) == -1) {
    fprintf(stderr, "Failed to read data from user: %s\n", strerror(errno));
    active = false;
  } else {
    if (strcmp(inputBuffer, "QUIT\n") == 0) {
      char message[] = "Server is shutting down";
      pduMess mess;
      mess.opCode = MESS;
      mess.messageSize = (uint16_t) strlen(message);
      mess.message = calloc(mess.messageSize, sizeof(char));
      memcpy(mess.message, message, mess.messageSize);
      mess.timeStamp = getCurrentTime();
      mess.id = NULL;
      mess.idSize = 0;

      // Send message to all clients that server is shutting down
      active = sendMessageFromServer(&mess);
      if (active) {
        // Send quit message to all clients to close the connection
        active = sendQuitFromServer();
      }

      while (currentFreeParticipantSpot > 0) {
        closeConnectionToClient(participantList[currentFreeParticipantSpot - 1].socket_fd, sData);
        for (int i = 0; i < currentFreeParticipantSpot; i++) {
          int socket_fd = participantList[currentFreeParticipantSpot - 1].socket_fd;
          if (socket_fd == participantList[i].socket_fd) {
             // Remove the client from the participants list
            free(participantList[i].clientID);
            participantList[i] = participantList[--currentFreeParticipantSpot];
            break;
          }
        }
      }
      // TODO: Shut down server gracefully.
      // TODO: Notify other thread that we are shutting down.
    } else {
      // Prepare message
      pduMess mess;
      mess.opCode = MESS;
      mess.id = NULL;
      mess.idSize = 0;
      mess.message = (uint8_t *) inputBuffer;
      mess.messageSize = (uint16_t) strlen(inputBuffer);
      mess.timeStamp = getCurrentTime();

      sendMessageFromServer(&mess);
      // TODO: Allow server to list active users
      // TODO: Allow server to kick clients
      // TODO: More nice functionality?
    }
  }
  free(inputBuffer);
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
    int client_fd = listenForIncomingConnection(socket_fd);
    facade_setToNonBlocking(client_fd);
    // Add Client socket
    struct epoll_event ev_client;
    ev_client.data.fd = client_fd;
    ev_client.events = EPOLLIN | EPOLLONESHOT;
    int result = facade_epoll_ctl(sData->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client);
    if (result == -1) {
      fprintf(stderr, "Failed to add socket %d to epoll: %s\n", client_fd, strerror(errno));
      allOk = false;
    } else {
      sData->numOfActiveFds++;
      fprintf(stdout, "Added client to epoll: %d, numberOfEpoll:%d\n", client_fd, sData->numOfActiveFds);
    }
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
      if (allOk) {
        allOk = closeConnectionToClient(socket_fd, sData);
      }
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