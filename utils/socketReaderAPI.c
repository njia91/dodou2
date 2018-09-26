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
  while (isSessionActive && rInfo->numOfActiveFds > 0) {
    availFds = facade_epoll_wait(rInfo->epoll_fd, events, MAX_EVENTS, -1);
    if (availFds == -1) {
      fprintf(stderr, "%s: ",__func__);
      perror("epoll_wait: ");
      isSessionActive = false;
    } else if (availFds == 0) { // Probably interrupted with a signal.
      fprintf(stderr, "%s: epoll_wait timed out with no readable FDs. Terminating. \n",__func__);
      isSessionActive = false;
    }

    for (int i = 0; i < availFds; i++) {
      if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR)){
        fprintf(stderr, "%s: Socket has shutdown by peer or due to error. \n", __func__);
        closeAndRemoveFD(rInfo->epoll_fd, events[i].data.fd);
        rInfo->numOfActiveFds--;
      } else if ((events[i].events & EPOLLIN) ){
        isSessionActive = rInfo->func(events[i].data.fd, rInfo->args);
        if (isSessionActive){
          ev.data.fd = events[i].data.fd;
          ev.events = EPOLLIN | EPOLLONESHOT ;
          if (facade_epoll_ctl(rInfo->epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1){
            fprintf(stderr, "%s: epoll_ctl failed. Errno: %s \n", __func__, strerror(errno));
          }
          facade_epoll_ctl(rInfo->epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
        }
      } else {
        fprintf(stderr, "%s: Unknown EPOLL EVENT %d  \n",__func__, events[i].events);
        closeAndRemoveFD(rInfo->epoll_fd, events[i].data.fd);
        rInfo->numOfActiveFds--;
      }
    }
  }
  return 0;
}

void closeAndRemoveFD(int epoll_fd, int toBeRemovedFd){
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, toBeRemovedFd, NULL);
  shutdown(toBeRemovedFd, SHUT_RDWR);
  close(toBeRemovedFd);
}

void clearStdin(){
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {}
}
