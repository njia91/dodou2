//
// Created by njia on 2018-07-20.
//
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <dod_socket.h>


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

int writeToSocket(int socket_fd, uint8_t *packet, int size) {
  return (int)mock();
}

int readFromSocket(int socket_fd, uint8_t *buffer, int size) {
  buffer = (char *)mock();
  return (int)mock();
}
