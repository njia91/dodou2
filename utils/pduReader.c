//
// Created by njia on 2018-05-20.
//


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "pduReader.h"
#include "pduCommon.h"

// TODO Check return values from read() and calloc().

genericPdu *getUdpDataFromSocket(int socket_fd) {
  genericPdu *pdu = NULL;

  uint8_t buffer[WORD_SIZE];
  ssize_t ret = facade_read(socket_fd, &buffer, WORD_SIZE);
  uint8_t opCode = buffer[0];

  //printf("WE GOT THIS OP CODE BACK %u   Size %zd\n", opCode, ret);

  if (ret == 0){
    fprintf(stderr, "%s: Could not read data from Socket\n.", __func__);
    return NULL;
  } else if (ret == -1){
    fprintf(stderr, "%s: %s \n",__func__, strerror(errno));
  }
  
  if (opCode == ACK) {
    pdu = (genericPdu*) pduReader_ack(buffer);
  } else if (opCode == NOTREQ) {
    pdu = (genericPdu*) pduReader_notReg(buffer);
  } else {
    fprintf(stderr, "Received unknown message with opCode:%d\n", opCode);
    return NULL;
  }

  return pdu;
}

genericPdu *getDataFromSocket(int socket_fd) {
  genericPdu *pdu = NULL;

  uint8_t opCode = 0;

  ssize_t ret = facade_read(socket_fd, &opCode, 1);

  //printf("WE GOT THIS OP CODE BACK %u   Size %zd\n", opCode, ret);

  if (ret == 0){
    fprintf(stderr, "%s: Could not read data from Socket.\n", __func__);
    return NULL;
  } else if (ret == -1){
    fprintf(stderr, "%s: %s \n",__func__, strerror(errno));
  }


  if(opCode == SLIST){
    pdu = (genericPdu *) pduReader_SList(socket_fd);
  }  else if (opCode == JOIN){
    pdu = (genericPdu *) pduReader_join(socket_fd);
  }  else if (opCode == PJOIN) {
    pdu = (genericPdu *) pduReader_pJoin(socket_fd);
  } else if (opCode == GETLIST){
    pdu = calloc(sizeof(pduGetList), 1);
    pdu->opCode = GETLIST;
  } else if (opCode == PARTICIPANTS){
    pdu = (genericPdu *) pduReader_participants(socket_fd);
  } else if (opCode == MESS){
    pdu = (genericPdu *) pduReader_mess(socket_fd);
  } else if (opCode == QUIT){
    pdu = (genericPdu *)calloc(sizeof(pduQuit), 1);
    pdu->opCode = QUIT;
  } else if (opCode == PLEAVE){
    pdu = (genericPdu *) pduReader_pLeave(socket_fd);
  } else if (opCode == ALIVE){
    fprintf(stderr, "Received ALIAVE PDU! wtf?\n");
  } else if (opCode == NOTREQ){
    fprintf(stderr, "Received NOTREQ PDU! \n");
  }


  return pdu;
}

//Server-nameserver interaction
pduAck *pduReader_ack(uint8_t* buffer){
  pduAck *p = calloc(sizeof(pduReg), 1);
  p->opCode = ACK;

  if (buffer[1] != 0){
    fprintf(stderr, "Invalid padding for ACK Packet %d\n", buffer[0]);
    return NULL;
  }

  memcpy(&p->id, buffer + (2 * BYTE_SIZE), sizeof(uint16_t));
  p->id = ntohs(p->id);
  return p;
}

pduNotReq *pduReader_notReg(uint8_t* buffer) {
  pduNotReq *p = calloc(sizeof(pduNotReq), 1);
  p->opCode = NOTREQ;

  if (buffer[1] != 0) {
    fprintf(stderr, "Invalid padding for NOTREG Packet %d\n", buffer[0]);
    return NULL;
  }

  memcpy(&p->id, buffer + (2 * BYTE_SIZE), sizeof(uint16_t));
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
      memset(buffer, 0, WORD_SIZE);
      facade_read(socket_fd, buffer, WORD_SIZE);
      // The null-terminator indicates end of of message.
      for (int k = 0; k < WORD_SIZE && buffer[k] != '\0'; k++){
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
    memset(buffer, 0, WORD_SIZE);
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; k < WORD_SIZE && buffer[k] != '\0'; k++){
      p->id[offset] = buffer[k];
      offset++;
    }
  }
  p->id[p->idSize] = '\0';
  return p;
}

