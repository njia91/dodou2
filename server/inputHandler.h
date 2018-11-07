#ifndef DODOU2_INPUTHANDLER_H
#define DODOU2_INPUTHANDLER_H

#include <stdbool.h>
#include <pduCommon.h>

#include "clientConnection.h"
#include "server.h"
#include "helpers.h"

/**
 *
 * @param sData
 * @return
 */
bool readInputFromUser(serverData *sData);

#endif //DODOU2_INPUTHANDLER_H
