//
// Created by njia on 2018-07-20.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "ThreadingSolution.h"
#include "sysCall_facade.h"
#include "client.h"
#include "pduCommon.h"
#include "pduReader.h"
#include "socketReaderAPI.h"
#include "clientSession.h"
#include <time.h>

/*

 FILE *fp;


   fp = freopen("file.txt", "r+", stdout);

   printf("This text is redirected to file.txt\n");

   fclose(fp);

 freopen("filename", "r", stdin);

*/

pduSList *createSListPdu(){
  pduSList *p;
  p = (pduSList *) calloc(1, sizeof(pduSList));
  p->opCode = SLIST;
  p->noOfServers = 2;
  p->sInfo = calloc(sizeof(serverInfo), p->noOfServers);

  uint8_t ip[2][4] = {{123,0,1,21},
                      {32,0,2,12}};
  const char * serverNames[] = {"Michaels Server",
                                "Anderssons Server"};

  uint16_t port= 123;
  uint8_t noOfClients = 6;
  uint8_t len = 0;

  for (int i = 0; i < p->noOfServers; i++){
    memcpy(&p->sInfo[i].ipAdress, ip[i], sizeof(uint32_t));
    len = strlen(serverNames[i]);
    p->sInfo[i].port = port + i;
    p->sInfo[i].noOfClients = noOfClients + i;
    p->sInfo[i].serverNameLen = len;
    p->sInfo[i].serverName = (uint8_t *) calloc(sizeof(uint8_t), p->sInfo[i].serverNameLen + 1);
    memcpy(p->sInfo[i].serverName, serverNames[i], p->sInfo[i].serverNameLen);
    p->sInfo[i].serverName[p->sInfo[i].serverNameLen] = '\0';
  }

  return p;
}

pduParticipants *createParticipantsPdu(){
  pduParticipants *p;
  p = (pduParticipants *) calloc(1, sizeof(pduParticipants));
  p->opCode = PARTICIPANTS;
  p->noOfIds = 3;
  p->ids = calloc(sizeof(serverInfo), p->noOfIds);

  const char * clientNames[] = {"Michaels Server",
                                "Anderssons Server",
                                "Janne"};

  uint8_t len = 0;
  for (int i = 0; i < p->noOfIds; i++){
    len = strlen(clientNames[i]) + 1;
    p->ids[i] = calloc(sizeof(uint8_t), len);
    memcpy(p->ids[i], clientNames[i], len);
  }

  return p;
}

void clientTest_connectToServer(void **state)
{
  pduSList *pSList;
  pduParticipants *pParticipants;
  struct addrinfo retAddr;
  FILE *fp;
  IS_MOCKOBJECT_SET = false;
  fp = freopen("../testSupport/connectToNs.txt", "r", stdin);

  int fd = 545;
  will_return_always(facade_createSocket, fd);
  will_return_always(facade_connect, 0);
  expect_value(facade_connect, socket_fd, fd);
  expect_value(facade_connect, socket_fd, fd);


  will_return_always(facade_writeToSocket, 1);
 // will_return(facade_readFromSocket, SLIST);
  //will_return(facade_readFromSocket, 1);
  //will_return(facade_readFromSocket, PARTICIPANTS);
  //will_return(facade_readFromSocket, 1);


  pSList = createSListPdu();
  pParticipants = createParticipantsPdu();

  will_return(facade_getAddrinfo, &retAddr);
  will_return(facade_getAddrinfo, 0);
  will_return(facade_getAddrinfo, &retAddr);
  will_return(facade_getAddrinfo, 0);


  will_return(getDataFromSocket, (void *) pSList);
  will_return(getDataFromSocket, (void *) pParticipants);

  char *argv[5] = {"client", "Micke :)", "ns", "123.0.0.1", "1234"};
  client_main(5, argv);

  // Free Slist
  for (int i = 0; i < pSList->noOfServers; i++){
    free(pSList->sInfo[i].serverName);
  }
  free(pSList->sInfo);
  free(pSList);

  // Free Participants
  for(int i = 0; i < pParticipants->noOfIds; i++){
    free(pParticipants->ids[i]);
  }
  free(pParticipants->ids);
  free(pParticipants);
  fclose(fp);
}

void clientTest_recieveAndSendDatafromServer(){
  FILE *fp;
  fp = freopen("../testSupport/dataSentFromServer.txt", "r", stdin);
  IS_MOCKOBJECT_SET = true;

//  readerInfo *rInfo = (readerInfo *)calloc(1, sizeof(readerInfo));
  readerInfo rInfo;

  will_return(facade_epoll_wait, 1);

  rInfo.epoll_fd = 1;
  rInfo.packetList = NULL;
  rInfo.func = processSocketData;


  char id[] = "Michael Åäö";
  char mess[] = "First mess ";
  pduMess *mess1 = calloc(1, sizeof(pduMess));
  mess1->idSize = (uint8_t) strlen(id);
  mess1->messageSize = (uint16_t) strlen(mess);
  mess1->message = calloc(mess1->messageSize, sizeof(uint8_t) + 1);
  memcpy(mess1->message, mess, mess1->messageSize + 1);
  mess1->id = calloc(mess1->idSize, sizeof(uint8_t) + 1);
  memcpy(mess1->id, id, mess1->idSize + 1);
  mess1->isCheckSumOk = 255;
  mess1->opCode = MESS;

  pduPJoin *pJoin = calloc(1, sizeof(pduPJoin));
  pJoin->opCode = PLEAVE;
  pJoin->idSize = (uint8_t) strlen(id);
  time((time_t *)&pJoin->timeStamp);
  pJoin->id = calloc(pJoin->idSize, sizeof(uint8_t) + 1);
  memcpy(pJoin->id, id, pJoin->idSize + 1);

  will_return(getDataFromSocket, pJoin);
  will_return(getDataFromSocket, mess1);
  will_return(getDataFromSocket, NULL);
  will_return_always(facade_epoll_wait, 1);

  waitForIncomingMessages((void *) &(rInfo));

  fclose(fp);
};


int main(int argc, char* argv[]) {
/*  const struct CMUnitTest tests[] = {
          cmocka_unit_test(clientTest_connectToServer),
          cmocka_unit_test(clientTest_recieveAndSendDatafromServer),
          };*/

  const struct CMUnitTest tests[] = {
          cmocka_unit_test(clientTest_recieveAndSendDatafromServer),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
  return 0;
}