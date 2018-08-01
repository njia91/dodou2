//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_SOCKETREADERAPI_H
#define DODOU2_SOCKETREADERAPI_H


#include "list.h"
#include<pthread.h>
#include <sys/epoll.h>
#include "pduReader.h"

static const MAX_EVENTS = 32;

typedef struct {
  int epoll_fd;
  pthread_cond_t incomingPacket;
  pthread_mutex_t packetList_mutex;
  dll *packetList;
} readerInfo;

void *waitForIncomingMessages(void *epollArgs);

void readDataFromSocket(int socketfd);





#endif //DODOU2_SOCKETREADERAPI_H
