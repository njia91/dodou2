//
// Created by njia on 2018-07-07.
//

#include "client.h"
#include "clientSession.h"
#include <stdio.h>
#include <unitypes.h>


void parseArgs(int argc, char **argv, inputArgs *args) {
  if (argc <= 4) {
    fprintf(stderr, "Too few or too many Arguments \n"
                    "[Username] [ns|cs] [remote IP Adress] [remote Port]\n");
    exit(EXIT_FAILURE);
  }
  args->username = argv[1];
  args->contactNS = strcmp("ns", argv[2]) ? false : true;
  args->ipAdress = (uint8_t *) argv[3];
  //memcpy(args->ipAdress, argv[3], sizeof(uint32_t));
  args->port = calloc(sizeof(uint8_t), PORT_LENGTH);
  memcpy(args->port, argv[4], PORT_LENGTH - 1);

}

int establishConnectionWithNs(inputArgs *cData){
  int nameserver_fd = 0;
  struct addrinfo* res=0;

  struct addrinfo hints;
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  hints.ai_flags=AI_ADDRCONFIG;

  int ret = facade_getAddrinfo((char *) cData->ipAdress, cData->port, &hints, &res);

  if (ret != 0) {
    fprintf(stderr, "%s \n",gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  nameserver_fd = facade_createSocket(&res);

  if (nameserver_fd == -1){
    fprintf(stderr, "Unable to setup socket to nameserver.\n");
    exit(EXIT_FAILURE);
  }

  ret = facade_connect(nameserver_fd, &res);
  if (ret == -1){
    fprintf(stderr, "Could not connect to NameServer %s \n", strerror(errno));
  }


  facade_freeaddrinfo(res);
  return nameserver_fd;

}

pduSList *getServerList(int nameServer_fd){
  size_t bufferSize;
  uint8_t *getList = pduCreator_getList(&bufferSize);

  ssize_t ret = facade_write(nameServer_fd, getList, bufferSize);

  free(getList);

  if (ret != WORD_SIZE){
    fprintf(stderr, "could not sent all data");
  }

  if (ret == -1 ){
    fprintf(stderr, "Unable to write data to socket.\n");
    return NULL;
  }

  genericPdu *pdu = getDataFromSocket(nameServer_fd);


  if (pdu->opCode != SLIST){
    fprintf(stderr, "Invalid packet received from Name Server.\n"
                    "Expected: %d\n"
                    "Actual: %d\n", SLIST, pdu->opCode);

    exit(EXIT_FAILURE);
  }
  return (pduSList *) pdu;
}

int getServerChoiceFromUser(pduSList *pSList, inputArgs *inArgs){
  int userInput = 0;

  // Frees old port string used to connect to NS
  free(inArgs->port);

  if (pSList->noOfServers == 0){
    fprintf(stderr, "No available or active chat servers on this name-server. Terminating program \n");
    return 0;
  }

  do{
    userInput = -1;
    fprintf(stdout, "-----------------------------------------------------------------\n");
    fprintf(stdout, "List of available servers: \n");
    for (int i = 0; i < pSList->noOfServers; i++){
      fprintf(stdout, "-----------------------------------------------------------------\n");
      fprintf(stdout, "Server ID \t: %d\n", i + 1);
      fprintf(stdout, "Server name \t: %s\n", pSList->sInfo[i].serverName);
      fprintf(stdout, "Number of active users \t: %d\n", pSList->sInfo[i].noOfClients);
      fprintf(stdout, "IP adress of server \t: ");
      for(int j = 0; j < 4; j++){
        fprintf(stdout, "%u. ", pSList->sInfo[i].ipAdress[j]);
      }
      fprintf(stdout, "\n");

    }
    // TODO CLear STDIN before input?
    fprintf(stdout, "-----------------------------------------------------------------\n");
    fprintf(stdout, "Select a server between 1 and %d. Zero to exit:  ", pSList->noOfServers);
    char *buffer = NULL;
    size_t size = 0;
    fflush(stdin);
    ssize_t ret  = getline(&buffer, &size, stdin);
    if (ret == -1){
      fprintf(stderr, "%s: %s \n",__func__,strerror(errno));
      return 0;
    }

    if (buffer[0] - '0' <= pSList->noOfServers && buffer[0] -'0' >= 0){
      userInput = buffer[0] - '0';
      if (userInput){
        inArgs->ipAdress = calloc(1, 16);
        // Convert DOT style IP address.
        sprintf((char *) inArgs->ipAdress, "%u.%u.%u.%u", pSList->sInfo[userInput - 1].ipAdress[0],
                                                         pSList->sInfo[userInput - 1].ipAdress[1],
                                                         pSList->sInfo[userInput - 1].ipAdress[2],
                                                         pSList->sInfo[userInput - 1].ipAdress[3]);
        inArgs->port = calloc(sizeof(uint8_t), PORT_LENGTH);
        sprintf(inArgs->port, "%d", pSList->sInfo[userInput - 1].port);
      }
    } else {
        fprintf(stdout, "Invalid choice. Please select a server in the list.\n");
    }
    free(buffer);
  }while(userInput == -1);
  fprintf(stdout, "\n-----------------------------------------------------------------\n\n");

  return userInput;
}

int client_main(int argc, char **argv){
  inputArgs cData;

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
    deletePdu((genericPdu *)pSList);

    if (ret == 0){
      exit(EXIT_SUCCESS);
    }
  }


  printf("Client name : %s, IP: %d.%d.%d.%d, Port %s \n\n", cData.username, cData.ipAdress[0],
         cData.ipAdress[1], cData.ipAdress[2], cData.ipAdress[3], cData.port);

  // Start Session
  startChatSession(&cData);
  free(cData.port);
  free(cData.ipAdress);
  return 0;

}
