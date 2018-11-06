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

uint8_t getCurrentFreeParticipantSpot() {
  uint8_t retVal;
  sem_wait(&helperMutex);
  retVal = currentFreeParticipantSpot;
  sem_post(&helperMutex);
  return retVal;
}

void setCurrentFreeParticipantSpot(uint8_t newVal) {
  sem_wait(&helperMutex);
  currentFreeParticipantSpot = newVal;
  sem_post(&helperMutex);
}

int stringToInt(char *string) {
  char *end;
  for (long i = strtol(string, &end, 10); string != end; i = strtol(string, &end, 10)) {
    string = end;
    if (errno != ERANGE) {
      return (int) i; // Found integer
    }
    errno = 0; // Keep searching string
  }
  fprintf(stderr, "Could not find integer\n");
  return -1;
}

void addToParticipantsList(int socket_fd, char *clientID) {
  sem_wait(&helperMutex);
  participant par;
  par.clientID = calloc(strlen(clientID) + 1, sizeof(char));
  memcpy(par.clientID, clientID, strlen(clientID));
  par.socket_fd = socket_fd;

  //fprintf(stdout, "%s added to participants\n", par.clientID);
  participantList[currentFreeParticipantSpot] = par;
  currentFreeParticipantSpot++;
  sem_post(&helperMutex);
}

void removeFromParticipantsList(int socket_fd) {
  sem_post(&helperMutex);
  for (int i = 0; i < currentFreeParticipantSpot; i++) {
    if (socket_fd == participantList[i].socket_fd) {
      // Remove the client from the participants list
      free(participantList[i].clientID);
      participantList[i] = participantList[--currentFreeParticipantSpot];
      break;
    }
  }
  sem_wait(&helperMutex);
}

void fillInAddrInfo(struct addrinfo **addrInfo, const int port, const uint8_t *IPAddress, int socketType, int flags) {
  char portId[15];
  sprintf(portId, "%d", port);
  struct addrinfo info;
  memset(&info, 0, sizeof(info));
  info.ai_family = AF_INET;
  info.ai_socktype = socketType;
  info.ai_protocol = 0;
  info.ai_flags = flags;

  if (getaddrinfo((char *)IPAddress, portId, &info, addrInfo) != 0) {
    fprintf(stderr, "Failed to get address info\n");
    exit(EXIT_FAILURE);
  }
}