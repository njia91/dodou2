//
// Created by njia on 2018-08-01.
//

#ifndef DODOU2_CLIENTSESSION_H
#define DODOU2_CLIENTSESSION_H

#define _GNU_SOURCE

#include <stdio.h>
#include "client.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "socketReaderAPI.h"
#include "pduCreator.h"
#include "pduReader.h"
#include <time.h>

/**
 * Will read collect a PDU from given socket.
 *
 * @param socket_fd Socket file descriptor
 * @return generic PDU
 *
 */
genericPdu *getPduFromSocket(int socket_fd);

/**
 * Will process data from the given socket. It will only
 * collect and process one PDU.
 *
 * @param socket_fd Socket file descriptor
 * #param args Client data arguments
 * @return True if socket should be re-armed in epoll, otherwise false.
 *
 */
int processSocketData(int socket_fd, void *args);

/**
 * Prints a server message one another client has left or
 * joined the chat session
 *
 * @param pJoin PDU Join
 *
 */
void notifyUserOfChatRoomChanges(pduPJoin *pJoin);


//void handlePLeacePdu(pduPLeave *pLeave);
//
//void handleQuitPdu(pduQuit *quit);

/**
 * Handles a message PDU from the server.
 * If checksum is OK it will print the message to the user.
 *
 * @param mess PDU message
 * @return True if checksum is OK, and the socket should be re-armed for epoll.
 */
bool handleMessPdu(pduMess *mess);

/**
 * Will do following:
 * - setup connection with server.
 * - configure the epoll session.
 * - Add stdin and ITC FD to epoll.
 *
 * @param inArgs Arguments given from user.
 */
void startChatSession(inputArgs *inArgs);

/**
 * Will do following:
 * - setup connection with server.
 *
 * @param ip server ip.
 * @param port server port
 */
int setupConnectionToServer(const uint8_t *ip, const char *port);

/**
 * Sends a PDU JOIN to the server to register the client to
 * the chat session.
 *
 * @param socket_fd Socket file descriptor
 * #param inArgs Client data arguments (client name)
 */
int joinChatSession(int server_fd, inputArgs *inArgs);

/**
 * Prints the PDU Participants to the client which will
 * present all active clients in the current chat session.
 *
 * @param pduParticipants Participants PDU.
 */
int printServerParticipants(pduParticipants *p);

/**
 * Read input from user
 *
 * @param cData Client Data
 */
int readInputFromUser(clientData *cData);

#endif //DODOU2_CLIENTSESSION_H
