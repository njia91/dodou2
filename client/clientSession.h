//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_CLIENTSESSION_H
#define DODOU2_CLIENTSESSION_H

#define _POSIX_C_SOURCE >= 200809L

#include <stdio.h>
#include "client.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "socketReaderAPI.h"

genericPdu getPduFromSocket(int socket_fd);

void processSocketData(int socket_fd, void *threadArgs);

void startChatSession(clientData *cData);

int setupConnectionToServer(const uint8_t *ip, const char *port);

int joinChatSession(int server_fd, clientData *cData);

int printServerParticipants(int server_fd, clientData *cData);

void readInputFromUser();


#endif //DODOU2_CLIENTSESSION_H