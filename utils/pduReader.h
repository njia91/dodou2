//
// Created by njia on 2018-05-20.
//

#ifndef DODOU2_PDUREADER_H
#define DODOU2_PDUREADER_H

#include "pduCommon.h"
#include "sysCall_facade.h"
genericPdu *getUdpDataFromSocket(int socket_fd);
genericPdu * getDataFromSocket(int socket_fd);

//Server-nameserver interaction
pduReg *pduReader_req(int socket_fd);

pduAlive *pduReader_alive(int socket_fd);

//Client-nameserver interaction
pduAck *pduReader_ack(uint8_t* buffer);

pduSList *pduReader_SList(int socket_fd);

//Client-server interaction
pduJoin *pduReader_join(int socket_fd);

pduPJoin *pduReader_pJoin(int socket_fd);

pduPLeave *pduReader_pLeave(int socket_fd);

pduParticipants *pduReader_participants(int socket_fd);

//pduQuit *pduReader_quit(uint8_t *buffer);

pduMess *pduReader_mess(int socket_fd);

void deletePdu(genericPdu *pdu);


#endif //DODOU2_PDUREADER_H
