//
// Created by njia on 2018-07-08.
//




#include "dod_socket.h"


int getAddrInformation(const char * name,
                       const char *service,
                       const struct addrinfo * req,
                       struct addrinfo ** pai){
  getaddrinfo(name, service, req, pai);
}

void freeAddrInformation(struct addrinfo **addr){
  freeaddrinfo(addr);
}

int createSocket(struct addrinfo **res){
  int fd = 0;
  fd = socket ((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);

  if (fd == -1){
    fprintf(stderr, "%s\n", strerror(errno));
  }
  return fd;
}

int bindSocket(int socket_fd, struct addrinfo **res){
  if (bind(socket_fd, (*res)->ai_addr, (*res)->ai_addrlen) == -1){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int connectToServer(int socket_fd, struct addrinfo **res){
  return connect(socket_fd, (*res)->ai_addr, (*res)->ai_addrlen);
}

int markSocketAsPassive(int socket_fd){
  if(listen(socket_fd, SOMAXCONN)){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int setsocketopt(int socket_fd, int lvl, int opt){
  int option = 1;
  if (setsockopt(socket_fd, lvl, opt, &option, sizeof(&option)) == -1){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int setToNonBlocking(int socket_fd){
  int on = 1;
  if (ioctl(socket_fd, FIONBIO, (char *)&on) == -1){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int tcp_acceptConnections(int socket_fd, void (*fPtr)()){
  return accept(socket_fd, NULL, NULL);
}

int facade_epoll_wait()

int writeToSocket(int socket_fd, uint8_t *packet, int size);

int readFromSocket(int socket_fd, uint8_t *buffer, int size);

