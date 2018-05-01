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
  r.serverNameSize = serLen;

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
  r.serverNameSize = strlen((char *)r.serverName);

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

TEST(PduCreatorTest, creatingParticipantsPdu){
  pduParticipants p;
  char *ids[3];

  ids[0] = (char *)"MICKE"; 
  ids[1]= (char *)"JONAS";
  ids[2] = (char *)"JOHAN";
  int dataSize = strlen(ids[0]) + strlen(ids[1]) + strlen(ids[2]) + 3;
  char allIds[dataSize];

  memset(allIds, 0, dataSize);
  memcpy(allIds, ids[0], strlen(ids[0]) + 1);
  memcpy(allIds + strlen(ids[0]) + 1, ids[1], strlen(ids[1]) + 1);
  memcpy(allIds + strlen(ids[0]) + strlen(ids[1]) + 2, ids[2], strlen(ids[2]) + 1);
  


  p.opCode = PARTICIPANTS;
  p.noOfIds = 3;
  p.dataSize = dataSize;
  p.ids = (uint8_t *)allIds;

  uint8_t *retVal = pduCreator_participants(&p);

  uint8_t retOpCode;
  uint8_t retNoOfIds;
  uint16_t retDataSize;
    

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retNoOfIds, retVal + BYTE_SIZE, sizeof(uint8_t));
  memcpy(&retDataSize , retVal + (2 * BYTE_SIZE), sizeof(uint16_t));
  retDataSize = ntohs(retDataSize);

  uint8_t retData[retDataSize];
  memcpy(&retData, retVal + WORD_SIZE, retDataSize);
  char *retIds[retNoOfIds];

  int i = 0;
  int idNo = 0;
  int pos = 0;
  while(idNo < retNoOfIds){
    do{
      i++;
    } while(retData[i] != '\0');
    retIds[idNo] = (char *)calloc(sizeof(uint8_t), i);
    memcpy(retIds[idNo], &retData[pos], i);
    pos = i + 1;
    idNo++;
  }

  EXPECT_EQ(retOpCode, p.opCode);
  EXPECT_EQ(retNoOfIds, p.noOfIds);
  EXPECT_EQ(retDataSize, p.dataSize);
  for(int i = 0; i < retNoOfIds; i++){
    EXPECT_EQ(strcmp(retIds[i], ids[i]), 0);
  }
  
  free(retVal);
}

TEST(PduCreatorTest, creatingPLeavePdu){
  pduPJoin p;
  char *id = (char *)"MICKEÅÄÖ";

  p.opCode = PLEAVE;
  p.idSize = strlen(id);
  p.timeStamp = (uint32_t)time(NULL);
  p.id = (uint8_t *)id;

  uint8_t *retVal = pduCreator_pleave(&p);

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

  EXPECT_EQ(retOpCode, p.opCode);
  EXPECT_EQ(retIdSize, p.idSize);
  EXPECT_EQ(strcmp(retId, (char *)p.id), 0);
  EXPECT_EQ(retTimeStamp, p.timeStamp); 

  free(retVal);
}

