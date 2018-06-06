#include "pduCommon.h"

uint8_t calculateCheckSum(void *p, int size){
  char *pdu = (char *)p;
  uint8_t ch = 0;;
  for(int i = 0; i < size; i++){
    ch += pdu[i];
  }
  return ch;
}

int calculateNoOfWords(int packetSize){
  if (packetSize % WORD_SIZE == 0){
    return (packetSize / 4);
  } else {
    return (packetSize / 4) + 1;
  }
}