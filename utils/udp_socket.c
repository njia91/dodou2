//
// Created by njia on 2018-07-18.
//

#include "udp_socket.h"


int udp_createSocket(struct addrinfo **res)

int udp_bindSocket(int socket_fd, struct addrinfo **res);

int udp_writeToSocket();

int udp_readFromSocket();
