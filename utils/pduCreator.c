#include "pduCreator.h"


//Server-nameserver interaction
uint8_t *pduCreator_reg(pduReg *reg, size_t *nByte) {

  if(reg->opCode != REG){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  size_t packetSize =  (WORD_SIZE * 2) + reg->serverNameSize;
  size_t noOfWords = calculateNoOfWords(packetSize);
  *nByte = (noOfWords * WORD_SIZE);
  uint16_t u16;

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);
  u16 = htons(reg->tcpPort);

  memcpy(pduBuffer, &(reg->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(reg->serverNameSize), sizeof(uint8_t));
  memcpy(pduBuffer + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));
  memcpy(pduBuffer + WORD_SIZE, &(reg->ipAddress[0]), sizeof(uint8_t) * 4);
  memcpy(pduBuffer + (WORD_SIZE * 2), reg->serverName, reg->serverNameSize);

  return pduBuffer;
} 

uint8_t *pduCreator_alive(pduAlive *alive, size_t *nByte) {

  if(alive->opCode != ALIVE){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  *nByte = WORD_SIZE;

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), WORD_SIZE);
  uint16_t u16;
  u16 = htons(alive->id);

  memcpy(pduBuffer, &(alive->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(alive->noOfClients), sizeof(uint8_t));
  memcpy(pduBuffer + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));

  return pduBuffer;
}

//Client-nameserver interaction
uint8_t *pduCreator_getList(size_t *nByte) {
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), WORD_SIZE);
  *nByte = WORD_SIZE;
  pduBuffer[0] = GETLIST;
  return pduBuffer;
}


//Client-server interaction
uint8_t *pduCreator_join(pduJoin *join, size_t *nByte){

  if(join->opCode != JOIN){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  size_t packetSize = WORD_SIZE + join->idSize;
  size_t noOfWords = calculateNoOfWords(packetSize);
  *nByte = noOfWords * WORD_SIZE;
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);


  memcpy(pduBuffer, &(join->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE , &(join->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE , join->id, join->idSize);

  return pduBuffer;
}

uint8_t *pduCreator_pJoin(pduPJoin *pJoin, size_t *nByte){

  if(!(pJoin->opCode == PJOIN || pJoin->opCode == PLEAVE)){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  size_t packetSize = (2 * WORD_SIZE) + pJoin->idSize;
  size_t noOfWords = calculateNoOfWords(packetSize);
  *nByte = noOfWords * WORD_SIZE;
  uint32_t u32 = htonl(pJoin->timeStamp);
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(pduBuffer, &(pJoin->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(pJoin->idSize), sizeof(uint8_t));
  memcpy(pduBuffer + WORD_SIZE, &u32, sizeof(uint32_t));
  memcpy(pduBuffer + (2 * WORD_SIZE), pJoin->id, pJoin->idSize);

  return pduBuffer;
}


uint8_t *pduCreator_participants(pduParticipants *par, size_t *nByte) {

  if(par->opCode != PARTICIPANTS){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  uint16_t dataSize = 0;
  for( int i = 0; i < par->noOfIds; i ++){
    dataSize += strlen((char *)par->ids[i]) + 1;
  }

  size_t packetSize = (2 * WORD_SIZE) + dataSize;
  size_t noOfWords = calculateNoOfWords(packetSize);
  *nByte = noOfWords * WORD_SIZE;

  dataSize = htons(dataSize);
  uint16_t strLen = 0;

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(pduBuffer, &(par->opCode), sizeof(uint8_t));
  memcpy(pduBuffer + BYTE_SIZE, &(par->noOfIds), sizeof(uint8_t));
  memcpy(pduBuffer + (2 *BYTE_SIZE) , &dataSize, sizeof(uint16_t));

  int offset = 0;
  for( int i = 0; i < par->noOfIds; i ++){
    strLen = strlen((char *)par->ids[i]) + 1;
    memcpy(pduBuffer + WORD_SIZE + offset, par->ids[i], strLen);
    offset += strLen;
  }

  return pduBuffer;
}

uint8_t *pduCreator_pLeave(pduPLeave *pLeave, size_t *nByte){
  return pduCreator_pJoin(pLeave, nByte);
}

uint8_t *pduCreator_quit(size_t *nByte) {
  uint8_t *pduBuffer = calloc(sizeof(uint8_t), WORD_SIZE);
  *nByte = WORD_SIZE;
  pduBuffer[0] = QUIT;
  return pduBuffer;
}

uint8_t *pduCreator_mess(pduMess *mess, size_t *nByte) {

   if(mess->opCode != MESS){
    fprintf(stderr, "Invalid Operation Code.\n");
    return NULL;
  }

  // Needed due to the variable length of both message and client id;
  size_t noOfWords = 3;
  size_t noOfWordsForMess = calculateNoOfWords(mess->messageSize);
  size_t noOfWordsForId = calculateNoOfWords(mess->idSize);

  noOfWords += noOfWordsForId + noOfWordsForMess;

  *nByte = noOfWords * WORD_SIZE;

  uint8_t *pduBuffer = calloc(1, noOfWords * WORD_SIZE);
  uint16_t u16 = htons(mess->messageSize);
  uint32_t u32 = htonl(mess->timeStamp);
  size_t offset = (3 + noOfWordsForMess) * WORD_SIZE;

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