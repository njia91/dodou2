//
// Created by njia on 2018-07-08.
//

#ifndef DODOU2_TCP_SOCKET_H
#define DODOU2_TCP_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h>

// Use as reference: https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/rzab6/xnonblock.htm


//  status = pthread_kill( threadid, SIGUSR1);  <<<---- Signal other threads to STOP

// Client needs a separate thread for reading and writing to socket.

int createSocket(struct addrinfo **res);

int bindSocket(int socket_fd, struct addrinfo **res);

int connectToServer(int socket_fd, struct addrinfo **res);

int acceptConnections(int socket_fd, void (*fPtr)());

int markSocketAsPassive(int socket_fd);

int setsocketopt(int socket_fd, int lvl, int opt);

int setToNonBlocking(int socket_fd);

int writeToSocket(int socket_fd, char *packet);

int readFromSocket(int socket_fd, char *packet);

#endif //DODOU2_TCP_SOCKET_H








