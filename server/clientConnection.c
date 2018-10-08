#include "clientConnection.h"

int setupServerSocket(serverInputArgs args) {
  int server_fd;
  struct addrinfo* addrInfo = 0;

  fprintf(stdout, "Listening on port: %d\n", atoi(args.serverPort));

  fillInAddrInfo(&addrInfo, atoi(args.serverPort), NULL, SOCK_STREAM, AI_PASSIVE);

  server_fd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
  if (server_fd == -1) {
    fprintf(stderr, "Failed to create socket\n");
    exit(EXIT_FAILURE);
  }

  // Allow reuse of local addresses.
  int resueAddr = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(resueAddr), sizeof(&resueAddr)) == -1) {
    close(server_fd);
    fprintf(stderr, "Failed to set socket optionals\n");
    exit(EXIT_FAILURE);
  }

  if (bind(server_fd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
    close(server_fd);
    fprintf(stderr, "Failed to bind socket\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(addrInfo);

  if (listen(server_fd, SOMAXCONN)) {
    fprintf(stderr, "Failed to listen to socket\n");
    exit(EXIT_FAILURE);
  }
  return server_fd;
}

int listenForIncomingConnection(int server_fd) {
  int connection_fd;
  fprintf(stdout, "Listening for connections on %d...\n", server_fd);
  // Listen for ONE incoming connection.
  while((connection_fd = accept(server_fd, 0, 0)) == -1) {
    fprintf(stderr, "SERVER ERROR: %s", strerror(errno));
  }
  return connection_fd;
}