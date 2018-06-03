#ifndef __PDUCOMMON
#define __PDUCOMMON

#include <stdint.h>

static const uint8_t BYTE_SIZE     = 1;
static const uint8_t WORD_SIZE     = 4;     

//Operational Codes 
static const uint8_t REQ           = 0;
static const uint8_t ACK           = 1;
static const uint8_t ALIVE         = 2;
static const uint8_t GETLIST       = 3;
static const uint8_t SLIST         = 4;
static const uint8_t MESS          = 10;
static const uint8_t QUIT          = 11; 
static const uint8_t JOIN          = 12;
static const uint8_t PJOIN         = 16;
static const uint8_t PLEAVE        = 19;
static const uint8_t PARTICIPANTS  = 19;
static const uint8_t NOTREQ        = 100;

typedef struct {
  uint8_t   opCode;
  uint8_t   serverNameSize;
  uint8_t   *serverName;
  uint16_t  tcpPort;
} pduReq;

typedef struct {
  uint8_t   opCode;
  uint8_t   noOfClients;
  uint16_t  id;
} pduAlive;

typedef struct {
  uint8_t   opCode;
  uint16_t  id;
} pduAck;

typedef struct {
  uint8_t   opCode;
  uint16_t  id;
} pduNotReq;

typedef struct {
  uint8_t   *ipAdress;
  uint8_t   noOfClients;
  uint8_t   serverNameLen;
  uint8_t   *serverName;
  uint16_t  port;
} serverInfo;

typedef struct {
  uint8_t   opCode;
  uint16_t  noOfServers;
  serverInfo *sInfo;
} pduSList;

typedef struct {
  uint8_t   opCode;
  uint8_t   idSize;
  uint8_t   *id;
} pduJoin;

typedef struct {
  uint8_t   opCode;
  uint8_t   noOfIds;
  uint8_t   dataSize;
  uint8_t   *ids;
} pduParticipants;

typedef struct {
  uint8_t   opCode;
  uint8_t   idSize;
  uint8_t   checkSum;
  uint8_t   messageSize;
  uint8_t   *message;
  uint8_t   *id;
  uint32_t  timeStamp;
} pduMess;

typedef struct {
  uint8_t   opCode;
  uint8_t   idSize;
  uint8_t   *id;
  uint32_t  timeStamp;
} pduPJoin;

typedef struct {
  uint8_t   opCode;
} pduQuit;


typedef pduPJoin pduPLeave;

uint8_t calculateCheckSum(void *p, int size);

#endif