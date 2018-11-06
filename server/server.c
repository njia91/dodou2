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

bool handleJoin(pduJoin *join, int socket_fd) {
  fprintf(stdout, "Received JOIN message from: %s\n", join->id);

  char* clientID = calloc(join->idSize + 1, sizeof(char));
  memcpy(clientID, join->id, join->idSize);


  addToParticipantsList(socket_fd, clientID);
  sendParticipantsListToClient(socket_fd);

  if (getCurrentFreeParticipantSpot() >= UINT8_MAX) {
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

  notifyClientsNewClientJoined(socket_fd, clientID);

  free(clientID);
  return true;
}

bool handleMess(pduMess *mess, int socket_fd) {

  if (!mess->isCheckSumOk) {
    fprintf(stderr, "bad checksum\n");
    return false;
  }

  // Add more info to the message
  sem_wait(&helperMutex);
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      mess->idSize = (uint8_t) strlen(participantList[i].clientID);
      memcpy(mess->id, participantList[i].clientID, mess->idSize);
      mess->timeStamp = getCurrentTime();
    }
  }
  sem_post(&helperMutex);

  fprintf(stdout, "Received a message from: %s\n", mess->id);
  // Send the message to everyone except the sender
  size_t bufferSize;
  uint8_t *buffer = pduCreator_mess(mess, &bufferSize);
  sem_wait(&helperMutex);
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd != participantList[i].socket_fd) {
      facade_write(participantList[i].socket_fd, buffer, bufferSize);
    }
  }
  sem_post(&helperMutex);
  free(buffer);
  return true;
}

bool handleQuit(int socket_fd) {
  // Prepare a leave message
  pduPLeave leave;
  leave.opCode = PLEAVE;
  leave.timeStamp = getCurrentTime();

  // Find the id of the leaving client
  sem_post(&helperMutex);
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      fprintf(stdout, "%s have left the chat\n", participantList[i].clientID);
      // Add the client id to the message
      leave.idSize = (uint8_t) strlen(participantList[i].clientID);
      leave.id = calloc(leave.idSize, sizeof(char));
      memcpy(leave.id, participantList[i].clientID, leave.idSize);
      removeFromParticipantsList(participantList[i].socket_fd);
      break;
    }
  }
  sem_post(&helperMutex);

  size_t bufferSize;
  uint8_t *buffer = pduCreator_pLeave(&leave, &bufferSize);
  free(leave.id);

  // Notify all in participants list that a client have left
  sem_wait(&helperMutex);
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    facade_write(participantList[i].socket_fd, buffer, bufferSize);
  }
  sem_post(&helperMutex);
  free(buffer);

  return true;
}

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
        sendQuitFromServer();
      }

      sem_post(&helperMutex);
      while (currentFreeParticipantSpot > 0) {
        closeConnectionToClient(participantList[currentFreeParticipantSpot - 1].socket_fd, sData);
        for (int i = 0; i < currentFreeParticipantSpot; i++) {
          int socket_fd = participantList[currentFreeParticipantSpot - 1].socket_fd;
          if (socket_fd == participantList[i].socket_fd) {
            removeFromParticipantsList(participantList[i].socket_fd);
            break;
          }
        }
      }
      sem_wait(&helperMutex);
      fprintf(stdout, "Shutting down server...\n");
      setRunning(false);
      active = false;
    } else if (strcmp(inputBuffer, "LIST\n") == 0) {
      // List all active users
      if (getCurrentFreeParticipantSpot() == 0) {
        fprintf(stdout, "No active users\n");
      } else {
        sem_wait(&helperMutex);
        for (int i = 0; i < currentFreeParticipantSpot; i++) {
          fprintf(stdout, "User ID: %d, User name:%s\n", participantList[i].socket_fd, participantList[i].clientID);
        }
        sem_post(&helperMutex);
      }
    } else if (startsWith("KICK", inputBuffer)) {
      char *clientID;
      clientID = strtok (inputBuffer," ");
      if (clientID != NULL) {
        clientID = strtok (NULL," ");
        if (clientID != NULL) {
          int client_fd = stringToInt(clientID);
          char messageString[] = "You have been kicked by the server";
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
          facade_write(client_fd, buffer, bufferSize);

          free(mess.message);
          free(buffer);

          buffer = pduCreator_quit(&bufferSize);
          // Tell client to close connection
          facade_write(client_fd, buffer, bufferSize);
          free(buffer);

          removeFromParticipantsList(client_fd);
          closeAndRemoveFD(sData->epoll_fd, client_fd);
          active = false;
        }
      }
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
      // TODO: More nice functionality?
    }
  }
  free(inputBuffer);
  return active;
}

