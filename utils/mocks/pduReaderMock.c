//
// Created by njia on 2018-07-23.
// Mock object for pduReader.h
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "pduReader.h"

void *getDataFromSocket(int socket_fd) {
  return (void *)mock();
}

//Server-nameserver interaction
pduReq *pduReader_req(int socket_fd);

pduAlive *pduReader_alive(int socket_fd);

//Client-nameserver interaction
pduAck *pduReader_ack(int socket_fd);

pduSList *pduReader_SList(int socket_fd){
  return (pduSList *)mock();
}

//Client-server interaction
pduJoin *pduReader_join(int socket_fd);

pduPJoin *pduReader_pJoin(int socket_fd);

pduPLeave *pduReader_pleave(int socket_fd);

pduParticipants *pduReader_participants(int socket_fd);

pduQuit *pduReader_quit(int socket_fd);

pduMess *pduReader_mess(int socket_fd);