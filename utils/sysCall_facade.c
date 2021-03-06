//
// Created by njia on 2018-07-08.
//

#include "sysCall_facade.h"
#include <netdb.h>


int facade_getAddrinfo(const char *name,
                       const char *service,
                       const struct addrinfo *req,
                       struct addrinfo **pai){
  return getaddrinfo(name, service, req, pai);
}

void facade_freeaddrinfo(struct addrinfo *addr){
  freeaddrinfo(addr);
}

int facade_createSocket(struct addrinfo **res){
  int fd = 0;
  fd = socket ((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);

  if (fd == -1){
    fprintf(stderr, "%s\n", strerror(errno));
  }
  return fd;
}

int facade_bindSocket(int socket_fd, struct addrinfo **res){
  if (bind(socket_fd, (*res)->ai_addr, (*res)->ai_addrlen) == -1){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int facade_connect(int socket_fd, struct addrinfo **res){
  return connect(socket_fd, (*res)->ai_addr, (*res)->ai_addrlen);
}

int facade_markSocketAsPassive(int socket_fd){
  if(listen(socket_fd, SOMAXCONN)){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
return 0;
}

int facade_setsocketopt(int socket_fd, int lvl, int opt){
  int option = 1;
  if (setsockopt(socket_fd, lvl, opt, &option, sizeof(&option)) == -1){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int facade_setToNonBlocking(int socket_fd){
  // Mark socket as non-blocking
  int flags = fcntl(socket_fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  return fcntl(socket_fd, F_SETFL, flags);
}

int facade_accept(int socket_fd) {
  return accept(socket_fd, 0, 0);
}

int facade_epoll_wait(int epfd, struct epoll_event *events, int maxEvents, int timeout){
  return epoll_wait(epfd, events, maxEvents, timeout);
}

int facade_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event){
  return epoll_ctl(epfd, op, fd, event);
}

ssize_t facade_write(int socket_fd, void *packet, size_t size){
  return write(socket_fd, packet, size);
}

ssize_t facade_read(int socket_fd, void *buffer, size_t size){
  return read(socket_fd, buffer, size);
}

