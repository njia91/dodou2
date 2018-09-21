//
// Created by njia on 2018-07-07.
//

#ifndef DODOU2_CLIENT_H
#define DODOU2_CLIENT_H

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sysCall_facade.h>

#include <netdb.h>
#include "pduCommon.h"
#include "pduReader.h"

#include "pduCreator.h"

static const size_t MAX_SIZE = 1024;


typedef struct {
  char *username;
  bool contactNS;
  uint8_t *ipAdress;
  char *port;
} clientData;

void parseArgs(int argc, char **argv, clientData *args);

int establishConnectionWithNs(clientData *cData);

pduSList *getServerList(int nameServer_fd);

int getServerChoiceFromUser(pduSList *pSList, clientData *cData);

int client_main(int argc, char **argv);

#endif //DODOU2_CLIENT_H
