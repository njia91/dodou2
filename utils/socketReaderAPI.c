//
// Created by njia on 2018-08-01.
//

#include "socketReaderAPI.h"

#include <stdio.h>


void *waitForIncomingMessages(void *threadData){
  readerInfo *rInfo = (readerInfo *) threadData;
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event ev;
  bool isSessionActive = true;


  int availFds = 0;
  while (isSessionActive) {
    availFds = facade_epoll_wait(rInfo->epoll_fd, events, MAX_EVENTS, -1);
    if (availFds == -1) {
      perror("epoll_wait: ");
      break;
    } else if (availFds == 0) { // Probably interrupted with a signal.
      break;
    }

    for (int i = 0; i < availFds; ++i) {
      // read data from available FD;
      isSessionActive = rInfo->func(events[i].data.fd, (void *) rInfo);
      if (isSessionActive){
        ev.data.fd = events[i].data.fd;
        ev.events = EPOLLIN | EPOLLONESHOT | EPOLLEXCLUSIVE;
        facade_epoll_ctl(rInfo->epoll_fd, EPOLL_CTL_ADD, events[i].data.fd, &ev);
      }
      else {
        break;
      }
    }
  }
  printf("AVSLUTA SLUT PÅ FUNCTION!!! \n");
  return 0;
  //pthread_exit(NULL);
}
