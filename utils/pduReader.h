//
// Created by njia on 2018-05-20.
//

#ifndef DODOU2_PDUREADER_H
#define DODOU2_PDUREADER_H

#include "pduCommon.h"
#include "dod_socket.h"

void *getDataFromSocket(int socket_fd, uint8_t opCode);

//Server-nameserver interaction
pduReq *pduReader_req(uint8_t *buffer);

pduAlive *pduReader_alive(uint8_t *buffer);

//Client-nameserver interaction
pduAck *pduReader_ack(uint8_t *buffer);

pduSList *pduReader_SList(int socket_fd);

//Client-server interaction
pduJoin *pduReader_join(uint8_t *buffer);

pduPJoin *pduReader_pJoin(uint8_t *buffer);

pduPLeave *pduReader_pleave(uint8_t *buffer);

pduParticipants *pduReader_participants(uint8_t *buffer);

pduQuit *pduReader_quit(uint8_t *buffer);

pduMess *pduReader_mess(uint8_t *buffer);

void deletePdu(void *pdu);


#endif //DODOU2_PDUREADER_H
