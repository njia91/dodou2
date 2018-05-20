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

  int noOfServers = 2;
  uint16_t u16;

  serverInfo *s = (serverInfo*)calloc(sizeof(serverInfo), noOfServers);

  uint8_t ip1[4] = {123, 0, 23, 1};
  uint8_t ip1[4] = {312, 0, 32, 1};

  char *serName1 = (char *)"MickeServer";
  char *serName2 = (char *)"Andersson";
  uint8_t serLen1 = strlen(serName1);
  uint8_t serLen2 = strlen(serName2);

  s[1].serverName = serName1;
  s[1].

  uint16_t u16 = htons(r.id);

  uint8_t *retVal = (uint8_t *)calloc(WORD_SIZE, 1);

  memcpy(retVal, &(r.opCode), sizeof(uint8_t));
  memcpy(retVal + (2 * BYTE_SIZE), &u16, sizeof(uint16_t));

  pduAck *ret = pduReader_ack(retVal);


  EXPECT_EQ(ret->opCode, ACK);
  EXPECT_EQ(ret->id, r.id);

  free(ret);
}