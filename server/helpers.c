#include "helpers.h"
#include "server.h"

uint32_t getCurrentTime() {
  time_t rawtime;
  time(&rawtime);
  return (uint32_t) rawtime;
}


/**
 * Add a client to the participants list
 * @param socket_fd The clients socket
 * @param clientID The clients ID
 */
void addToParticipantsList(int socket_fd, char *clientID) {
  participant par;
  par.clientID = calloc(strlen(clientID) + 1, sizeof(char));
  memcpy(par.clientID, clientID, strlen(clientID));
  par.socket_fd = socket_fd;

  fprintf(stdout, "%s added to participants\n", par.clientID);
  participantList[currentFreeParticipantSpot] = par;
  currentFreeParticipantSpot++;
}

void fillInAddrInfo(struct addrinfo **addrInfo, const int port, const char *IPAddress, int socketType, int flags) {
  char portId[15];
  sprintf(portId, "%d", port);
  struct addrinfo info;
  memset(&info, 0, sizeof(info));
  info.ai_family = AF_UNSPEC;
  info.ai_socktype = socketType;
  info.ai_protocol = 0;
  info.ai_flags = flags;

  if (getaddrinfo(IPAddress, portId, &info, addrInfo) != 0) {
    fprintf(stderr, "Failed to get address info\n");
    exit(EXIT_FAILURE);
  }
}