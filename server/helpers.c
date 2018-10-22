#include "helpers.h"
#include "server.h"

uint32_t getCurrentTime() {
  time_t rawTime;
  time(&rawTime);
  return (uint32_t) rawTime;
}

bool startsWith(const char *pre, const char *str) {
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void addToParticipantsList(int socket_fd, char *clientID) {
  participant par;
  par.clientID = calloc(strlen(clientID) + 1, sizeof(char));
  memcpy(par.clientID, clientID, strlen(clientID));
  par.socket_fd = socket_fd;

  fprintf(stdout, "%s added to participants\n", par.clientID);
  participantList[currentFreeParticipantSpot] = par;
  currentFreeParticipantSpot++;
}

void removeFromParticipantsList(int socket_fd) {
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      // Remove the client from the participants list
      free(participantList[i].clientID);
      participantList[i] = participantList[--currentFreeParticipantSpot];
      break;
    }
  }
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