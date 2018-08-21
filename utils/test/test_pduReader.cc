//
// Created by njia on 2018-05-20.
//

#include <gtest/gtest.h>

#include <string>
#include <iostream>


#ifdef __cplusplus
extern "C"{
#endif
  #include "pduCommon.h"
  #include "pduCreator.h"
  #include "pduReader.h"
  #include <stdlib.h>
  #include <time.h>
  #include <stdarg.h>
  #include <stddef.h>
  #include <setjmp.h>
  #include <cmocka.h>
  #include "sysCall_facade.h"
#ifdef __cplusplus
}
#endif

// Necessary evil...
int globalOffset;
uint8_t *globalBuffer;

int facade_writeToSocket(int socket_fd, uint8_t *packet, int size);

int facade_readFromSocket(int socket_fd, uint8_t *buffer, int size){
  memcpy(buffer, globalBuffer + globalOffset, size);
  globalOffset += size;
  return size;
}

class PduReaderTest : public testing::Test
{
  void SetUp(){
    setlocale(LC_CTYPE, "");
  }

  void TearDown(){
    globalOffset = 0;
    globalBuffer = NULL;
  }
};


TEST_F(PduReaderTest, read_reqPacket){
  pduReq r;
  char *serName = (char *)"testHost";
  uint8_t serLen = strlen(serName);

  r.opCode = REQ;
  r.tcpPort = 454;
  r.serverName = (uint8_t *)serName;
  r.serverNameSize = serLen;

  uint8_t *retVal = pduCreator_req(&r);
  globalBuffer = retVal + 1;

  pduReq *ret = pduReader_req(4);

  EXPECT_EQ(ret->opCode, REQ);
  EXPECT_EQ(ret->serverNameSize, serLen);
  EXPECT_EQ(ret->tcpPort, r.tcpPort);
  EXPECT_EQ(strcmp((char *)ret->serverName, (char *)r.serverName), 0);
  globalOffset = 0;
  free(retVal);
  free(ret->serverName);
  free(ret);
}

TEST_F(PduReaderTest, read_ackPacket){
  pduAck r;

  r.opCode = ACK;
  r.id = 454;

  uint16_t u16 = htons(r.id);

  uint8_t *retVal = (uint8_t *)calloc(WORD_SIZE, 1);

  memcpy(retVal, &(r.opCode), sizeof(uint8_t));
  memcpy(retVal + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));

  globalBuffer = retVal + 1;

  pduAck *ret = pduReader_ack(4);

  EXPECT_EQ(ret->opCode, ACK);
  EXPECT_EQ(ret->id, r.id);

  free(ret);
  free(retVal);
}

