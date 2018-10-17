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

participant participantList[200];
uint8_t currentFreeParticipantSpot;

dll *participantsList;

void freeParticipant(void *id);

void parseServerArgs(int argc, char **argv, serverInputArgs *args);

void handleJoin(pduJoin *join, int socket_fd);

void handleMess(pduMess *mess, int socket_fd);

void handleQuit(pduQuit *quit, int socket_fd);

bool processSocketData(int socket_fd, void *args);

void server_main(int argc, char **argv);

#endif //DODOU2_SERVER_H
