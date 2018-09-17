//
// Created by njia on 2018-08-01.
//

#include "clientSession.h"
#include <pduReader.h>
#include "sysCall_facade.h"
#include "pduCommon.h"


genericPdu *getPduFromSocket(int socket_fd){
  // If failed, terminate socket and shutdown
  genericPdu *pdu =  getDataFromSocket(socket_fd);

  if (pdu == NULL){
    fprintf(stderr, "getDataFromSocket(): returned NULL \n");
  }

  printf(" NU ÄR JAG HÄR ! OPCODE %u !! \n", pdu->opCode);

  return pdu;
}

void processSocketData(int socket_fd, void *args){

  printf(" TJooohoo Process SocketData!! \n");
  genericPdu *p = getPduFromSocket(socket_fd);

  p++;

  printf("Nu dör jag... \n");
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

  int ret = facade_getAddrinfo((char *) ip, port, &hints, &res);
  if (ret){
    fprintf(stderr, "setupConnectionToSever(): Something wrong with getAddrInfo \n");
    exit(EXIT_FAILURE);
  }

  // Try each address until we connect
  for (rp = res; rp != NULL; rp = rp->ai_next){
    socket_fd = facade_createSocket(&res);
    if (socket_fd ==-1){
      continue;
    }

    if(facade_connectToServer(socket_fd, &res) != -1){
      break;
    }
  }
  if (rp == NULL){
    facade_freeaddrinfo(res);
    fprintf(stderr,"setupConnectionToSever(): Cannot setup connection to Server: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  facade_freeaddrinfo(res);
  return socket_fd;
}

int joinChatSession(int server_fd, clientData *cData){
  pduJoin p;
  p.opCode = JOIN;
  p.id = (uint8_t *)cData->username;
  p.idSize = strlen(cData->username);
  int bufferSize = 0;
  uint8_t *buffer = pduCreator_join(&p, &bufferSize);

  int ret = facade_writeToSocket(server_fd, buffer, bufferSize);
  if (ret == -1){
    return -1;
  }
  free(buffer);
  return 0;
}

int printServerParticipants(int server_fd, clientData *cData){
  genericPdu *pdu = getDataFromSocket(server_fd);

  if (pdu == NULL){
    fprintf(stderr, "Unable to read data from socket: %s\n", strerror(errno));
  }

  if (pdu->opCode != PARTICIPANTS){
    fprintf(stderr, "Invalid packet received from Server.\n"
                    "Expected: %d\n"
                    "Actual: %d\n", PARTICIPANTS, pdu->opCode);
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  pduParticipants *p = (pduParticipants *) pdu;

  fprintf(stdout, "-----------------------------------------------------------------\n");
  fprintf(stdout, "Online users: \n");

  for (int i = 0; i < p->noOfIds; i++){
    fprintf(stdout, "%s\n", p->ids[i]);
  }

  return 0;
}

void readInputFromUser(){
  size_t buffSize = 0;
  char *buffer = NULL;
  bool active = true;
  while (active){

    if (getline(&buffer, &buffSize, stdin) == -1){
      fprintf(stderr, "Failed to read data from user: %s\n", strerror(errno));
      break;
    }
  }
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

  epoll_fd = epoll_create1(0);

  if (epoll_fd == -1){
    fprintf(stderr,"Failed to create epoll FD -  %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // Setup Epoll
  ev.data.fd = server_fd;
  ev.events = EPOLLIN | EPOLLONESHOT | EPOLLEXCLUSIVE;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD,server_fd, &ev);

  readerInfo rInfo;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;
  rInfo.packetList = NULL;

  // Start chat session
  ret = pthread_create(&clientThread, NULL, waitForIncomingMessages, (void *)&rInfo);
  if (ret){
    fprintf(stderr, "Unable to create a pthread. Error: %d\n", ret);
  }

  int *pthread_ret;
  pthread_join(clientThread, (void **)&pthread_ret);
  close(epoll_fd);
  close(server_fd);
}