TEST_F(PduReaderTest, read_sListPacket){
  pduSList r;
  int socket_fd = 7;

  uint16_t noOfServers = 2;
  uint16_t u16;

  uint8_t noClients1 = 4;
  uint8_t noClients2 = 2;

  uint16_t port1 = 1234;
  uint16_t port2 = 4321;

  serverInfo *s = (serverInfo*)calloc(sizeof(serverInfo), noOfServers);


  uint8_t *serName1 = (uint8_t *)"MickeServer";
  uint8_t *serName2 = (uint8_t *)"Andersson";
  uint8_t serLen1 = strlen((char *)serName1);
  uint8_t serLen2 = strlen((char *)serName2);

  s[0].serverName = serName1;
  s[0].noOfClients = noClients1;
  s[0].serverNameLen = serLen1;
  s[0].ipAdress[0] = 132;
  s[0].ipAdress[1] = 0;
  s[0].ipAdress[2] = 32;
  s[0].ipAdress[3] = 1;
  s[0].port = port1;

  s[1].serverName = serName2;
  s[1].noOfClients = noClients2;
  s[1].serverNameLen = serLen2;
  s[0].ipAdress[0] = 123;
  s[0].ipAdress[1] = 0;
  s[0].ipAdress[2] = 32;
  s[0].ipAdress[3] = 1;
  s[1].port = port2;

  r.opCode = SLIST;
  r.sInfo = s;
  r.noOfServers = noOfServers;
  u16 = htons(noOfServers);
  int noOfWords = 1 + (2 * noOfServers) + calculateNoOfWords(serLen1) + calculateNoOfWords(serLen2);
  int offset = 0;
  uint8_t *buffer = (uint8_t *)calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  memcpy(buffer, &(r.opCode), sizeof(uint8_t));
  memcpy(buffer + (2 * BYTE_SIZE), &(u16), sizeof(uint16_t));
  offset = WORD_SIZE;

  //For first server
  memcpy(buffer + offset, &(r.sInfo[0].ipAdress), sizeof(uint32_t));
  offset += WORD_SIZE;
  u16 = htons(r.sInfo[0].port);
  memcpy(buffer + offset, &(u16), sizeof(uint16_t));
  offset += sizeof(uint16_t);
  memcpy(buffer + offset, &(r.sInfo[0].noOfClients), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, &(r.sInfo[0].serverNameLen), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, (r.sInfo[0].serverName), r.sInfo[0].serverNameLen);
  offset += calculateNoOfWords((int)r.sInfo[0].serverNameLen) * WORD_SIZE;

  //For second server
  memcpy(buffer + offset, &(r.sInfo[1].ipAdress), sizeof(uint32_t));
  offset += WORD_SIZE;
  u16 = htons(r.sInfo[1].port);
  memcpy(buffer + offset, &(u16), sizeof(uint16_t));
  offset += sizeof(uint16_t);
  memcpy(buffer + offset, &(r.sInfo[1].noOfClients), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, &(r.sInfo[1].serverNameLen), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, (r.sInfo[1].serverName), r.sInfo[1].serverNameLen);
  offset += r.sInfo[1].serverNameLen;

  pduSList *ret = NULL;
  globalBuffer = buffer + 1;
  ret = pduReader_SList(socket_fd);

  ASSERT_TRUE(ret);

  EXPECT_EQ(ret->opCode, SLIST);
  EXPECT_EQ(ret->noOfServers, r.noOfServers);

  EXPECT_EQ(ret->sInfo[0].noOfClients, s[0].noOfClients);
  EXPECT_EQ(ret->sInfo[0].serverNameLen, s[0].serverNameLen);
  EXPECT_EQ(ret->sInfo[0].port, s[0].port);
  EXPECT_EQ(strcmp((char*)ret->sInfo[0].serverName, (char*)s[0].serverName), 0);
  for (int i = 0; i < 4; i++){
    EXPECT_EQ(ret->sInfo[0].ipAdress[i], s[0].ipAdress[i]);
  }


  EXPECT_EQ(ret->sInfo[1].noOfClients, s[1].noOfClients);
  EXPECT_EQ(ret->sInfo[1].serverNameLen, s[1].serverNameLen);
  EXPECT_EQ(ret->sInfo[1].port, s[1].port);
  EXPECT_EQ(strcmp((char *)ret->sInfo[1].serverName, (char *)s[1].serverName), 0);
  for (int i = 0; i < 4; i++){
    EXPECT_EQ(ret->sInfo[1].ipAdress[i], s[1].ipAdress[i]);
  }

  deletePdu(ret);
  free(s);
  free(buffer);
}

TEST_F(PduReaderTest, read_JoinPdu){
  pduJoin r;
  char str[] = "Micke";
  int bufferSize;
  r.opCode = JOIN;
  r.id = (uint8_t *) str;
  r.idSize = strlen((char * ) r.id);

  uint8_t *retVal = pduCreator_join(&r, &bufferSize);
  globalBuffer = retVal + 1;
  pduJoin *ret = pduReader_join(4);

  EXPECT_EQ(ret->opCode, JOIN);
  EXPECT_EQ(r.idSize, ret->idSize);
  EXPECT_EQ(strcmp((char *) ret->id, (char *)r.id), 0);

  free(ret->id);
  free(ret);
  free(retVal);
}

TEST_F(PduReaderTest, read_pJoinPdu){
  pduPJoin r;
  char str[] = "Micke";
  int bufferSize;
  r.opCode = PJOIN;
  r.id = (uint8_t *) str;
  r.idSize = strlen((char *) r.id);
  r.timeStamp = 34567;

  uint8_t *retVal = pduCreator_pJoin(&r, &bufferSize);
  globalBuffer = retVal + 1;
  pduPJoin *ret = pduReader_pJoin(4);

  EXPECT_EQ(ret->opCode, PJOIN);
  EXPECT_EQ(r.idSize, ret->idSize);
  EXPECT_EQ(strcmp((char *) ret->id, (char *)r.id), 0);
  EXPECT_EQ(r.timeStamp, ret->timeStamp);

  free(ret->id);
  free(ret);
  free(retVal);
}

