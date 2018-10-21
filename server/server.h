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

bool sendDataFromServer(uint8_t *data, size_t dataSize);
bool sendQuitFromServer();
bool sendMessageFromServer(pduMess *mess);

bool handleJoin(pduJoin *join, int socket_fd);
bool handleMess(pduMess *mess, int socket_fd);
bool handleQuit(int socket_fd);

bool readInputFromUser(serverData *sData);

bool processSocketData(int socket_fd, void *args);

void server_main(int argc, char **argv);

#endif //DODOU2_SERVER_H
