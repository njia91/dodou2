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


void *getDataFromSocket(int socket_fd, uint8_t opCode){
  void *pdu;

  if(opCode == SLIST){
    pdu = pduReader_SList(socket_fd);
  }

  return pdu;
}

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

pduSList *pduReader_SList(int socket_fd){

  pduSList *p = (pduSList *) calloc(sizeof(pduSList), 1);
  uint8_t buffer[4];
  p->opCode = SLIST;
  int offset = 0;
  readFromSocket(socket_fd, buffer, 3 * BYTE_SIZE);

  if (buffer[0] != 0){
    fprintf(stderr, "Invalid padding for SList Packet \n");
    return NULL;
  }

  memcpy(&p->noOfServers, buffer + (BYTE_SIZE), sizeof(uint16_t));
  p->noOfServers = ntohs(p->noOfServers);

  p->sInfo = calloc(sizeof(serverInfo), p->noOfServers);

  for (int i = 0; i < p->noOfServers; i++){
    readFromSocket(socket_fd, buffer, WORD_SIZE);
    memcpy(&p->sInfo[i].ipAdress, buffer, sizeof(uint32_t));

    readFromSocket(socket_fd, buffer, WORD_SIZE);
    memcpy(&p->sInfo[i].port, buffer, sizeof(uint16_t));
    p->sInfo[i].port = ntohs(p->sInfo[i].port);
    memcpy(&p->sInfo[i].noOfClients, buffer + (2 * BYTE_SIZE), sizeof(uint8_t));
    memcpy(&p->sInfo[i].serverNameLen, buffer + (3 * BYTE_SIZE), sizeof(uint8_t));

    p->sInfo[i].serverName = (uint8_t *) calloc(sizeof(uint8_t), p->sInfo[i].serverNameLen + 1);

    offset = 0;
    for (int j = 0; j < calculateNoOfWords(p->sInfo[i].serverNameLen); j++){
      readFromSocket(socket_fd, buffer, WORD_SIZE);
      // The null-terminator indicates end of of message.
      for (int k = 0; buffer[k] != '\0' && k < 4; k++){
        p->sInfo[i].serverName[offset] = buffer[k];
        offset++;
      }
    }
    p->sInfo[i].serverName[p->sInfo[i].serverNameLen] = '\0';
  }

  return p;
}

//Client-server interaction
pduJoin *pduReader_join(uint8_t *buffer){
  pduJoin *p = calloc(sizeof(pduJoin), 1);

  memcpy(&p->opCode, buffer, sizeof(uint8_t));
  memcpy(&p->idSize, buffer + BYTE_SIZE, sizeof(uint8_t));
  p->id = calloc(sizeof(uint8_t), p->idSize + 1);
  memcpy(p->id, buffer + WORD_SIZE, p->idSize);
  p->id[p->idSize] = '\0';

  return p;
}

pduPJoin *pduReader_pJoin(uint8_t *buffer){
  pduPJoin *p = calloc(sizeof(pduPJoin), 1);

  memcpy(&p->opCode, buffer, sizeof(uint8_t));
  memcpy(&p->idSize, buffer + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&p->timeStamp, buffer + WORD_SIZE, sizeof(uint32_t));
  p->timeStamp = ntohl(p->timeStamp);
  p->id = calloc(sizeof(uint8_t), p->idSize + 1);
  memcpy(p->id, buffer + (2 * WORD_SIZE), p->idSize);
  p->id[p->idSize] = '\0';

  return p;
}

pduPLeave *pduReader_pleave(uint8_t *buffer){
  return pduReader_pJoin(buffer);
}

pduParticipants *pduReader_participants(uint8_t *buffer){
  pduParticipants *p =  calloc(sizeof(pduParticipants), 1);
  uint16_t dataSize;

  memcpy(&p->opCode, buffer, sizeof(uint8_t));
  memcpy(&p->noOfIds, buffer + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&dataSize, buffer + (2 * BYTE_SIZE), sizeof(uint16_t));
  p->ids = calloc(sizeof(uint8_t *), p->noOfIds);
  dataSize = htons(dataSize);
  buffer += WORD_SIZE;
  int idNo = 0;
  int pos = 0;
  int offset = 0;
  while(idNo < p->noOfIds){
    // To find where one ID ends...
    do{
      pos++;
    } while(buffer[pos] != '\0');
    pos = pos + 1;
    p->ids[idNo] = (uint8_t *)calloc(sizeof(uint8_t), pos);
    memcpy(p->ids[idNo], buffer + offset, pos);
    offset += pos;
    idNo++;
  }


  return p;
}

pduQuit *pduReader_quit(uint8_t *buffer){
  pduQuit *p = calloc(sizeof(pduQuit), 1);
  p->opCode = buffer[0];

  return p;
}

pduMess *pduReader_mess(uint8_t *buffer){
  pduMess *p = calloc(sizeof(pduMess), 1);
  int offset = 0;

  memcpy(&p->opCode, buffer, sizeof(uint8_t));
  offset += 2 * BYTE_SIZE;
  memcpy(&p->idSize, buffer + offset, sizeof(uint8_t));
  offset += BYTE_SIZE;
  memcpy(&p->checkSum, buffer + offset, sizeof(uint8_t));
  offset += BYTE_SIZE;
  memcpy(&p->messageSize, buffer + offset, sizeof(uint16_t));
  offset += WORD_SIZE;
  p->messageSize = htons(p->messageSize);
  memcpy(&p->timeStamp, buffer + offset, sizeof(uint32_t));
  p->timeStamp = htonl(p->timeStamp);
  offset += WORD_SIZE;
  p->message = calloc(sizeof(uint8_t), p->messageSize + 1);
  memcpy(p->message, buffer + offset, (p->messageSize));
  p->message[p->messageSize] = '\0';
  offset += calculateNoOfWords(p->messageSize) * WORD_SIZE;
  p->id = calloc(sizeof(uint8_t), p->idSize + 1);
  memcpy(p->id, buffer + offset, p->idSize +1);
  p->id[p->idSize] = '\0';

  return p;
}


void deletePdu(void *pdu){
  uint8_t opCode = *(uint8_t*) pdu;
  if(opCode == SLIST){
    pduSList *p = (pduSList *) pdu;
    for (int i = 0; i < p->noOfServers; i++){
      free(p->sInfo[i].serverName);
    }
    free(p->sInfo);
    free(p);
  }
}
