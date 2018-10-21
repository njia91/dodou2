#ifndef DODOU2_SERVER_H
#define DODOU2_SERVER_H

#include <pthread.h>
#include <socketReaderAPI.h>

#include "clientConnection.h"
#include "pduReader.h"
#include "nameServerConnection.h"
#include "helpers.h"
#include "list.h"

serverData sData;

void parseServerArgs(int argc, char **argv, serverInputArgs *args);

/**
 * Will do the following:
 * - Send the participants list to the new client
 * - Check if the server is full. If it is, notify the new client and close the connection
 * - If the server is not full, add the new client to the participants list
 * - Notify other participants that a new client have joined
 * @param join The join message
 * @param socket_fd The new clients socket
 * @return If everything was ok
 */
bool handleJoin(pduJoin *join, int socket_fd);

/**
 * Will send a message to everyone except one
 * @param mess The message to send
 * @param socket_fd The client's socket to not send to
 * @return If everything is ok
 */
bool handleMess(pduMess *mess, int socket_fd);

/**
 * Notify all clients in the participants that a client have left.
 * Except for the leaving client, it does not receive a message.
 * @param socket_fd The socket of the leaving client
 * @return If all is ok
 */
bool handleQuit(int socket_fd);

/**
 *
 * @param sData
 * @return
 */
bool readInputFromUser(serverData *sData);

/**
 * Processes the data that is read from a socket
 * @param socket_fd The socket to read data from
 * @param args Important data
 * @return If all is ok
 */
bool processSocketData(int socket_fd, void *args);

void server_main(int argc, char **argv);

#endif //DODOU2_SERVER_H
