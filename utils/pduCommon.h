#ifndef __PDUCOMMON
#define __PDUCOMMON

#include <stdint.h>

static const uint8_t BYTE_SIZE     = 1;
static const uint8_t WORD_SIZE     = 4;     

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
  uint8_t   serverNameLen;
  uint16_t  tcpPort;
  uint8_t   *serverName;
} pduReq;

typedef struct {
  uint8_t   opCode;
  uint8_t   noOfClients;
  uint16_t  id;
} pduAlive;

#endif