//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_CLIENTSESSION_H
#define DODOU2_CLIENTSESSION_H

#define _GNU_SOURCE

#include <stdio.h>
#include "client.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "socketReaderAPI.h"
#include "pduCreator.h"
#include "pduReader.h"
#include <time.h>

genericPdu *getPduFromSocket(int socket_fd);

bool processSocketData(int socket_fd, void *args);

void notifyUserOfChatRoomChanges(pduPJoin *pJoin);

void handlePLeacePdu(pduPLeave *pLeave);

void handleQuitPdu(pduQuit *quit);

bool handleMessPdu(pduMess *mess);

void startChatSession(inputArgs *inArgs);

int setupConnectionToServer(const uint8_t *ip, const char *port);

int joinChatSession(int server_fd, inputArgs *inArgs);

int printServerParticipants(pduParticipants *p);

bool readInputFromUser(clientData *cData);

#endif //DODOU2_CLIENTSESSION_H
