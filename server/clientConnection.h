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

/**
 * When a client have joined the server, this can be used to send a list
 * of all current participants to the client.
 * @param socket_fd The socket of the client who should receive the participants list
 */
void sendParticipantsListToClient(int socket_fd);

/**
 * Will notify all participants that a new client have joined the server.
 * Will not notify the new client.
 * @param socket_fd Socket of the new client
 * @param clientID ID of the new client
 */
void notifyClientsNewClientJoined(int socket_fd, char *clientID);

bool sendDataFromServer(uint8_t *data, size_t dataSize);
bool sendQuitFromServer();
bool sendMessageFromServer(pduMess *mess);

bool closeConnectionToClient(int client_fd, serverData *sData);

#endif //DODOU2_CLIENTCONNECTION_H
