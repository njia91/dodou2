#include <gtest/gtest.h>

#include <string>
#include <iostream>

#ifdef __cplusplus
extern "C"{
#endif
  #include "pduCommon.h"
  #include "pduCreator.h"
  #include <stdlib.h>
#ifdef __cplusplus
}
#endif
class PduCreatorTest : public testing::Test
{
  void SetUp(){
    std::cout << " I SETUP !!!" << std::endl;
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


