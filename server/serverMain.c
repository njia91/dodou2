#include <locale.h>
#include "server.h"

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");
  server_main(argc, argv);
}