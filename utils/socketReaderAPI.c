//
// Created by njia on 2018-08-01.
//

#include "socketReaderAPI.h"


void *waitForIncomingMessages(void *args){
  readerInfo *rInfo = (readerInfo *) args;
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event ev;

  int availFds = 0;

  while (true) {
    availFds = epoll_wait(rInfo->epoll_fd, events, MAX_EVENTS, -1);

    for (int i = 0; i < availFds; ++i) {
      // read data from available FD;

      ev.data.fd = events[i].data.fd;
      ev.events = EPOLLIN | EPOLLONESHOT | EPOLLEXCLUSIVE;
      epoll_ctl(rInfo->epoll_fd, EPOLL_CTL_ADD,events[i].data.fd, &ev);
    }
    break;
  }
}

void readDataFromSocket(int socketfd){
  int opCode = 0;
  int ret = readFromSocket(socketfd, &opCode, 1);
  void *pdu = getDataFromSocket()
}
