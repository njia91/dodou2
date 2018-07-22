//
// Created by njia on 2018-07-07.
//




#include "client.h"
#include <stdio.h>

/*
int main(int argc, char **argv){
  printf("Tjoo heeej! \n");

  return 0;
}*/

void parseArgs(int argc, char **argv, clientData *args) {
  if (argc < 4) {
    fprintf(stderr, "Too few or too many Arguments \n"
                    "<ProgramName> [localPort] [remote IP Adress] [remote Port]\n");
    exit(EXIT_FAILURE);
  }
  args->clientName = argv[1];
  args->contactNS = strcmp("ns", argv[2]) ? true : false;
  args->ipAdress = argv[3];
  args->port = argv[4];


}

int establishConnectionWithNs(clientData *cData){
  int nameserver_fd = 0;
  struct addrinfo* res=0;

  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  hints.ai_flags=AI_ADDRCONFIG;

  int ret = getaddrinfo(cData->ipAdress, cData->port , &hints, &res);

  if (ret != 0) {
    fprintf(stderr, gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  nameserver_fd = createSocket(&res);

  if (nameserver_fd == -1){
    fprintf(stderr, "Unable to setup socket to nameserver.\n");
    exit(EXIT_FAILURE);
  }

  printf("Value from createSocket %d \n", nameserver_fd);

  ret = connectToServer(nameserver_fd, &res);

  printf("Value from connectTOServer %d \n", ret);

  return nameserver_fd;

}
//pduGetList getList;
//getList.opCode = GETLIST;

//   ret = writeToSocket(nameserver_fd, (char *) &getList.opCode);

int getServerList(int nameserver_fd){


}

int client_main(int argc, char **argv){
  clientData cData;

  parseArgs(argc, argv, &cData);

  if (cData.contactNS){
    int nameServer_fd = establishConnectionWithNs(&cData);
    getServerList(nameServer_fd);
  }

  return 0;

}
