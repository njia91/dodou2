//
// Created by njia on 2018-08-01.
//

#include "clientSession.h"
#include <pduReader.h>
#include <fcntl.h>
#include <ctype.h>
#include "sysCall_facade.h"
#include "pduCommon.h"


genericPdu *getPduFromSocket(int socket_fd){
  // If failed, terminate socket and shutdown
  genericPdu *pdu =  getDataFromSocket(socket_fd);

  if (pdu == NULL){
    fprintf(stderr, "getDataFromSocket(): returned NULL \n");
  }

  return pdu;
}

int processSocketData(int socket_fd, void *args){
  int allOk = REARM_FD;
  clientData *cData = (clientData *)args;
  if (socket_fd == cData->commonEventFd) {
    eventfd_t value = 0;

    if (facade_read(socket_fd, &value, sizeof(eventfd_t))){
      if (value == TERMINATE){
        allOk = TERMINATE_SESSION;
      }
    } else {
      fprintf(stderr, "Enable to read from eventfd_Read()");
      allOk = TERMINATE_SESSION;
    }

  } else if (socket_fd == STDIN_FILENO){
    allOk = readInputFromUser(cData);
    if (!allOk){
      eventfd_t e = TERMINATE;
      write(cData->commonEventFd, &e, sizeof(eventfd_t));
    }

  } else {
    genericPdu *p = getPduFromSocket(socket_fd);
    if (p == NULL){
      return REMOVE_FD;
    }

    if (p->opCode == PJOIN || p->opCode == PLEAVE){
      pduPJoin *pJoin = (pduPJoin *) p;
      if (strcmp((char *)pJoin->id, cData->username) != 0){
        notifyUserOfChatRoomChanges((pduPJoin *) p);
      }
    } else if (p->opCode == QUIT){
      fprintf(stderr, "\nChat server has terminated the session. Terminating \n");
      allOk = REMOVE_FD;
    } else if (p->opCode == MESS){
      pduMess *mess = (pduMess *) p;
      // Do not display messages sent by receiving client.
      if (strcmp((char *)mess->id, cData->username) != 0){
        allOk = handleMessPdu((pduMess *) p);
      }
    } else if (p->opCode == PARTICIPANTS) {
      pduParticipants *participants = (pduParticipants *) p;
      printServerParticipants(participants);
    }
    deletePdu(p);
  }
  return allOk;
}

int readInputFromUser(clientData *cData) {
  size_t buffSize = 0;
  char *buffer = NULL;
  int active = REARM_FD;
  ssize_t ret = 0;
  fflush(stdin);

  if (getline(&buffer, &buffSize, stdin) == -1){
    fprintf(stderr, "Failed to read data from user: %s\n", strerror(errno));
    active = REMOVE_FD;
  } else {
    if (strcmp(buffer, "QUIT\n") == 0){
      active = TERMINATE_SESSION;
      uint8_t *pduBuffer = pduCreator_quit(&buffSize);
      ret = facade_write(cData->server_fd, pduBuffer, buffSize);
      free(pduBuffer);
      if (ret != buffSize){
        fprintf(stderr, "Unable to write all data to socket.\n");
      }
      write(cData->commonEventFd, &TERMINATE, sizeof(uint64_t));
    } else {
      char *text = calloc(strlen(buffer) - 1, sizeof(char));
      memcpy(text, buffer, strlen(buffer) - 1);
      // Prepare Message PDU
      pduMess mess;
      size_t size = 0;
      mess.opCode = MESS;
      mess.id = NULL;
      mess.idSize =  0;
      mess.message = (uint8_t *) text;
      mess.messageSize = (uint16_t) strlen(text);
      mess.timeStamp = 0;

      uint8_t *packet = pduCreator_mess(&mess, &size);
      // Send package
      ret = facade_write(cData->server_fd, packet, size);
      free(packet);
      if (ret != size){
        fprintf(stderr, "%s: Unable to write all data to socket. Size %zd  ret %zd\n",__func__, size, ret);
        fprintf(stderr, "Errno : %s \n", strerror(errno));
        if (errno == EBADF){
          active = TERMINATE_SESSION;
        }
      }
    }
  }
  free(buffer);
  return active;
}

void notifyUserOfChatRoomChanges(pduPJoin *pJoin){
  struct tm *timeInfo = localtime((time_t *) &pJoin->timeStamp);
  char timeString[20];
  convertTimeToString(timeString, timeInfo);
  fprintf(stdout, "%s [Server notification] User %s has %s the chat room \n", timeString, pJoin->id, (pJoin->opCode == PJOIN) ? "joined" : "left");
}

