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
  args->contactNS = strcmp("ns", argv[2]) ? false : true;
  memcpy(args->ipAdress, argv[3], sizeof(uint32_t));
  args->port =  atoi(argv[4]);


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

  char port[5];
  sprintf(port, "%d", cData->port );
  int ret = getAddrInformation((char *)cData->ipAdress, port , &hints, &res);

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

int getServerChoiceFromUser(pduSList *pSList, clientData *cData){
  int userInput = 0;
  const int bufferSize = 24;
  char buffer[bufferSize];
  memset(buffer, 0, bufferSize);
  do{
    userInput = -1;
    fprintf(stdout, "-----------------------------------------------------------------\n");
    fprintf(stdout, "List of available servers.: \n");
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
    // TODO CLear STDIN before input?
    fprintf(stdout, "-----------------------------------------------------------------\n");
    fprintf(stdout, "Select a server between 1 and %d. Zero to exit:  ", pSList->noOfServers);
    fgets(buffer, bufferSize, stdin);
    if (buffer[0] - '0' <= pSList->noOfServers + 1 && buffer[0] -'0' >= 0){
      userInput = buffer[0] - '0';
      if (userInput){
        memcpy(cData->ipAdress, pSList->sInfo[userInput - 1].ipAdress, sizeof(uint32_t));
        cData->port = pSList->sInfo[userInput - 1].port;
      }
    } else {
        fprintf(stdout, "Invalid choice. Please select a server in the list.\n");
    }
  }while(userInput == -1);
  fprintf(stdout, "-----------------------------------------------------------------\n\n");


  return userInput;
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
    int ret = getServerChoiceFromUser(pSList, &cData);

    if (ret == 0){
      fprintf(stdout, "User chose to terminate the session \n");
      exit(EXIT_SUCCESS);
    }
  }


  return 0;

}
