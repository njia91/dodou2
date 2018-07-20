//
// Created by njia on 2018-07-18.
//

#ifndef DODOU2_UDP_SOCKET_H
#define DODOU2_UDP_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h>

int udp_createSocket(struct addrinfo **res);

int udp_bindSocket(int socket_fd, struct addrinfo **res);

int udp_writeToSocket();

int udp_readFromSocket();

#endif //DODOU2_UDP_SOCKET_H
