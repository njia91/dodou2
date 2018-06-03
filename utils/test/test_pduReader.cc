//
// Created by njia on 2018-05-20.
//

#include <gtest/gtest.h>

#include <string>
#include <iostream>
#include <pduCommon.h>

#ifdef __cplusplus
extern "C"{
#endif
#include "pduCommon.h"
#include "pduCreator.h"
#include "pduReader.h"
#include <stdlib.h>
#include <time.h>
#ifdef __cplusplus
}
#endif
class PduReaderTest : public testing::Test
{
  void SetUp(){
    setlocale(LC_CTYPE, "");
  }

  void TearDown(){}
};


TEST(PduReaderTest, readReqPacket){
  pduReq r;
  char *serName = (char *)"testHost";
  uint8_t serLen = strlen(serName);

  r.opCode = REQ;
  r.tcpPort = 454;
  r.serverName = (uint8_t *)serName;
  r.serverNameSize = serLen;

  uint8_t *retVal = pduCreator_req(&r);

  pduReq *ret = pduReader_req(retVal);

  EXPECT_EQ(ret->opCode, REQ);
  EXPECT_EQ(ret->serverNameSize, serLen);
  EXPECT_EQ(ret->tcpPort, r.tcpPort);
  EXPECT_EQ(strcmp((char *)ret->serverName, (char *)r.serverName), 0);

  free(ret);
}

TEST(PduReaderTest, readAckPacket){
  pduAck r;
  char *serName = (char *)"testHost";
  uint8_t serLen = strlen(serName);

  r.opCode = ACK;
  r.id = 454;

  uint16_t u16 = htons(r.id);

  uint8_t *retVal = (uint8_t *)calloc(WORD_SIZE, 1);

  memcpy(retVal, &(r.opCode), sizeof(uint8_t));
  memcpy(retVal + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));

  pduAck *ret = pduReader_ack(retVal);


  EXPECT_EQ(ret->opCode, ACK);
  EXPECT_EQ(ret->id, r.id);

  free(ret);
}

TEST(PduReaderTest, readSListPacket){
  pduSList r;

  int noOfServers = 2;
  uint16_t u16;

  uint8_t noClients1 = 4;
  uint8_t noClients2 = 2;

  uint16_t port1 = 1234;
  uint16_t port2 = 4321;

  serverInfo *s = (serverInfo*)calloc(sizeof(serverInfo), noOfServers);

  uint8_t ip1[4] = {123, 0, 23, 1};
  uint8_t ip2[4] = {312, 0, 32, 1};

  char *serName1 = (char *)"MickeServer";
  char *serName2 = (char *)"Andersson";
  uint8_t serLen1 = strlen(serName1);
  uint8_t serLen2 = strlen(serName2);

  s[0].serverName = serName1;
  s[0].noOfClients = noClients1;
  s[0].serverNameLen = serLen1;
  s[0].ipAdress = ip1;

  s[1].serverName = serName2;
  s[1].noOfClients = noClients2;
  s[1].serverNameLen = serLen2;
  s[1].ipAdress = ip2;

  r.opCode = SLIST;
  r.noOfServers = noOfServers;
  r.sInfo = s;

  int bufferSize = 2 + (4 * 2) +serLen1 + serLen2;
  int offset = 0;
  uint8_t *buffer = (uint8_t *)calloc(sizeof(uint8_t), bufferSize);

  memcpy(buffer, &(r.opCode), sizeof(uint8_t));
  memcpy(buffer + (2 * BYTE_SIZE), &(r.noOfServers), sizeof(uint16_t));
  offset = WORD_SIZE;
  //For first server
  memcpy(buffer + WORD_SIZE, &(r.sInfo[0].ipAdress), sizeof(uint32_t));
  offset += WORD_SIZE;
  u16 = htons(r.sInfo[0].port);
  memcpy((buffer + offset, &(u16), sizeof(uint16_t));
  offset += sizeof(uint16_t);
  memcpy(buffer + offset, &(r.sInfo[0].noOfClients), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, &(r.sInfo[0].serverNameLen), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, &(r.sInfo[0].serverName), r.sInfo[0].serverNameLen);
  offset += r.sInfo[1].serverNameLen;
  //For second server
  memcpy(buffer + WORD_SIZE, &(r.sInfo[1].ipAdress), sizeof(uint32_t));
  offset += WORD_SIZE;
  u16 = htons(r.sInfo[1].port);
  memcpy((buffer + offset, &(u16), sizeof(uint16_t));
  offset += sizeof(uint16_t);
  memcpy(buffer + offset, &(r.sInfo[1].noOfClients), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, &(r.sInfo[1].serverNameLen), sizeof(uint8_t));
  offset += sizeof(uint8_t);
  memcpy(buffer + offset, &(r.sInfo[1].serverName), r.sInfo[1].serverNameLen);
  offset += r.sInfo[1].serverNameLen;

  pduSList *ret = pduReader_SList(buffer);


  EXPECT_EQ(ret->opCode, SLIST);
  EXPECT_EQ(ret->noOfServers, r.noOfServers);

  EXPECT_EQ(ret->sInfo[0].noOfClients, s[0].noOfClients);
  EXPECT_EQ(ret->sInfo[0].serverNameLen, s[0].serverNameLen);
  EXPECT_EQ(ret->sInfo[0].port, s[0].port);
  EXPECT_EQ(strcmp((char*)ret->sInfo[0].serverName, (char*)s[0].serverName), 0);
  EXPECT_EQ(strcmp((char*)ret->sInfo[0].ipAdress, (char*)s[0].ipAdress), 0);

  EXPECT_EQ(ret->sInfo[1].noOfClients, s[1].noOfClients);
  EXPECT_EQ(ret->sInfo[1].serverNameLen, s[1].serverNameLen);
  EXPECT_EQ(ret->sInfo[1].port, s[0].port);
  EXPECT_EQ(strcmp((char*)ret->sInfo[1].serverName, (char*)s[1].serverName), 0);
  EXPECT_EQ(strcmp((char*)ret->sInfo[1].ipAdress, (char*)s[1].ipAdress), 0);

  free(ret);
  free(buffer);

}