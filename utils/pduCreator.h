#ifndef __PDUCREATOR
#define __PDUCREATOR

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <arpa/inet.h>


#include "pduCommon.h"

//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req);

uint8_t *pduCreator_alive(pduAlive *alive);

//Client-nameserver interaction
uint8_t *pduCreator_getList();


//Client-server interaction
uint8_t *pduCreator_join(uint8_t idLength, uint8_t *idStr);

uint8_t *pduCreator_pjoin(uint8_t idLength, uint32_t time, uint8_t* idStr);

uint8_t *pduCreator_pleave(uint8_t idLength, uint32_t time, uint8_t* idStr);

uint8_t *pduCreator_participands(uint8_t noOfIds, uint16_t length);

uint8_t *pduCreator_quit();

uint8_t *pduCreator_mess(uint8_t idLength, uint16_t messageLength, 
                      uint32_t time, uint8_t* message);

#endif