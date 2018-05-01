#include "pduCreator.h"


//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req){

  if(req->opCode != REQ){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int packetSize =  WORD_SIZE + req->serverNameSize;
  int noOfWords = calculateNoOfWords(packetSize);
  uint16_t u16;

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);
  u16 = htons(req->tcpPort);

  memcpy(pduBuffer, &(req->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(req->serverNameSize), sizeof(uint8_t));
  memcpy(pduBuffer + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));
  memcpy(pduBuffer + WORD_SIZE, req->serverName, req->serverNameSize);

  return pduBuffer;
} 

uint8_t *pduCreator_alive(pduAlive *alive){

  if(alive->opCode != ALIVE){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), WORD_SIZE);
  uint16_t u16;
  u16 = htons(alive->id);

  memcpy(pduBuffer, &(alive->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(alive->noOfClients), sizeof(uint8_t));
  memcpy(pduBuffer + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));

  return pduBuffer;
}

//Client-nameserver interaction
uint8_t *pduCreator_getList(){
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), WORD_SIZE);
  pduBuffer[0] = GETLIST;
  return pduBuffer;
}


//Client-server interaction
uint8_t *pduCreator_join(pduJoin *join){

  if(join->opCode != JOIN){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int packetSize = WORD_SIZE + join->idSize;
  int noOfWords = calculateNoOfWords(packetSize);
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(pduBuffer, &(join->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE , &(join->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE , join->id, join->idSize);

  return pduBuffer;
}

uint8_t *pduCreator_pJoin(pduPJoin *pJoin){

  if(!(pJoin->opCode == PJOIN || pJoin->opCode == PLEAVE)){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int packetSize = (2 * WORD_SIZE) + pJoin->idSize;
  int noOfWords = calculateNoOfWords(packetSize);
  uint32_t u32 = htonl(pJoin->timeStamp);
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(pduBuffer, &(pJoin->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(pJoin->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE, &u32, sizeof(uint32_t));
  memcpy(pduBuffer + (2 * WORD_SIZE), pJoin->id, pJoin->idSize);

  return pduBuffer;
}


uint8_t *pduCreator_participants(pduParticipants *par){

  if(par->opCode != PARTICIPANTS){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int packetSize = (2 * WORD_SIZE) + par->dataSize;
  int noOfWords = calculateNoOfWords(packetSize);

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);
  uint16_t u16 = htons(par->dataSize);

  memcpy(pduBuffer, &(par->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(par->noOfIds), sizeof(uint8_t));
  memcpy(pduBuffer + (2 *BYTE_SIZE) , &u16, sizeof(uint16_t));
  memcpy(pduBuffer + WORD_SIZE, par->ids, par->dataSize);

  return pduBuffer;
}

uint8_t *pduCreator_pleave(pduPLeave *pLeave){
  return pduCreator_pJoin(pLeave);
}

uint8_t *pduCreator_quit(){
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), WORD_SIZE);
  pduBuffer[0] = GETLIST;
  return pduBuffer;
}

uint8_t *pduCreator_mess(pduMess *mess){

   if(mess->opCode != MESS){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  //Special solution due to the variable length of both message and client id;
  int noOfWords = 3;
  int noOfWordsForMess = calculateNoOfWords(mess->messageSize);
  int noOfWordsForId = calculateNoOfWords(mess->idSize);
  noOfWords += noOfWordsForId + noOfWordsForMess;

  uint8_t *pduBuffer = calloc(noOfWords * WORD_SIZE, sizeof(uint8_t));
  uint16_t u16 = htons(mess->messageSize);
  uint32_t u32 = htonl(mess->timeStamp);
  int offset = (3 + noOfWordsForMess) * WORD_SIZE;

  memcpy(pduBuffer, &(mess->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + (2 * BYTE_SIZE), &(mess->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE, &u16, sizeof(uint16_t));
  memcpy(pduBuffer + (2 * WORD_SIZE), &u32, sizeof(uint32_t));
  memcpy(pduBuffer + (3 * WORD_SIZE), mess->message, mess->messageSize);
  memcpy(pduBuffer + offset, mess->id, mess->idSize);

  uint8_t u8 = ~calculateCheckSum(pduBuffer, noOfWords * WORD_SIZE);
  memcpy(pduBuffer + (3 * BYTE_SIZE), &u8, sizeof(uint8_t));

  return pduBuffer;
}

int calculateNoOfWords(int packetSize){
  if (packetSize % WORD_SIZE == 0){
    return (packetSize / 4);
  } else {
    return (packetSize / 4) + 1;
  }
}