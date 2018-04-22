#include "pduCreator.h"

//Server-nameserver interaction
char *pduCreator_req(uint8_t *serverName, uint16_t port){
  pduReq *req = (pduReq*) calloc( 1, sizeof(pduReq));

  req->opCode = REQ;
  req->serverLength = strlen((char*) serverName);
  req->tcpPort = htons(port);
  //req->serverName = calloc(serverName;

  return (char*) req;
}

char *pduCreator_alive(uint8_t noOfClients, uint16_t idNo);

//Client-nameserver interaction
char *pduCreator_getList();


//Client-server interaction
char *pduCreator_join(uint8_t idLength, uint8_t *idStr);

char *pduCreator_pjoin(uint8_t idLength, uint32_t time, uint8_t *idStr);

char *pduCreator_pleave(uint8_t idLength, uint32_t time, uint8_t *idStr);

char *pduCreator_participands(uint8_t noOfIds, uint16_t length);

char *pduCreator_quit();

char *pduCreator_mess(uint8_t idLength, uint16_t messageLength,
                      uint32_t time, uint8_t *message);