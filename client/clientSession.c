//
// Created by njia on 2018-08-01.
//

#include "clientSession.h"
#include <pduReader.h>
#include <fcntl.h>
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

bool processSocketData(int socket_fd, void *args){
  bool allOk = true;
  clientData *cData = (clientData *)args;

  if (socket_fd == cData->commonEventFd) {
    fprintf(stderr, "Terminate RECEIVED FROM PARTNER THREAD \n");
    eventfd_t value = 0;

    if (facade_read(socket_fd, &value, sizeof(eventfd_t))){
      if (value == TERMINATE){
        allOk = false;
      }
    } else {
      fprintf(stderr, "Enable to read from eventfd_Read()");
      allOk = false;
      // TODO Also set allOk to false?
    }

  } else if (socket_fd == STDIN_FILENO){
    allOk = readInputFromUser(cData);
    if (!allOk){
      eventfd_t e = TERMINATE;
      write(cData->commonEventFd, &e, sizeof(eventfd_t));
    }

  } else {
    printf("\nHandle Message from Server \n");
    genericPdu *p = getPduFromSocket(socket_fd);
    if (p == NULL){
      return false;
    }

    if (p->opCode == PJOIN || p->opCode == PLEAVE){
      notifyUserOfChatRoomChanges((pduPJoin *) p);
    } else if (p->opCode == QUIT){
      fprintf(stderr, "\nChat server has terminated the session. Terminating \n");
      allOk = false;
    } else if (p->opCode == MESS){
      handleMessPdu((pduMess *) p);
    }

    deletePdu(p);
  }

  printf("\n End of processSocketData... \n");
  return allOk;
}

bool readInputFromUser(clientData *cData) {
  size_t buffSize = 0;
  char *buffer = NULL;
  bool active = true;
  ssize_t ret = 0;
  fflush(stdin);
  fprintf(stdout, "Reading input from user...  ! \n");
  if (getline(&buffer, &buffSize, stdin) == -1){
    fprintf(stderr, "Failed to read data from user: %s\n", strerror(errno));
    active = false;

  } else {
    fprintf(stdout, "STDIN DATA %s  SIZE OF %zd buffsize %zd\n",buffer, sizeof(buffer), buffSize);


    if (!(strcmp(buffer, "quit\n") && strcmp(buffer, "QUIT\n"))){
      active = false;
      uint8_t *pduBuffer = pduCreator_quit(&buffSize);
      ret = facade_write(cData->server_fd, pduBuffer, buffSize);
      free(pduBuffer);
      printf("QUIT QUIT QUIT \n");
      if (ret != buffSize){
        fprintf(stderr, "Unable to write all data to socket.\n");
      }
    } else {
      printf("\nTJOO HEEEJ1111 \n");
      pduMess mess;
      size_t size = 0;
      //char str[] = "Michael";
      mess.opCode = MESS;
      mess.id = NULL;
      mess.idSize =  0;
      mess.message = buffer;
      mess.messageSize = (uint16_t) strlen(buffer);
      mess.timeStamp = 0;

      uint8_t *packet = pduCreator_mess(&mess, &size);

      for (int i = 0; i < size; i++){
        printf("%d - ", packet[i]);
      }
      printf("\n");
      for (int i = 0; i < size; i++){
        printf("%c - ", packet[i]);
      }
      printf("\n");
      ret = facade_write(cData->server_fd, packet, size);
      if (ret != size){
        fprintf(stderr, "%s: Unable to write all data to socket. Size %zd  ret %zd\n",__func__, size, ret);
        fprintf(stderr, "Errno : %s \n", strerror(errno));
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
  fprintf(stdout, "%s > User %s has %s the chat room \n", timeString, pJoin->id, pJoin->opCode == PJOIN ? "joined" : "left");
}

void handleMessPdu(pduMess *mess){
  if (mess->isCheckSumOk){
    struct tm *timeInfo = localtime((time_t *) &mess->timeStamp);
    char timeString[20];
    convertTimeToString(timeString, timeInfo);
    fprintf(stdout, "%s > [%s] %s \n", timeString, mess->id, mess->message);
  } else {
    fprintf(stderr, "%s: Invalid checksum....\n",__func__);
    fprintf(stderr, "OPCODE %u\n", mess->opCode);
    fprintf(stderr, "Id %u\n", mess->idSize);
    fprintf(stderr, "Id %s\n", mess->id);
    fprintf(stderr, "Mess %u\n", mess->messageSize);
    fprintf(stderr, "Mess %s\n", mess->message);
    fprintf(stderr, "Time %lu\n", mess->timeStamp);
  }
}

int setupConnectionToServer(const uint8_t *ip, const char *port) {
  int socket_fd;
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
    if (socket_fd ==-1){
      continue;
    }

    if(facade_connect(socket_fd, &res) != -1){
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
  p.idSize = strlen(inArgs->username);
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

int printServerParticipants(int server_fd, inputArgs *inArgs){
  genericPdu *pdu = getDataFromSocket(server_fd);

  if (pdu == NULL){
    fprintf(stderr, "Unable to read data from socket: %s\n", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
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
  fprintf(stdout, "Welcome! \n");
  fprintf(stdout, "Online users: \n");

  for (int i = 0; i < p->noOfIds; i++){
    fprintf(stdout, "\t%s\n", p->ids[i]);
  }
  fprintf(stdout, "-----------------------------------------------------------------\n");
  deletePdu(pdu);
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

  ret = printServerParticipants(server_fd, inArgs);

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
  printf(" EVENT FD %d \n", event_fd);

  //facade_setToNonBlocking(STDIN_FILENO);
  facade_setToNonBlocking(server_fd);

  clientData cData;
  cData.commonEventFd = event_fd;
  cData.server_fd = server_fd;
  cData.username = inArgs->username;


  readerInfo rInfo;
  rInfo.args = &cData;
  rInfo.epoll_fd = epoll_fd;
  rInfo.func = processSocketData;


  fflush(stdin);

  // Start chat session
  ret = pthread_create(&receivingThread, NULL, waitForIncomingMessages, (void *)&rInfo);
  if (ret){
    fprintf(stderr, "Unable to create a pthread. Error: %d\n", ret);
  }

  //readInputFromUser(server_fd);
  //waitForIncomingMessages((void *)&rInfo);

  pthread_join(receivingThread, NULL);
  fprintf(stdout, "EXIT !!! \n");
  close(epoll_fd);
  //close(server_fd);
}