bool handleMessPdu(pduMess *mess){
  if (mess->isCheckSumOk){
    struct tm *timeInfo = localtime((time_t *) &mess->timeStamp);
    char timeString[TIMESTR_LENGTH];
    convertTimeToString(timeString, timeInfo);
    if (mess->idSize == 0){
      fprintf(stdout, "\n%s [SERVER MESSAGE] > %s\n", timeString, mess->message);
    } else {
      fprintf(stdout, "%s [%s] %s\n", timeString, mess->id, mess->message);
    }
    return true;
  } else {
    fprintf(stderr, "%s: Invalid checksum packet checksum.\n",__func__);
    return false;
  }
}

int setupConnectionToServer(const uint8_t *ip, const char *port) {
  int socket_fd = -1;
  struct addrinfo* res=0;
  struct addrinfo *rp = NULL;
  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  hints.ai_flags=AI_ADDRCONFIG;

  int ret = facade_getAddrinfo((char *) ip, port, &hints, &res);

  if (ret){
    fprintf(stderr, "setupConnectionToSever(): %s \n", gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  // Try each address until we connect
  for (rp = res; rp != NULL; rp = rp->ai_next){
    socket_fd = facade_createSocket(&res);
    if (socket_fd == -1){
      continue;
    }

    if (facade_connect(socket_fd, &res) != -1){
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

int joinChatSession(int server_fd, inputArgs *inArgs){
  pduJoin p;
  p.opCode = JOIN;
  p.id = (uint8_t *)inArgs->username;
  p.idSize = (uint8_t) strlen(inArgs->username);
  size_t bufferSize = 0;
  uint8_t *buffer = pduCreator_join(&p, &bufferSize);

  ssize_t ret = facade_write(server_fd, buffer, bufferSize);
  if (ret == -1){
    fprintf(stderr, "Could not write data to socket \n");
    return -1;
  } else if (ret != bufferSize){
    fprintf(stderr, "Could not write all data to socket \n");
  }
  free(buffer);
  return 0;
}

int printServerParticipants(pduParticipants *p){
  fprintf(stdout, "-----------------------------------------------------------------\n");
  fprintf(stdout, "Welcome! \n");
  fprintf(stdout, "Online users: \n");

  for (int i = 0; i < p->noOfIds; i++){
    fprintf(stdout, "\t%s\n", p->ids[i]);
  }
  fprintf(stdout, "-----------------------------------------------------------------\n");
  return 0;
}

void startChatSession(inputArgs *inArgs){
  // Connect to server
  pthread_t receivingThread;
  int event_fd;
  int epoll_fd;

  int server_fd = setupConnectionToServer(inArgs->ipAdress, inArgs->port);

  // Listen on server socket and join.
  int ret = joinChatSession(server_fd, inArgs);

  if (ret == -1){
    fprintf(stderr," Failed to send JOIN PDU to server: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  epoll_fd = epoll_create1(0);

  if (epoll_fd == -1){
    fprintf(stderr,"Failed to create epoll FD -  %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // Setup Epoll
  struct epoll_event ev_server;
  ev_server.data.fd = server_fd;
  ev_server.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev_server);

  // Add stdin
  struct epoll_event ev_stdin;
  ev_stdin.data.fd = STDIN_FILENO;
  ev_stdin.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev_stdin);

  // Add inter-thread communication FD.
  event_fd = eventfd(0, O_NONBLOCK);
  struct epoll_event ev_ITC;
  ev_ITC.data.fd = event_fd;
  ev_ITC.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &ev_ITC);

  facade_setToNonBlocking(server_fd);

  clientData cData;
  cData.commonEventFd = event_fd;
  cData.server_fd = server_fd;
  cData.username = inArgs->username;

  readerInfo rInfo;
  rInfo.args = &cData;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;
  rInfo.numOfActiveFds = 1; // Only counting the Server socket.

  fflush(stdin);

  // Start chat session
  ret = pthread_create(&receivingThread, NULL, waitForIncomingMessages, (void *)&rInfo);
  if (ret){
    fprintf(stderr, "Unable to create a pthread. Error: %d\n", ret);
  }

  pthread_join(receivingThread, NULL);
  fprintf(stdout, "Terminating session. \n");
  close(epoll_fd);
}
