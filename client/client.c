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
  memcpy(args->ipAdress, argv[3], sizeof(uint32_t));
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

  int ret = getAddrInformation((char *)cData->ipAdress, cData->port , &hints, &res);

  if (ret != 0) {
    fprintf(stderr, gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  nameserver_fd = createSocket(&res);

  if (nameserver_fd == -1){
    fprintf(stderr, "Unable to setup socket to nameserver.\n");
    exit(EXIT_FAILURE);
  }

  ret = connectToServer(nameserver_fd, &res);

  return nameserver_fd;

}

pduSList *getServerList(int nameServer_fd){
  pduGetList getList;
  getList.opCode = GETLIST;

  int ret = writeToSocket(nameServer_fd, &getList.opCode, 1);

  if (ret == -1 ){
    fprintf(stderr, "Unable to write data to socket.\n");
    return NULL;
  }
  uint8_t opCode = 0;
  ret = readFromSocket(nameServer_fd, &opCode, 1);

  if (opCode != SLIST){
    fprintf(stderr, "Invalid packet received from Name Server.\n"
                    "Expected: %d\n"
                    "Actual: %d\n", SLIST, opCode);

    exit(EXIT_FAILURE);
  }
  return (pduSList *)getDataFromSocket(nameServer_fd, opCode);
}

void getServerChoiceFromUser(pduSList *pSList, clientData *cData){
  int UserInput = 0;
  do{
    fprintf(stdout, "Based on server id, select a chat server you wish to connect to: \n");
    for (int i = 0; i < pSList->noOfServers; i++){
      fprintf(stdout, "-----------------------------------------------------------------\n");
      fprintf(stdout, "Server ID \t: %d\n", i + 1);
      fprintf(stdout, "Server name \t: %s\n", pSList->sInfo[i].serverName);
      fprintf(stdout, "Number of active users \t: %d\n", pSList->sInfo[i].noOfClients);
      fprintf(stdout, "IP adress of server \t: ");
      for(int j = 0; j < 4; j++){
        fprintf(stdout, "%d ", pSList->sInfo[i].ipAdress[j]);

      }
      fprintf(stdout, "\n");

    }
    UserInput = getchar();
    if(UserInput > pSList->noOfServers){
      fprintf(stdout, "Invalid choice. Please select a server in the list.\n");
      UserInput = -1;
    }
  }while(UserInput != -1);
  fprintf(stdout, "-----------------------------------------------------------------\n");

  memcpy(cData->ipAdress, pSList->sInfo[UserInput].ipAdress, sizeof(uint32_t));
  sprintf(cData->port, "%d", pSList->sInfo[UserInput].port);
}

int client_main(int argc, char **argv){
  clientData cData;
  parseArgs(argc, argv, &cData);

  if (cData.contactNS){
    int nameServer_fd = establishConnectionWithNs(&cData);
    pduSList *pSList = getServerList(nameServer_fd);
    if (pSList == NULL){
      fprintf(stderr, "Something went wrong when reading data from Socket. "
                      "Terminating Program\n");
      exit(EXIT_FAILURE);
    }
    getServerChoiceFromUser(pSList, &cData);





  }

  return 0;

}
