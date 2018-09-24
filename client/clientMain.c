//
// Created by michael on 2018-09-16.
//

#include <locale.h>
#include "client.h"


int main(int argc, char **argv){
  setlocale(LC_CTYPE, "");
  client_main(argc, argv);
}