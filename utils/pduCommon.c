#include <bits/types/struct_tm.h>
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

void convertTimeToString(char *timeStr, struct tm *timeInfo){
  sprintf(timeStr, "%d/%d/%d %d:%d:%d",
          timeInfo->tm_year,
          timeInfo->tm_mon,
          timeInfo->tm_mday,
          timeInfo->tm_hour,
          timeInfo->tm_min,
          timeInfo->tm_sec);
}