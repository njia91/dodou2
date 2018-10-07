#include <pthread.h>
#include <socketReaderAPI.h>
#include "server.h"
#include "clientConnection.h"

void parseServerArgs(int argc, char **argv, serverInputArgs *args) {
  if (argc <= 4) {
    fprintf(stderr, "Too few or too many Arguments \n"
                    "[Port] [Server name] [Nameserver IP Adress] [Nameserver Port]\n");
    exit(EXIT_FAILURE);
  }
  args->serverPort = argv[1];
  args->serverName = calloc(strlen(argv[2]), sizeof(char));
  memcpy(args->serverName, argv[2], strlen(argv[2]));

  args->nameServerIP = calloc(strlen(argv[3]) + 1, sizeof(uint8_t));
  strcpy((char *) args->nameServerIP, argv[3]);
  args->nameServerPort = calloc(PORT_LENGTH, sizeof(uint8_t));
  memcpy(args->nameServerPort, argv[4], PORT_LENGTH - 1);
}

void *setupClientThread(void *args) {
  serverInputArgs inputArgs = *((serverInputArgs*) args);


  int server_fd = setupServerSocket(inputArgs);

  int epoll_fd = epoll_create1(0);

  // Setup Epoll
  struct epoll_event ev_server;
  ev_server.data.fd = server_fd;
  ev_server.events = EPOLLIN | EPOLLONESHOT;
  facade_epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev_server);

  if (epoll_fd == -1) {
    fprintf(stderr,"Failed to create epoll FD -  %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  facade_setToNonBlocking(server_fd);

  struct epoll_event events[MAX_EVENTS];
  struct epoll_event ev;
  bool isSessionActive = true;
  int availFds;
  int numOfActiveFds = 1;

  while (isSessionActive) {

    fprintf(stdout, "Waiting for epoll...\n");
    availFds = facade_epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (availFds == -1) {
      fprintf(stderr, "%s: ",__func__);
      perror("epoll_wait: ");
      isSessionActive = false;
    } else if (availFds == 0) { // Probably interrupted with a signal.
      fprintf(stderr, "%s: epoll_wait timed out with no readable FDs. Terminating. \n",__func__);
      isSessionActive = false;
    }

    fprintf(stdout, "Found epoll\n");
    for (int i = 0; i < availFds; i++) {
      if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR)) {
        fprintf(stderr, "%s: Socket has shutdown by peer or due to error. \n", __func__);
        closeAndRemoveFD(epoll_fd, events[i].data.fd);
        numOfActiveFds--;
      } else if ((events[i].events & EPOLLIN)) {
        fprintf(stdout, "Found something in epoll\n");
        //isSessionActive = rInfo->func(events[i].data.fd, rInfo->args);
        if (isSessionActive) {
          ev.data.fd = events[i].data.fd;
          ev.events = EPOLLIN | EPOLLONESHOT;
          if (facade_epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1) {
            fprintf(stderr, "%s: epoll_ctl failed. Errno: %s \n", __func__, strerror(errno));
          }
          facade_epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
        }
      } else {
        fprintf(stderr, "%s: Unknown EPOLL EVENT %d  \n", __func__, events[i].events);
        closeAndRemoveFD(epoll_fd, events[i].data.fd);
        numOfActiveFds--;
      }
    }

    fprintf(stdout, "ClientThread\n");
    sleep(1);
  }
}

void server_main(int argc, char **argv) {
  serverInputArgs args;

  parseServerArgs(argc, argv, &args);

  int nameServerSocket = establishConnectionWithNs(args);

  fprintf(stdout, "Socket value: %d\n", nameServerSocket);

  registerToServer(nameServerSocket, args);

  pthread_t receivingThread;
  int ret = pthread_create(&receivingThread, NULL, setupClientThread, (void *)&args);
  if (ret){
    fprintf(stderr, "Unable to create a pthread. Error: %d\n", ret);
  }

  bool running = true;
  while (running) {
    if (gotACKResponse(nameServerSocket)) {
      fprintf(stdout, "Still connected to server\n");
    } else {
      registerToServer(nameServerSocket, args);
      fprintf(stdout, "Lost contact with name server, connecting again\n");
    }
    sleep(8);
  }
  pthread_join(receivingThread, NULL);

  free(args.nameServerIP);
  free(args.serverName);
  free(args.nameServerPort);
}