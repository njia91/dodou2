//
// Created by njia on 2018-05-20.
//


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <arpa/inet.h>

#include "pduReader.h"
#include "pduCommon.h"


//Server-nameserver interaction
pduReq *pduReader_req(uint8_t *buffer){
  pduReq *p = calloc(sizeof(pduReq), 1);

  memcpy(&p->opCode, buffer, sizeof(uint8_t));
  memcpy(&p->serverNameSize, buffer + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&p->tcpPort, buffer + (2 * BYTE_SIZE ), sizeof(uint16_t));

  p->serverName = calloc(sizeof(uint8_t), p->serverNameSize + 1);
  memcpy(p->serverName, buffer + WORD_SIZE, p->serverNameSize);

  p->serverName[p->serverNameSize] = '\0';
  p->tcpPort = ntohs(p->tcpPort);

  return p;
}

pduAck *pduReader_ack(uint8_t *buffer){
  pduAck *p = calloc(sizeof(pduReq), 1);

  memcpy(&p->opCode, buffer, sizeof(uint8_t));
  memcpy(&p->id, buffer + (2 * BYTE_SIZE), sizeof(uint16_t));

  p->id = ntohs(p->id);

  return p;
}

//Client-nameserver interaction

pduSList *pduReader_SList(uint8_t *buffer){

}

//Client-server interaction
pduJoin *pduReader_join(uint8_t *buffer);

pduPJoin *pduReader_pJoin(uint8_t *buffer);

pduPLeave *pduReader_pleave(uint8_t *buffer);

pduParticipants *pduReader_participants(uint8_t *buffer);

pduQuit *pduReader_quit();

pduMess *pduReader_mess(uint8_t *buffer);
