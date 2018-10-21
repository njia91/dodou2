#ifndef DODOU2_CLIENTCONNECTION_H
#define DODOU2_CLIENTCONNECTION_H

#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sysCall_facade.h>
#include <pduCommon.h>
#include <pduCreator.h>


#include "helpers.h"


int setupServerSocket(serverInputArgs args);

int listenForIncomingConnection(int server_fd);

void sendParticipantsListToClient(int socket_fd);
void notifyClientsNewClientJoined(int socket_fd, char *clientID);
bool closeConnectionToClient(int client_fd, serverData *sData);

#endif //DODOU2_CLIENTCONNECTION_H
