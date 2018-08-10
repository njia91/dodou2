//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_SOCKETREADERAPI_H
#define DODOU2_SOCKETREADERAPI_H


#include "list.h"
#include <pthread.h>
#include <sys/epoll.h>
#include "pduReader.h"
#include "pduCommon.h"

static const int MAX_EVENTS = 32;

typedef void processEvent(int, void *);

typedef struct {
  int epoll_fd;
  dll *packetList;
  pthread_cond_t incomingPacket;
  pthread_mutex_t packetList_mutex;
  processEvent *func;
} readerInfo;


void *waitForIncomingMessages(void *threadData);





#endif //DODOU2_SOCKETREADERAPI_H
