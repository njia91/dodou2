//
// Created by njia on 2018-07-20.
//

#include "client.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <dod_socket.h>
#include "dod_socket.h"

int mock_createSocket(struct addrinfo **res){
  return (int)mock();
}

int mock_bindSocket(int socket_fd, struct addrinfo **res){
  return (int)mock();
}

int mock_connectToServer(int socket_fd, struct addrinfo **res){
  return (int)mock();
}


int mock_acceptConnections(int socket_fd, void (*fPtr)()){
  return (int)mock();
}

int mock_markSocketAsPassive(int socket_fd){
  return (int)mock();
}

int mock_setsocketopt(int socket_fd, int lvl, int opt){
  return (int)mock();
}

int mock_setToNonBlocking(int socket_fd){
  return (int)mock();
}

int mock_writeToSocket(int socket_fd, char *packet){
  return (int)mock();
}

int mock_readFromSocket(int socket_fd, char *packet){
  packet = (char *)mock();
  return (int)mock();
}


int main(int argc, char* argv[]) {
//  const UnitTest tests[] = { unit_test(test_connect_to_customer_database),
//                             unit_test(fail_connect_to_customer_database),
//                             unit_test(test_get_customer_id_by_name), };
//  return run_tests(tests);
  return 0;
}