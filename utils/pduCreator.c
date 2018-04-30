#include "pduCreator.h"


//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req){

  if(req->opCode != REQ){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int packetSize =  WORD_SIZE + req->serverNameLen;
  int noOfWords = calculateNoOfWordsInPacket(packetSize);
  uint16_t u16;

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);
  u16 = htons(req->tcpPort);

  memcpy(pduBuffer, &(req->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(req->serverNameLen), sizeof(uint8_t));
  memcpy(pduBuffer + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));
  memcpy(pduBuffer + WORD_SIZE, req->serverName, req->serverNameLen);

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
  int noOfWords = calculateNoOfWordsInPacket(packetSize);
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(pduBuffer, &(join->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE , &(join->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE , join->id, join->idSize);

  return pduBuffer;
}

uint8_t *pduCreator_pJoin(pduPJoin *pJoin){

  if(pJoin->opCode != PJOIN){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int packetSize = (2 * WORD_SIZE) + pJoin->idSize;
  int noOfWords = calculateNoOfWordsInPacket(packetSize);
  uint32_t u32 = htonl(pJoin->timeStamp);
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(pduBuffer, &(pJoin->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(pJoin->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE, &u32, sizeof(uint32_t));
  memcpy(pduBuffer + (2 * WORD_SIZE), pJoin->id, pJoin->idSize);

  return pduBuffer;
}

uint8_t *pduCreator_pleave(uint8_t idLength, uint32_t time, uint8_t *idStr);

uint8_t *pduCreator_participands(uint8_t noOfIds, uint16_t length);

uint8_t *pduCreator_quit();

uint8_t *pduCreator_mess(uint8_t idLength, uint16_t messageLength,
                      uint32_t time, uint8_t *message);


int calculateNoOfWordsInPacket(int packetSize){
  if (packetSize % WORD_SIZE == 0){
    return (packetSize / 4);
  } else {
    return (packetSize / 4) + 1;
  }
}