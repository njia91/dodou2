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



void clientTest_connectToNameServer(void **state)
{
// Return a single customer ID when mock_query_database() is called.
  int fd = 545;
  will_return(createSocket, fd);
  will_return(connectToServer, 0);
  expect_value(connectToServer, socket_fd, fd);


  char *argv[5] = {"client", "ns", "123.0.0.1", "1234"};
  client_main(4, argv);
}


int main(int argc, char* argv[]) {
  const struct CMUnitTest tests[] = { cmocka_unit_test(clientTest_connectToNameServer)};
  return cmocka_run_group_tests(tests, NULL, NULL);
  return 0;
}