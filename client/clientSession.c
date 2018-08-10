//
// Created by njia on 2018-08-01.
//

#include <pduReader.h>

#include "clientSession.h"
#include "dod_socket.h"
#include "pduCommon.h"


genericPdu getPduFromSocket(int socket_fd){
  uint8_t opCode = 0;
  // If failed, terminate socket and shutdown
  int ret = readFromSocket(socket_fd, &opCode, 1);

  if (ret == -1){
    fprintf(stderr, "getPduFromSocket(): unable to read from socket \n");
    exit(EXIT_FAILURE);
  }

  void *pdu = getDataFromSocket(socket_fd, opCode);

  if (pdu == NULL){
    fprintf(stderr, "getDataFromSocket(): returned NULL \n");
  }

  return pdu;

}

void proccessSocketData(int socket_fd, void *threadArgs){
  genericPdu p = getPduFromSocket(socket_fd);

  p++;

  // Do logic with the data from socket.
}


int setupConnectionToServer(const uint8_t *ip, const char *port) {
  int socket_fd;
  struct addrinfo* res=0;
  struct addrinfo *rp = NULL;
  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  hints.ai_flags=AI_ADDRCONFIG;

  int ret = getAddrInformation((char *)ip, port , &hints, &res);
  if (ret){
    fprintf(stderr, "setupConnectionToSever(): Something wrong with getAddrInfo \n");
    exit(EXIT_FAILURE);
  }

  // Try each address until we connect
  for (rp = res; rp != NULL; rp = rp->ai_next){
    socket_fd = createSocket(&res);
    if (socket_fd ==-1){
      continue;
    }

    if(connectToServer(socket_fd, &res) != -1){
      break;
    }
  }
  if (rp == NULL){
    freeaddrinfo(res);
    fprintf(stderr,"setupConnectionToSever(): Cannot setup connection to Server: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  freeAddrInformation(&res);
  return socket_fd;
}

int joinChatSession(int server_fd, clientData *cData){
  pduJoin p;
  p.opCode = JOIN;
  p.id = (uint8_t *)cData->username;
  p.idSize = strlen(cData->username);
  int bufferSize = 0;
  uint8_t *buffer = pduCreator_join(&p, &bufferSize);

  int ret = writeToSocket(server_fd, buffer, bufferSize);
  if (ret == -1){
    return -1;
  }

  return 0;
}

int printServerParticipants(int server_fd, clientData *cData){
  uint8_t opCode = 0;
  int ret = readFromSocket(server_fd, &opCode, 1);

  if (ret == -1){
    fprintf(stderr, "Unable to read data from socket: %s\n", strerror(errno));
  }

  if (opCode != PARTICIPANTS){
    fprintf(stderr, "Invalid packet received from Server.\n"
                    "Expected: %d\n"
                    "Actual: %d\n", PARTICIPANTS, opCode);
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  pduParticipants *p = (pduParticipants *) getDataFromSocket(server_fd, opCode);

  fprintf(stdout, "-----------------------------------------------------------------\n");
  fprintf(stdout, "Online users: \n");

  for (int i = 0; i < p->noOfIds; i++){
    fprintf(stdout, "%s\n", p->ids[i]);
  }

  return 0;
}


void startChatSession(clientData *cData){
  // Connect to server
  pthread_t clientThread;
  struct epoll_event ev;
  int epoll_fd;

  int server_fd = setupConnectionToServer(cData->ipAdress, cData->port);

  // Listen on server socket and join.
  int ret = joinChatSession(server_fd, cData);

  if (ret == -1){
    fprintf(stderr," Failed to send JOIN PDU to server: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  ret = printServerParticipants(server_fd, cData);

  epoll_fd = epoll_create(0);

  if (epoll_fd == -1){
    fprintf(stderr,"Failed to create epoll FD -  %d\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // Setup Epoll
  ev.data.fd = server_fd;
  ev.events = EPOLLIN | EPOLLONESHOT | EPOLLEXCLUSIVE;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD,server_fd, &ev);

  readerInfo rInfo;
  rInfo.epoll_fd;
  rInfo.func = proccessSocketData;
  rInfo.packetList = NULL;
  rInfo.packetList_mutex = NULL;
  rInfo.incomingPacket = NULL;

  // Start chatt session
  ret = pthread_create(&clientThread, NULL, waitForIncomingMessages, (void *)rInfo);

  close(epoll_fd);
  close(server_fd);
}
