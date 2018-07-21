//
// Created by njia on 2018-07-20.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "dod_socket.h"
#include "client.h"


//extern int createSocket(struct addrinfo **res);

int createSocket(struct addrinfo **res){
  return (int)mock();
}

int bindSocket(int socket_fd, struct addrinfo **res){
  return (int)mock();
}

int connectToServer(int socket_fd, struct addrinfo **res){
  check_expected(socket_fd);
  return (int)mock();
}


int acceptConnections(int socket_fd, void (*fPtr)()){
  return (int)mock();
}

int markSocketAsPassive(int socket_fd){
  return (int)mock();
}

int setsocketopt(int socket_fd, int lvl, int opt){
  return (int)mock();
}

int setToNonBlocking(int socket_fd){
  return (int)mock();
}

int writeToSocket(int socket_fd, char *packet){
  return (int)mock();
}

int readFromSocket(int socket_fd, char *packet){
  packet = (char *)mock();
  return (int)mock();
}

void clientTest_connectToNameServer(void **state)
{
// Return a single customer ID when mock_query_database() is called.
  int fd = 545;
  will_return(createSocket, fd);
  will_return(connectToServer, 0);
  expect_value(connectToServer, socket_fd, fd);

  will_return(writeToSocket, 0);

  char *argv[5] = {"client", "ns", "123.0.0.1", "1234"};
  client_main(4, argv);
}


int main(int argc, char* argv[]) {
  const struct CMUnitTest tests[] = { cmocka_unit_test(clientTest_connectToNameServer)};
  return cmocka_run_group_tests(tests, NULL, NULL);
  return 0;
}