pduPJoin *pduReader_pJoin(int socket_fd){
  return pduReader_pJoinLeave(socket_fd, PJOIN);
}

pduPLeave *pduReader_pLeave(int socket_fd){
  return pduReader_pJoinLeave(socket_fd, PLEAVE);
}

pduPJoin *pduReader_pJoinLeave(int socket_fd, uint8_t opCode){
  pduPJoin *p = calloc(sizeof(pduPJoin), 1);
  uint8_t buffer[WORD_SIZE];
  int offset = 0;

  p->opCode = opCode;

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
    memset(buffer, 0, WORD_SIZE);
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; k < WORD_SIZE && buffer[k] != '\0'; k++){
      p->id[offset] = buffer[k];
      offset++;
    }
  }
  p->id[p->idSize] = '\0';

  return p;
}

pduParticipants *pduReader_participants(int socket_fd){
  pduParticipants *p =  calloc(1, sizeof(pduParticipants));
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
    memset(buffer, 0, WORD_SIZE);
    facade_read(socket_fd, buffer, WORD_SIZE);
    // The null-terminator indicates end of of an ID.
    for (int k = 0; k < WORD_SIZE && idNo < p->noOfIds; k++){
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
  pduMess *p = (pduMess *) calloc(sizeof(pduMess), 1);
  p->opCode = MESS;
  uint8_t buffer[WORD_SIZE];
  int offset = 0;
  uint8_t givenCheckSum = 0;
  uint16_t calculatedChecksum = p->opCode;

  facade_read(socket_fd, buffer, 3 * BYTE_SIZE);
  calculatedChecksum += calculateCheckSum((void *) buffer, 3 * BYTE_SIZE);

  if (buffer[0] != 0){
    fprintf(stderr, "Invalid padding for JOIN Packet %d\n", buffer[0]);
    return NULL;
  }

  memcpy(&p->idSize, buffer + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&givenCheckSum, buffer + 2 * BYTE_SIZE, sizeof(uint8_t));


  facade_read(socket_fd, buffer, WORD_SIZE);
  calculatedChecksum += calculateCheckSum((void *) buffer, WORD_SIZE);

  memcpy(&p->messageSize, buffer , sizeof(uint16_t));
  p->messageSize = htons(p->messageSize);

  if (buffer[2] != 0 || buffer[3] != 0){
    fprintf(stderr, "Invalid padding for MESSAGE Packet \n");
    return NULL;
  }

  facade_read(socket_fd, buffer, WORD_SIZE);
  calculatedChecksum += calculateCheckSum((void *) buffer, WORD_SIZE);

  memcpy(&p->timeStamp, buffer, sizeof(uint32_t));
  p->timeStamp = htonl(p->timeStamp);

  p->message = calloc(sizeof(uint8_t), p->messageSize + 1);
  for (int i = 0; i < calculateNoOfWords(p->messageSize); i++){
    memset(buffer, 0, WORD_SIZE);
    facade_read(socket_fd, buffer, WORD_SIZE);
    calculatedChecksum += calculateCheckSum((void *) buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0;k < WORD_SIZE && buffer[k] != '\0'; k++){
      p->message[offset] = buffer[k];
      offset++;
    }
  }
  p->message[p->messageSize] = '\0';
  offset = 0;

  p->id = calloc(sizeof(uint8_t), p->idSize + 1);

  for (int i = 0; i < calculateNoOfWords(p->idSize); i++){
    memset(buffer, 0, WORD_SIZE);
    facade_read(socket_fd, buffer, WORD_SIZE);
    calculatedChecksum += calculateCheckSum((void *) buffer, WORD_SIZE);
    // The null-terminator indicates end of of message.
    for (int k = 0; k < WORD_SIZE && buffer[k] != '\0'; k++){
        p->id[offset] = buffer[k];
        offset++;
    }
  }
  p->id[p->idSize] = '\0';

  while (calculatedChecksum > UINT8_MAX) {
    calculatedChecksum -= 255;
  }

  if(calculatedChecksum == UINT8_MAX){
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
  } else if (opCode == JOIN){
    pduJoin *join = (pduJoin *) pdu;
    free(join->id);
    free(join);
  }
}
