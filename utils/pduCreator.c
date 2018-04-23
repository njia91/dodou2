#include "pduCreator.h"

int main(int argc, char **argv){
  printf("Hello...\n");
  return 1;
}

//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req){
  int reqLen =  WORD_SIZE + req->serverNameLen;
  int noOfWords;

  if (reqLen % WORD_SIZE == 0){
    noOfWords = (reqLen / 4);
  } else {
    noOfWords = (reqLen / 4) + 1;
  }

  uint8_t *pduBuffer = calloc(sizeof(uint8_t), noOfWords * WORD_SIZE);

  //req->serverName = calloc(serverName;

  return pduBuffer;

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