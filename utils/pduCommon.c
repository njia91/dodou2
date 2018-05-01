#include "pduCommon.h"

uint8_t calculateCheckSum(void *p, int size){
  char *pdu = (char *)p;
  uint8_t ch = 0;;
  for(int i = 0; i < size; i++){
    ch += pdu[i];
  }
  return ch;
}