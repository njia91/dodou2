#include <gtest/gtest.h>

#include <string>
#include <iostream>

#ifdef __cplusplus
extern "C"{
#endif
  #include "pduCommon.h"
  #include "pduCreator.h"
  #include <stdlib.h>
  #include <time.h>
#ifdef __cplusplus
}
#endif
class PduCreatorTest : public testing::Test
{
  void SetUp(){
    std::cout << " I SETUP !!!" << std::endl;
    setlocale(LC_CTYPE, "");
  }

  void TearDown(){}
};


TEST(PduCreatorTest, creatingReqPacket){
  pduReq r;
  char *serName = (char *)"testHost";
  uint8_t serLen = strlen(serName);

  r.opCode = REQ;
  r.tcpPort = 454;
  r.serverName = (uint8_t *)serName;
  r.serverNameLen = serLen;

  uint8_t *retVal = pduCreator_req(&r);

  uint8_t retOpCode;
  uint16_t retTcpPort;
  uint8_t retSerNameLen;

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retSerNameLen, retVal + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&retTcpPort, retVal + (2 * BYTE_SIZE ), sizeof(uint16_t));
  char retSerName[retSerNameLen + 1];
  memcpy(retSerName, retVal + WORD_SIZE, retSerNameLen);
  retSerName[retSerNameLen] = '\0';
  retTcpPort = ntohs(retTcpPort);

  EXPECT_EQ(retOpCode, REQ);
  EXPECT_EQ(retSerNameLen, serLen);
  EXPECT_EQ(retTcpPort, r.tcpPort);
  EXPECT_EQ(strcmp(retSerName, (char *)r.serverName), 0);

  free(retVal);
}

TEST(PduCreatorTest, creatingReqPacketWithInvalidOpCode){
  pduReq r;

  r.opCode = ACK;
  r.tcpPort = 454;
  r.serverName = (uint8_t *)"testHost";
  r.serverNameLen = strlen((char *)r.serverName);

  uint8_t *retVal = pduCreator_req(&r);

  EXPECT_TRUE(retVal == NULL);
}

TEST(PduCreatorTest, creatingAlivePdu){
  pduAlive a;

  a.opCode = ALIVE;
  a.noOfClients = 5;
  a.id = 23;

  uint8_t *retVal = pduCreator_alive(&a);

  uint8_t retOpCode;
  uint8_t retNoOfClients;
  uint16_t retId;

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retNoOfClients  , retVal + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&retId  , retVal + (2 *BYTE_SIZE), sizeof(uint16_t));
  retId = ntohs(retId);

  EXPECT_EQ(retOpCode, a.opCode);
  EXPECT_EQ(retNoOfClients, a.noOfClients);
  EXPECT_EQ(retId, a.id);

  free(retVal);
}

TEST(PduCreatorTest, creatingJoinPdu){
  pduJoin j;
  char *id = (char *)"MICKEÅÄÖ";

  j.opCode = JOIN;
  j.idSize = strlen(id);
  j.id = (uint8_t *)id;

  uint8_t *retVal = pduCreator_join(&j);

  uint8_t retOpCode;
  uint8_t retIdSize;

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retIdSize  , retVal + BYTE_SIZE, sizeof(uint8_t));
  uint8_t retId[retIdSize + 1];
  memcpy(&retId  , retVal + WORD_SIZE, retIdSize);
  retId[retIdSize] = '\0';

  EXPECT_EQ(retOpCode, j.opCode);
  EXPECT_EQ(retIdSize, j.idSize);
  EXPECT_EQ(strcmp((char *)retId, (char *)j.id), 0);

  free(retVal);
}

TEST(PduCreatorTest, creatingPJoinPdu){
  pduPJoin pj;
  char *id = (char *)"MICKEÅÄÖ";

  pj.opCode = PJOIN;
  pj.idSize = strlen(id);
  pj.timeStamp = (uint32_t)time(NULL);
  pj.id = (uint8_t *)id;

  uint8_t *retVal = pduCreator_pJoin(&pj);

  uint8_t retOpCode;
  uint8_t retIdSize;
  uint32_t retTimeStamp;
  

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retIdSize, retVal + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&retTimeStamp , retVal + WORD_SIZE, sizeof(uint32_t));
  char retId[retIdSize + 1];
  memcpy(&retId, retVal + (2 *WORD_SIZE), retIdSize);
  retId[retIdSize] = '\0';
  retTimeStamp = ntohl(retTimeStamp);

  EXPECT_EQ(retOpCode, pj.opCode);
  EXPECT_EQ(retIdSize, pj.idSize);
  EXPECT_EQ(strcmp(retId, (char *)pj.id), 0);
  EXPECT_EQ(retTimeStamp, pj.timeStamp); 

  free(retVal);
}
