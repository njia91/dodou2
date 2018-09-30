#ifndef __PDUCREATOR
#define __PDUCREATOR

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "pduCommon.h"

//Server-nameserver interaction
uint8_t *pduCreator_req(pduReq *req, size_t *nByte);

uint8_t *pduCreator_alive(pduAlive *alive, size_t *nByte);

//Client-nameserver interaction
uint8_t *pduCreator_getList(size_t *nByte);

//Client-server interaction
uint8_t *pduCreator_join(pduJoin *join, size_t *nByte);

uint8_t *pduCreator_pJoin(pduPJoin *pjoin, size_t *nByte);

uint8_t *pduCreator_pLeave(pduPLeave *pLeave, size_t *nByte);

uint8_t *pduCreator_participants(pduParticipants *par, size_t *nByte);

uint8_t *pduCreator_quit(size_t *nByte);

uint8_t *pduCreator_mess(pduMess *mess, size_t *nByte);

#endif