int processSocketData(int socket_fd, void *args) {
  int allOk = REARM_FD;
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
      return REMOVE_FD;
    } else {
      sData->numOfActiveFds++;
      //fprintf(stdout, "Added client to epoll: %d, numberOfEpoll:%d\n", client_fd, sData->numOfActiveFds);
    }
  } else if (socket_fd == STDIN_FILENO) {
    // Input from the server terminal
    allOk = readInputFromUser(sData);
    if (!allOk) {
      setRunning(false);
      allOk = TERMINATE_SESSION;
    }
  } else {
    // Client socket
    genericPdu *p = getDataFromSocket(socket_fd);
    if (p == NULL) {
      // Client have disconnected unexpectedly
      handleQuit(socket_fd);
      closeConnectionToClient(socket_fd, sData);
      return REMOVE_FD;
    }
    // It's a message, process it
    if (p->opCode == JOIN) {
      allOk = handleJoin((pduJoin *)p, socket_fd);
    } else if (p->opCode == MESS) {
      handleMess((pduMess *)p, socket_fd);
    } else if (p->opCode == QUIT) {
      handleQuit(socket_fd);
      closeConnectionToClient(socket_fd, sData);
      return REMOVE_FD;
    } else {
      fprintf(stderr, "Received unhandled message with OP Code: %d\n", p->opCode);
    }

    deletePdu(p);
  }
  return allOk;
}

bool checkRunning() {
  bool currentRunning;
  sem_wait(&mutex);
  currentRunning = running;
  sem_post(&mutex);
  return currentRunning;
}

bool setRunning(bool newRunning) {
  sem_wait(&mutex);
  running = newRunning;
  sem_post(&mutex);
}

void server_main(int argc, char **argv) {
  serverInputArgs args;

  parseServerArgs(argc, argv, &args);

  int nameServerSocket = establishConnectionWithNs(args);

  fprintf(stdout, "Socket value: %d\n", nameServerSocket);

  registerToServer(nameServerSocket, args);

  sem_init(&helperMutex, 0, 1);
  setCurrentFreeParticipantSpot(0);

  int epoll_fd = epoll_create1(0);

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

  facade_setToNonBlocking(server_fd);

  sData.server_fd = server_fd;
  sData.numOfActiveFds = 1; // Just the server
  sData.epoll_fd = epoll_fd;

  readerInfo rInfo;
  rInfo.args = &sData;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;
  rInfo.numOfActiveFds = 1; // Only counting the Server socket.

  sem_init(&mutex, 0, 1);
  setRunning(true);

  fflush(stdin);

  pthread_t receivingThread;
  pthread_t receivingThread2;
  int ret = pthread_create(&receivingThread, NULL, waitForIncomingMessages, (void *)&rInfo);

  pthread_create(&receivingThread2, NULL, waitForIncomingMessages, (void *)&rInfo);

  if (ret) {
    fprintf(stderr, "Unable to create a pthread. Error: %d\n", ret);
  }

  while (checkRunning()) {
    if (gotACKResponse(nameServerSocket)) {
      fprintf(stdout, "Still connected to server\n");
    } else {
      registerToServer(nameServerSocket, args);
      fprintf(stdout, "Lost contact with name server, connecting again\n");
    }
    for (int i = 0; i < 8; i++) {
      if (!checkRunning()) {
        break;
      } else {
        sleep(1);
      }
    }
  }
  fprintf(stdout, "Shutting down server\n");
  closeAndRemoveFD(epoll_fd, server_fd);
  pthread_join(receivingThread, NULL);
  pthread_join(receivingThread2, NULL);
  sem_destroy(&mutex);
  sem_destroy(&helperMutex);

  free(args.nameServerIP);
  free(args.serverName);
  free(args.nameServerPort);
}