TEST(PduCreatorTest, creatingMessPduServerSide){
  pduMess p;
  char *id = (char *)"MICKE1233";
  char *mess = (char *)"Fösta meddelande...23";

  p.opCode = MESS;
  p.idSize = strlen(id);
  p.timeStamp = (uint32_t)time(NULL);
  p.id = (uint8_t *)id;
  p.messageSize = (uint8_t)strlen(mess);
  p.message = (uint8_t *)mess;

  uint8_t *retVal = pduCreator_mess(&p);

  uint8_t retOpCode;
  uint8_t retIdSize;
  uint8_t retCheckSum;
  uint16_t retMessSize;
  uint32_t retTimeStamp;

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retIdSize, retVal + (2 * BYTE_SIZE), sizeof(uint8_t));
  memcpy(&retCheckSum, retVal + (3 * BYTE_SIZE), sizeof(uint8_t));
  memcpy(&retMessSize, retVal + WORD_SIZE, sizeof(uint16_t));
  memcpy(&retTimeStamp , retVal + (2 * WORD_SIZE), sizeof(uint32_t));

  retTimeStamp = ntohl(retTimeStamp);
  retMessSize = ntohs(retMessSize);

  int noOfWords = 3; //Size of header
  noOfWords += calculateNoOfWords(retIdSize);
  noOfWords += calculateNoOfWords(retMessSize);
  uint8_t checkSum = calculateCheckSum(retVal, 3 * BYTE_SIZE);
  checkSum += calculateCheckSum(retVal + WORD_SIZE, WORD_SIZE * (noOfWords - 1));

  char retId[retIdSize + 1];
  char retMess[retMessSize + 1];
  int offset = calculateNoOfWords(retMessSize) * WORD_SIZE;
  memcpy(&retMess, retVal + (3 * WORD_SIZE), retMessSize);
  memcpy(&retId, retVal + (3 * WORD_SIZE) + offset, retIdSize);
  retId[retIdSize] = '\0';
  retMess[retMessSize] = '\0';
 
  EXPECT_EQ(retOpCode, p.opCode);
  EXPECT_EQ(retIdSize, p.idSize);
  EXPECT_EQ(retMessSize, p.messageSize);
  EXPECT_EQ(strcmp(retId, (char *)p.id), 0);
  EXPECT_EQ(strcmp(retMess, (char *)p.message), 0);
  EXPECT_EQ(retTimeStamp, p.timeStamp); 

  uint8_t res = ~(checkSum + retCheckSum);
  EXPECT_FALSE(res);

  free(retVal);

}

TEST(PduCreatorTest, creatingMessPduClientSide){
  pduMess p;
  char *mess = (char *)"Fösta meddelande...23";

  p.opCode = MESS;
  p.idSize = 0;
  p.timeStamp = 0;
  p.id = NULL;
  p.messageSize = (uint8_t)strlen(mess);
  p.message = (uint8_t *)mess;

  uint8_t *retVal = pduCreator_mess(&p);

  uint8_t retOpCode;
  uint8_t retIdSize;
  uint8_t retCheckSum;
  uint16_t retMessSize;
  uint32_t retTimeStamp;

  memcpy(&retOpCode, retVal, sizeof(uint8_t));
  memcpy(&retIdSize, retVal + (2 * BYTE_SIZE), sizeof(uint8_t));
  memcpy(&retCheckSum, retVal + (3 * BYTE_SIZE), sizeof(uint8_t));
  memcpy(&retMessSize, retVal + WORD_SIZE, sizeof(uint16_t));
  memcpy(&retTimeStamp , retVal + (2 * WORD_SIZE), sizeof(uint32_t));

  retTimeStamp = ntohl(retTimeStamp);
  retMessSize = ntohs(retMessSize);

  int noOfWords = 3; //Size of header
  noOfWords += calculateNoOfWords(retIdSize);
  noOfWords += calculateNoOfWords(retMessSize);
  uint8_t checkSum = calculateCheckSum(retVal, 3 * BYTE_SIZE);
  checkSum += calculateCheckSum(retVal + WORD_SIZE, WORD_SIZE * (noOfWords - 1));

  char retId[retIdSize + 1];
  char retMess[retMessSize + 1];
  int offset = calculateNoOfWords(retMessSize) * WORD_SIZE;
  memcpy(&retMess, retVal + (3 * WORD_SIZE), retMessSize);
  memcpy(&retId, retVal + (3 * WORD_SIZE) + offset, retIdSize);
  retId[retIdSize] = '\0';
  retMess[retMessSize] = '\0';
 
  EXPECT_EQ(retOpCode, p.opCode);
  EXPECT_EQ(retIdSize, p.idSize);
  EXPECT_EQ(retMessSize, p.messageSize);
  EXPECT_EQ(strcmp(retId, (char *)p.id), 0);
  EXPECT_EQ(strcmp(retMess, (char *)p.message), 0);
  EXPECT_EQ(retTimeStamp, p.timeStamp); 

  uint8_t res = ~(checkSum + retCheckSum);
  EXPECT_FALSE(res);

  free(retVal);

}
