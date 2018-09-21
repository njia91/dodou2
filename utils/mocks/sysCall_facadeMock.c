//
// Created by njia on 2018-07-20.
//
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <sysCall_facade.h>
#include <unitypes.h>
#include <stdio.h>
#include <stdbool.h>

#include "ThreadingSolution.h"


int facade_getAddrinfo(const char *__restrict __name,
                       const char *__restrict __service,
                       const struct addrinfo *__restrict __req,
                       struct addrinfo **__restrict __pai){
  *__pai = (struct addrinfo *)mock();
  return (int)mock();
}

void facade_freeaddrinfo(struct addrinfo *addr){
  return;
}

int facade_createSocket(struct addrinfo **res){
  return (int)mock();
}

int facade_bindSocket(int socket_fd, struct addrinfo **res){
  return (int)mock();
}

int facade_connect(int socket_fd, struct addrinfo **res){
  check_expected(socket_fd);
  return (int)mock();
}

int facade_acceptConnections(int socket_fd, void (*fPtr)()){
  return (int)mock();
}

int facade_markSocketAsPassive(int socket_fd){
  return (int)mock();
}

int facade_setsocketopt(int socket_fd, int lvl, int opt){
  return (int)mock();
}

int facade_setToNonBlocking(int socket_fd){
  return (int)mock();
}

int facade_epoll_wait(int epfd, struct epoll_event *events, int maxEvents, int timeout){
  if(IS_MOCKOBJECT_SET){
    return (int)mock();
  }
  return 0;
}

int facade_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event){
  return 0;
}

ssize_t facade_writeToSocket(int socket_fd, uint8_t *packet, int size) {
  return (int)mock();
}

ssize_t facade_readFromSocket(int socket_fd, uint8_t *buffer, int size) {
  *buffer = (uint8_t)mock();
  return size;
}
