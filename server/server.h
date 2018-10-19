#ifndef DODOU2_SERVER_H
#define DODOU2_SERVER_H

#include <pthread.h>
#include <socketReaderAPI.h>

#include "clientConnection.h"
#include "pduReader.h"
#include "nameServerConnection.h"
#include "helpers.h"
#include "list.h"

typedef struct {
  int commonEventFd;
  int numOfActiveFds;
  void *args;
  int server_fd;
  int epoll_fd;
} serverData;

typedef struct {
  char *clientID;
  int socket_fd;
} participant;

participant participantList[UINT8_MAX];
uint8_t currentFreeParticipantSpot;
serverData sData;

void freeParticipant(void *id);

void parseServerArgs(int argc, char **argv, serverInputArgs *args);

void addToParticipantsList(int socket_fd, char *clientID);
void sendParticipantsListToClient(int socket_fd);
void notifyClientsNewClientJoined(int socket_fd, char *clientID);

bool handleJoin(pduJoin *join, int socket_fd);

bool handleMess(pduMess *mess, int socket_fd);

bool handleQuit(int socket_fd);

bool readInputFromUser(serverData *sData);

bool processSocketData(int socket_fd, void *args);

void server_main(int argc, char **argv);

#endif //DODOU2_SERVER_H
