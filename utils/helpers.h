#ifndef DODOU2_HELPERS_H
#define DODOU2_HELPERS_H

#include <stdio.h>
#include <memory.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

typedef struct {
    char* serverPort;
    char* serverName;
    uint8_t* nameServerIP;
    char* nameServerPort;
} serverInputArgs;


typedef struct {
  void *args;
  int server_fd;
  int epoll_fd;
  int commonEvent_fd;
} serverData;

typedef struct {
  char *clientID;
  int socket_fd;
} participant;

participant participantList[UINT8_MAX];
uint8_t currentFreeParticipantSpot;
sem_t helperMutex;

uint32_t getCurrentTime();
bool startsWith(const char *pre, const char *str);

uint8_t getCurrentFreeParticipantSpot();
void setCurrentFreeParticipantSpot(uint8_t newVal);

int stringToInt(char *string);

/**
 * Add a client to the participants list
 * @param socket_fd The clients socket
 * @param clientID The clients ID
 */
void addToParticipantsList(int socket_fd, char *clientID);

void removeFromParticipantsList(int socket_fd);

void fillInAddrInfo(struct addrinfo **addrInfo, int port, const uint8_t *IPAddress, int socketType, int flags);

#endif //DODOU2_HELPERS_H
