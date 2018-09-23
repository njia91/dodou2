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


void *getDataFromSocket(int socket_fd) {
  genericPdu *pdu = NULL;

  uint8_t opCode = UINT8_MAX;

  int ret = facade_read(socket_fd, &opCode, 1);

  if (ret == 0){
    fprintf(stderr, "Could not read data from Socket\n.");
    return NULL;
  }

  if(opCode == SLIST){
    pdu = pduReader_SList(socket_fd);
  } else if (opCode == REQ){
    pdu = pduReader_req(socket_fd);
  } else if (opCode == ACK){
    pdu = pduReader_ack(socket_fd);
  }  else if (opCode == JOIN){
    pdu = pduReader_join(socket_fd);
  }  else if (opCode == PJOIN) {
    pdu = pduReader_pJoin(socket_fd);
  } else if (opCode == GETLIST){
    pdu = calloc(1, sizeof(GETLIST));
    pdu->opCode = GETLIST;
  }


  return pdu;
}

//Server-nameserver interaction
pduReq *pduReader_req(int socket_fd){
  pduReq *p = calloc(sizeof(pduReq), 1);
  uint8_t buffer[WORD_SIZE];
  p->opCode = REQ;
  int offset = 0;
  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);

  memcpy(&p->serverNameSize, buffer, sizeof(uint8_t));
  memcpy(&p->tcpPort, buffer + BYTE_SIZE, sizeof(uint16_t));

  p->serverName = calloc(sizeof(uint8_t), p->serverNameSize + 1);

  for (int i = 0; i < calculateNoOfWords(p->serverNameSize); i++){
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; buffer[k] != '\0' && k < WORD_SIZE; k++){
      p->serverName[offset] = buffer[k];
      offset++;
    }
  }
  p->serverName[p->serverNameSize] = '\0';
  p->tcpPort = ntohs(p->tcpPort);

  return p;
}

pduAck *pduReader_ack(int socket_fd){
  pduAck *p = calloc(sizeof(pduReq), 1);
  p->opCode = ACK;
  uint8_t buffer[WORD_SIZE];

  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);

  if (buffer[0] != 0){
    fprintf(stderr, "Invalid padding for ACK Packet %d\n", buffer[0]);
    return NULL;
  }

  memcpy(&p->id, buffer +  BYTE_SIZE, sizeof(uint16_t));
  p->id = ntohs(p->id);
  return p;
}

//Client-nameserver interaction

pduSList *pduReader_SList(int socket_fd){

  pduSList *p = (pduSList *) calloc(sizeof(pduSList), 1);
  uint8_t buffer[WORD_SIZE];
  p->opCode = SLIST;
  int offset = 0;
  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);

  if (buffer[0] != 0){
    fprintf(stderr, "Invalid padding for SLIST Packet %d\n", buffer[0]);
    return NULL;
  }

  memcpy(&p->noOfServers, buffer + (BYTE_SIZE), sizeof(uint16_t));
  p->noOfServers = ntohs(p->noOfServers);

  p->sInfo = calloc(sizeof(serverInfo), p->noOfServers);

  for (int i = 0; i < p->noOfServers; i++){
    facade_read(socket_fd, buffer, WORD_SIZE);
    memcpy(&p->sInfo[i].ipAdress, buffer, sizeof(uint32_t));

    facade_read(socket_fd, buffer, WORD_SIZE);
    memcpy(&p->sInfo[i].port, buffer, sizeof(uint16_t));
    p->sInfo[i].port = ntohs(p->sInfo[i].port);
    memcpy(&p->sInfo[i].noOfClients, buffer + (2 * BYTE_SIZE), sizeof(uint8_t));
    memcpy(&p->sInfo[i].serverNameLen, buffer + (3 * BYTE_SIZE), sizeof(uint8_t));

    p->sInfo[i].serverName = (uint8_t *) calloc(sizeof(uint8_t), p->sInfo[i].serverNameLen + 1);

    offset = 0;
    for (int j = 0; j < calculateNoOfWords(p->sInfo[i].serverNameLen); j++){
      facade_read(socket_fd, buffer, WORD_SIZE);
      // The null-terminator indicates end of of message.
      for (int k = 0; buffer[k] != '\0' && k < WORD_SIZE; k++){
        p->sInfo[i].serverName[offset] = buffer[k];
        offset++;
      }
    }
    p->sInfo[i].serverName[p->sInfo[i].serverNameLen] = '\0';
  }

  return p;
}

//Client-server interaction
pduJoin *pduReader_join(int socket_fd){
  pduJoin *p = calloc(sizeof(pduJoin), 1);
  p->opCode = JOIN;
  uint8_t buffer[WORD_SIZE];
  int offset = 0;

  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);
  p->idSize = buffer[0];

  if (buffer[1] != 0 || buffer[2] != 0 ){
    fprintf(stderr, "Invalid padding for JOIN Packet %d\n", buffer[0]);
    return NULL;
  }

  p->id = calloc(sizeof(uint8_t), p->idSize + 1);
  for (int i = 0; i < calculateNoOfWords(p->idSize); i++){
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; buffer[k] != '\0' && k < WORD_SIZE; k++){
      p->id[offset] = buffer[k];
      offset++;
    }
  }
  p->id[p->idSize] = '\0';
  return p;
}

