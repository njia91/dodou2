#include "inputHandler.h"

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

          sem_post(&helperMutex);
          removeFromParticipantsList(client_fd);
          sem_wait(&helperMutex);
          closeAndRemoveFD(sData->epoll_fd, client_fd);
          active = true;
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