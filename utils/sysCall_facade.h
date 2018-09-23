//
// Created by njia on 2018-07-08.
//

#ifndef DODOU2_TCP_SOCKET_H
#define DODOU2_TCP_SOCKET_H

#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

// Use as reference: https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/rzab6/xnonblock.htm

//Defines own version for testing purposes

//  status = pthread_kill( threadid, SIGUSR1);  <<<---- Signal other threads to STOP

// Client needs a separate thread for reading and writing to socket.


int facade_getAddrinfo(const char *name,
                       const char *service,
                       const struct addrinfo *req,
                       struct addrinfo **pai);

void facade_freeaddrinfo(struct addrinfo *addr);

int facade_createSocket(struct addrinfo **res);

int facade_bindSocket(int socket_fd, struct addrinfo **res);

int facade_connect(int socket_fd, struct addrinfo **res);

int facade_acceptConnections(int socket_fd, void (*fPtr)());

int facade_markSocketAsPassive(int socket_fd);

int facade_setsocketopt(int socket_fd, int lvl, int opt);

int facade_setToNonBlocking(int socket_fd);

int facade_epoll_wait(int epfd, struct epoll_event *events, int maxEvents, int timeout);

int facade_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

ssize_t facade_write(int socket_fd, void *packet, size_t size);

ssize_t facade_read(int socket_fd, void *buffer, size_t size);

#endif //DODOU2_TCP_SOCKET_H








