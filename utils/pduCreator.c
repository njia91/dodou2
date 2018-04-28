#include "pduCreator.h"


//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req){

  if(req->opCode != REQ){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  int reqLen =  WORD_SIZE + req->serverNameLen;
  int noOfWords;
  uint16_t u16;

  if (reqLen % WORD_SIZE == 0){
    noOfWords = (reqLen / 4);
  } else {
    noOfWords = (reqLen / 4) + 1;
  }

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

  memcpy(pduBuffer, &(alive->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(alive->noOfClients), sizeof(uint8_t));
  u16 = htons(alive->id);
  memcpy(pduBuffer + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));

  return pduBuffer;
}

//Client-nameserver interaction
uint8_t *pduCreator_getList();


//Client-server interaction
uint8_t *pduCreator_join(uint8_t idLength, uint8_t *idStr);

uint8_t *pduCreator_pjoin(uint8_t idLength, uint32_t time, uint8_t *idStr);

uint8_t *pduCreator_pleave(uint8_t idLength, uint32_t time, uint8_t *idStr);

uint8_t *pduCreator_participands(uint8_t noOfIds, uint16_t length);

uint8_t *pduCreator_quit();

uint8_t *pduCreator_mess(uint8_t idLength, uint16_t messageLength,
                      uint32_t time, uint8_t *message);