//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_CLIENTSESSION_H
#define DODOU2_CLIENTSESSION_H

#include "client.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "socketReaderAPI.h"

genericPdu getPduFromSocket(int socket_fd);

void proccessSocketData(int socket_fd, void *threadArgs);


void startChatSession(clientData *cData);

int setupConnectionToServer(const char *ip, const char *port);

int joinChatSession(int server_fd, clientData *cData);

int printServerParticipants(int server_fd, clientData *cData);


#endif //DODOU2_CLIENTSESSION_H
