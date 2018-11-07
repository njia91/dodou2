#ifndef DODOU2_NAMESERVERCONNECTION_H
#define DODOU2_NAMESERVERCONNECTION_H

#include <pduCommon.h>
#include <pduCreator.h>
#include <sysCall_facade.h>
#include <pduReader.h>

#include "helpers.h"

int establishConnectionWithNs(serverInputArgs args);

void registerToNameServer(int sock, serverInputArgs args);
bool gotACKResponse(int nameServerSocket);

#endif //DODOU2_NAMESERVERCONNECTION_H
