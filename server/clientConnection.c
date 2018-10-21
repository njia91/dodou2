#include "clientConnection.h"

int setupServerSocket(serverInputArgs args) {
  int server_fd;
  struct addrinfo* addrInfo = 0;

  fprintf(stdout, "Listening on port: %d\n", atoi(args.serverPort));

  fillInAddrInfo(&addrInfo, atoi(args.serverPort), NULL, SOCK_STREAM, AI_PASSIVE);

  server_fd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
  if (server_fd == -1) {
    fprintf(stderr, "Failed to create socket\n");
    exit(EXIT_FAILURE);
  }

  // Allow reuse of local addresses.
  int resueAddr = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(resueAddr), sizeof(&resueAddr)) == -1) {
    close(server_fd);
    fprintf(stderr, "Failed to set socket optionals\n");
    exit(EXIT_FAILURE);
  }

  if (bind(server_fd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
    close(server_fd);
    fprintf(stderr, "Failed to bind socket\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(addrInfo);

  if (listen(server_fd, SOMAXCONN)) {
    fprintf(stderr, "Failed to listen to socket\n");
    exit(EXIT_FAILURE);
  }
  return server_fd;
}

int listenForIncomingConnection(int server_fd) {
  int connection_fd;
  fprintf(stdout, "Listening for connections on %d...\n", server_fd);
  // Listen for ONE incoming connection.
  while((connection_fd = accept(server_fd, 0, 0)) == -1) {
    fprintf(stderr, "SERVER ERROR: %s", strerror(errno));
  }
  return connection_fd;
}

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

void notifyClientsNewClientJoined(int socket_fd, char *clientID) {
  pduPJoin pJoin;
  pJoin.opCode = PJOIN;
  pJoin.idSize = (uint8_t) strlen(clientID);
  pJoin.id = calloc(pJoin.idSize, sizeof(char));
  memcpy(pJoin.id, clientID, pJoin.idSize);
  pJoin.timeStamp = getCurrentTime();

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

bool closeConnectionToClient(int client_fd, serverData *sData) {
  bool allOk = true;
  struct epoll_event ev_client;
  ev_client.data.fd = client_fd;
  ev_client.events = EPOLLIN | EPOLLONESHOT;
  int result = facade_epoll_ctl(sData->epoll_fd, EPOLL_CTL_DEL, client_fd, &ev_client);
  if (result == -1) {
    fprintf(stderr, "Failed to remove socket %d from epoll: %s\n", client_fd, strerror(errno));
    allOk = false;
  } else {
    sData->numOfActiveFds--;
    fprintf(stdout, "Removed client from epoll: %d, numberOfEpoll:%d\n", client_fd, sData->numOfActiveFds);
  }
  close(client_fd);
  return allOk;
}