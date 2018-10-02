//
// Created by njia on 2018-07-23.
// Mock object for pduReader.h
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "pduReader.h"

genericPdu * getDataFromSocket(int socket_fd) {
  return (void *)mock();
}

//Server-nameserver interaction
pduReg *pduReader_req(int socket_fd);

pduAlive *pduReader_alive(int socket_fd);

//Client-nameserver interaction
pduAck *pduReader_ack(int socket_fd);

pduSList *pduReader_SList(int socket_fd){
  return (pduSList *)mock();
}

//Client-server interaction
pduJoin *pduReader_join(int socket_fd);

pduPJoin *pduReader_pJoin(int socket_fd);

pduPLeave *pduReader_pleave(int socket_fd);

pduParticipants *pduReader_participants(int socket_fd);

pduQuit *pduReader_quit(int socket_fd);

pduMess *pduReader_mess(int socket_fd);

// Copy paste from target code.
void deletePdu(genericPdu *pdu){
  uint8_t opCode = pdu->opCode;

  if (opCode == SLIST){
    pduSList *pSList = (pduSList *) pdu;
    for (int i = 0; i < pSList->noOfServers; i++){
      free(pSList->sInfo[i].serverName);
    }
    free(pSList->sInfo);
    free(pSList);
  } else if (opCode == PARTICIPANTS){
    pduParticipants *pParticipants = (pduParticipants *) pdu;
    for(int i = 0; i < pParticipants->noOfIds; i++){
      free(pParticipants->ids[i]);
    }
    free(pParticipants->ids);
    free(pParticipants);
  } else if (opCode == PJOIN || opCode == PLEAVE){
    pduPJoin *pJoin = (pduPJoin *) pdu;
    free(pJoin->id);
    free(pJoin);
  } else if (opCode == QUIT || opCode == GETLIST){
    free(pdu);
  } else if (opCode == MESS) {
    pduMess *pMess = (pduMess *) pdu;
    free(pMess->message);
    free(pMess->id);
    free(pMess);
  }
}