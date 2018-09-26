//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_SOCKETREADERAPI_H
#define DODOU2_SOCKETREADERAPI_H


#include "list.h"
#include <pthread.h>
#include "sysCall_facade.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "pduReader.h"
#include "pduCommon.h"

static const int MAX_EVENTS = 32;

typedef uint64_t eventfd_t;

// EventFd Code
static const eventfd_t TERMINATE = 1;

typedef bool processEvent(int, void *);

typedef struct {
  int epoll_fd;
  int numOfActiveFds;
  void *args;
  processEvent *func;
} readerInfo;


void *waitForIncomingMessages(void *threadData);

void closeAndRemoveFD(int epoll_fd, int toBeRemovedFd);

void clearStdin();


#endif //DODOU2_SOCKETREADERAPI_H
