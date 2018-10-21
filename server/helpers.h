#ifndef DODOU2_HELPERS_H
#define DODOU2_HELPERS_H

#include <stdio.h>
#include <memory.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    char* serverPort;
    char* serverName;
    uint8_t* nameServerIP;
    char* nameServerPort;
} serverInputArgs;


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

uint32_t getCurrentTime();

void fillInAddrInfo(struct addrinfo **addrInfo, const int port, const char *IPAddress, int socketType, int flags);
void addToParticipantsList(int socket_fd, char *clientID);

#endif //DODOU2_HELPERS_H
