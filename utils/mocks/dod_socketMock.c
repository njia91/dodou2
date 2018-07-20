//
// Created by njia on 2018-07-20.
//
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <dod_socket.h>

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

