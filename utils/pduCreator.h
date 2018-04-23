#ifndef __PDUCREATOR
#define __PDUCREATOR

#include <stdint.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <arpa/inet.h>

//#include <unicode/unistr.h>

#include "pduCommon.h"

//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req);


char *pduCreator_alive(uint8_t noOfClients, uint16_t idNo);

//Client-nameserver interaction
char *pduCreator_getList();


//Client-server interaction
char *pduCreator_join(uint8_t idLength, uint8_t *idStr);

char *pduCreator_pjoin(uint8_t idLength, uint32_t time, uint8_t* idStr);

char *pduCreator_pleave(uint8_t idLength, uint32_t time, uint8_t* idStr);

char *pduCreator_participands(uint8_t noOfIds, uint16_t length);

char *pduCreator_quit();

char *pduCreator_mess(uint8_t idLength, uint16_t messageLength, 
                      uint32_t time, uint8_t* message);

#endif