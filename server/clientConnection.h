#ifndef DODOU2_CLIENTCONNECTION_H
#define DODOU2_CLIENTCONNECTION_H

#include <unistd.h>
#include <errno.h>

#include "helpers.h"

int setupServerSocket(serverInputArgs args);

int listenForIncomingConnection(int server_fd);

#endif //DODOU2_CLIENTCONNECTION_H
