//
// Created by njia on 2018-08-01.
//

#include "socketReaderAPI.h"


void *waitForIncomingMessages(void *threadData){
  readerInfo *rInfo = (readerInfo *) threadData;
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event ev;

  int availFds = 0;

  while (true) {
    availFds = epoll_wait(rInfo->epoll_fd, events, MAX_EVENTS, -1);

    if (availFds == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < availFds; ++i) {
      // read data from available FD;
      rInfo->func(events[i].data.fd, (void *) rInfo);

      ev.data.fd = events[i].data.fd;
      ev.events = EPOLLIN | EPOLLONESHOT | EPOLLEXCLUSIVE;
      epoll_ctl(rInfo->epoll_fd, EPOLL_CTL_ADD,events[i].data.fd, &ev);
    }
    break;
  }

  pthread_exit(NULL);
}