TEST_F(PduReaderTest, read_pLeavePdu){
  pduPLeave r;
  char str[] = "Micke";
  int bufferSize;
  r.opCode = PJOIN;
  r.id = (uint8_t *) str;
  r.idSize = strlen((char *) r.id);
  r.timeStamp = 34567;

  uint8_t *retVal = pduCreator_pLeave(&r, &bufferSize);
  globalBuffer = retVal + 1;
  pduPLeave *ret = pduReader_pLeave(4);

  EXPECT_EQ(ret->opCode, PJOIN);
  EXPECT_EQ(r.idSize, ret->idSize);
  EXPECT_EQ(strcmp((char *) ret->id, (char *)r.id), 0);
  EXPECT_EQ(r.timeStamp, ret->timeStamp);

  free(ret->id);
  free(ret);
  free(retVal);
}

TEST_F(PduReaderTest, read_participants){
  pduParticipants r;
  char *ids[2];

  ids[0] = (char *)"Micke";
  ids[1] = (char *)"Jonas";

  r.opCode = PARTICIPANTS;
  r.noOfIds = 2;
  r.ids = (uint8_t **)ids;

  uint8_t *retVal = pduCreator_participants(&r);
  globalBuffer = retVal + 1;
  pduParticipants *ret = pduReader_participants(4);

  EXPECT_EQ(ret->opCode, PARTICIPANTS);
  EXPECT_EQ(r.noOfIds, ret->noOfIds);
  for (int i = 0; i < ret->noOfIds; i++){
    EXPECT_EQ(strcmp((char *) ret->ids[i], (char *)r.ids[i]), 0);
    free(ret->ids[i]);
  }

  free(ret->ids);
  free(ret);
  free(retVal);
}

/*TEST_F(PduReaderTest, read_quit){
  uint8_t *retVal = pduCreator_quit();
  pduQuit *ret = pduReader_quit(retVal);
  EXPECT_EQ(ret->opCode, QUIT);
  free(ret);
  free(retVal);
}*/

TEST_F(PduReaderTest, read_mess){
  pduMess r;

  r.opCode = MESS;
  r.id = (uint8_t *) "Micke";
  r.message = (uint8_t *)"Snart är vi klara!!";
  r.timeStamp = 1337;
  r.messageSize = strlen((char *) r.message);
  r.idSize = strlen((char *) r.id);

  uint8_t checksum = calculateCheckSum((void*)&r.opCode, 1);
  checksum += calculateCheckSum((void*)&r.idSize, 1);
  checksum += calculateCheckSum((void*)&r.messageSize, 2);
  checksum += calculateCheckSum((void*)&r.timeStamp, 4);
  checksum += calculateCheckSum((void *)r.message, r.messageSize);
  checksum += calculateCheckSum((void *)r.id, r.idSize);
  checksum = ~checksum;

  uint8_t *retVal = pduCreator_mess(&r);
  globalBuffer = retVal + 1;
  pduMess *ret = pduReader_mess(4);

  uint8_t retChecksum = calculateCheckSum((void*) &ret->opCode, 1);
  retChecksum += calculateCheckSum((void*) &ret->idSize, 1);
  retChecksum += calculateCheckSum((void*) &ret->messageSize, 2);
  retChecksum += calculateCheckSum((void*) &ret->timeStamp, 4);
  retChecksum += calculateCheckSum((void *) ret->message,  ret->messageSize);
  retChecksum += calculateCheckSum((void *) ret->id, ret->idSize);

  EXPECT_EQ(ret->opCode, MESS);
  EXPECT_EQ(r.timeStamp, ret->timeStamp);
  EXPECT_EQ(r.idSize, ret->idSize);
  EXPECT_EQ(r.messageSize, ret->messageSize);
  EXPECT_EQ(strcmp((char *)r.message, (char *) ret->message), 0);
  EXPECT_EQ(strcmp((char *)r.id, (char *) ret->id), 0);

  uint8_t res = ~(retChecksum + ret->checkSum);
  EXPECT_FALSE(res);


  free(ret->message);
  free(ret->id);
  free(ret);
  free(retVal);
}



