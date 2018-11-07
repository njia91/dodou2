#include "server.h"

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
  pduPLeave leave;
  leave.opCode = PLEAVE;
  leave.timeStamp = getCurrentTime();

  // Find the id of the leaving client
  sem_wait(&helperMutex);
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
    }
  } else if (socket_fd == STDIN_FILENO) {
    // Input from the server terminal
    allOk = readInputFromUser(sData);
    if (!allOk) {
      fprintf(stdout, "Inter thread start terminate\n");
      setRunning(false);
      allOk = TERMINATE_SESSION;
      eventfd_t e = TERMINATE;
      write(sData->commonEvent_fd, &e, sizeof(eventfd_t));
    }
  } else if (socket_fd == sData->commonEvent_fd) {
    // Inter thread
    eventfd_t value = 0;

    if (facade_read(socket_fd, &value, sizeof(eventfd_t))){
      if (value == TERMINATE){
        allOk = EXIT_FD;
        eventfd_t e = TERMINATE;
        write(sData->commonEvent_fd, &e, sizeof(eventfd_t));
      }
    } else {
      fprintf(stderr, "Unnable to read from eventfd_Read()");
      allOk = TERMINATE_SESSION;
      eventfd_t e = TERMINATE;
      write(sData->commonEvent_fd, &e, sizeof(eventfd_t));
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
  registerToNameServer(nameServerSocket, args);

  sem_init(&helperMutex, 0, 1);
  setCurrentFreeParticipantSpot(0);

  // Setup Epoll and add server
  int epoll_fd = epoll_create1(0);
  int server_fd = setupServerSocket(args);
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
  int event_fd = eventfd(0, O_NONBLOCK);
  struct epoll_event ev_ITC;
  ev_ITC.data.fd = event_fd;
  ev_ITC.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &ev_ITC);

  facade_setToNonBlocking(server_fd);

  sData.server_fd = server_fd;
  sData.epoll_fd = epoll_fd;
  sData.commonEvent_fd = event_fd;

  readerInfo rInfo;
  rInfo.args = &sData;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;

  sem_init(&mutex, 0, 1);
  setRunning(true);

  fflush(stdin);

  // Create threads
  int numberOfThreads = 10;
  pthread_t threads[numberOfThreads];
  for (int i = 0; i < numberOfThreads; i++) {
    int threadCreated = pthread_create(&threads[i], NULL, waitForIncomingMessages, (void *)&rInfo);
    if (threadCreated != 0) {
      fprintf(stderr, "Unable to create pthread number %d. Error: %d\n", i, threadCreated);
    }
  }

  // Keep connection with name server
  while (checkRunning()) {
    if (gotACKResponse(nameServerSocket)) {
      fprintf(stdout, "Still connected to name server\n");
    } else {
      registerToNameServer(nameServerSocket, args);
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

  fprintf(stdout, "Shutting down threads...\n");
  for (int i = 0; i < numberOfThreads; i++) {
    int threadJoined = pthread_join(threads[i], NULL);
    if (threadJoined != 0) {
      fprintf(stderr, "Unable to join pthread number %d. Error: %d\n", i, threadJoined);
    }
  }

  sem_destroy(&mutex);
  sem_destroy(&helperMutex);
  closeAndRemoveFD(epoll_fd, server_fd);

  free(args.nameServerIP);
  free(args.serverName);
  free(args.nameServerPort);
}