pduPJoin *pduReader_pJoin(int socket_fd){
  pduPJoin *p = calloc(sizeof(pduPJoin), 1);
  uint8_t buffer[WORD_SIZE];
  int offset = 0;

  p->opCode = PJOIN;

  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);

  memcpy(&p->idSize, buffer , sizeof(uint8_t));

  if (buffer[1] != 0 || buffer[2] != 0 ){
    fprintf(stderr, "Invalid padding for JOIN Packet %d\n", buffer[0]);
    return NULL;
  }

  facade_read(socket_fd, buffer, WORD_SIZE);

  memcpy(&p->timeStamp, buffer, sizeof(uint32_t));
  p->timeStamp = ntohl(p->timeStamp);
  p->id = calloc(sizeof(uint8_t), p->idSize + 1);
  for (int i = 0; i < calculateNoOfWords(p->idSize); i++){
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; buffer[k] != '\0' && k < WORD_SIZE; k++){
      p->id[offset] = buffer[k];
      offset++;
    }
  }
  p->id[p->idSize] = '\0';

  return p;
}

pduPLeave *pduReader_pLeave(int socket_fd){
  return pduReader_pJoin(socket_fd);
}

pduParticipants *pduReader_participants(int socket_fd){
  pduParticipants *p =  calloc(sizeof(pduParticipants), 1);
  uint16_t dataSize;
  uint8_t buffer[WORD_SIZE];
  uint8_t idBuffer[256];
  int offset = 0;

  p->opCode = PARTICIPANTS;

  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);

  memcpy(&p->noOfIds, buffer , sizeof(uint8_t));
  memcpy(&dataSize, buffer + BYTE_SIZE, sizeof(uint16_t));
  p->ids = calloc(sizeof(uint8_t *), p->noOfIds);
  dataSize = htons(dataSize);

  int idNo = 0;
  while(idNo < p->noOfIds){
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of an ID.
    for (int k = 0;k < WORD_SIZE && idNo < p->noOfIds; k++){
      idBuffer[offset] = buffer[k];
      offset++;
      // End of an ID, copy string id to PDU struct
      if (buffer[k] == '\0'){
        p->ids[idNo] = calloc(sizeof(uint8_t), offset);
        memcpy(p->ids[idNo], idBuffer, offset);
        idNo++;
        offset=0;

      }
    }
  }
  return p;
}

pduMess *pduReader_mess(int socket_fd){
  pduMess *p = calloc(sizeof(pduMess), 1);
  uint8_t buffer[WORD_SIZE];
  int offset = 0;
  uint8_t givenCheckSum;

  p->opCode = MESS;

  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);

  if (buffer[0] != 0){
    fprintf(stderr, "Invalid padding for JOIN Packet %d\n", buffer[0]);
    return NULL;
  }


  memcpy(&p->idSize, buffer + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&givenCheckSum, buffer + 2 * BYTE_SIZE, sizeof(uint8_t));


  facade_read(socket_fd, buffer, WORD_SIZE);

  memcpy(&p->messageSize, buffer , sizeof(uint16_t));
  p->messageSize = htons(p->messageSize);

  if (buffer[2] != 0 || buffer[3] != 0){
    fprintf(stderr, "Invalid padding for MESSAGE Packet \n");
    return NULL;
  }

  facade_read(socket_fd, buffer, WORD_SIZE);

  memcpy(&p->timeStamp, buffer, sizeof(uint32_t));
  p->timeStamp = htonl(p->timeStamp);

  p->message = calloc(sizeof(uint8_t), p->messageSize + 1);
  for (int i = 0; i < calculateNoOfWords(p->messageSize); i++){
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; buffer[k] != '\0' && k < WORD_SIZE; k++){
      p->message[offset] = buffer[k];
      offset++;
    }
  }
  p->message[p->messageSize] = '\0';
  offset = 0;

  p->id = calloc(sizeof(uint8_t), p->idSize + 1);
  for (int i = 0; i < calculateNoOfWords(p->idSize); i++){
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; buffer[k] != '\0' && k < WORD_SIZE; k++){
      p->id[offset] = buffer[k];
      offset++;
    }
  }
  p->id[p->idSize] = '\0';

  uint8_t calculatedChecksum = calculateCheckSum((void*) &p->opCode, 1);
  calculatedChecksum += calculateCheckSum((void*) &p->idSize, 1);
  calculatedChecksum += calculateCheckSum((void*) &p->messageSize, 2);
  calculatedChecksum += calculateCheckSum((void*) &p->timeStamp, 4);
  calculatedChecksum += calculateCheckSum((void *) p->message,  p->messageSize);
  calculatedChecksum += calculateCheckSum((void *) p->id, p->idSize);

  if(~(calculatedChecksum + givenCheckSum) == false){
    p->isCheckSumOk = true;
  } else {
    p->isCheckSumOk = false;
  }

  return p;
}


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
