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
uint8_t *pduCreator_join(pduJoin *join, int *bufferSize);

uint8_t *pduCreator_pJoin(pduPJoin *pjoin, int *bufferSize);

uint8_t *pduCreator_pLeave(pduPLeave *pLeave, int *bufferSize);

uint8_t *pduCreator_participants(pduParticipants *par);

uint8_t *pduCreator_quit();

uint8_t *pduCreator_mess(pduMess *mess);

#endif