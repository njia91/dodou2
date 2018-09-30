#include "pduCommon.h"

uint8_t calculateCheckSum(void *p, size_t size) {
  uint8_t ch = 0;
  for(int i = 0; i < size; i++){
    ch += *(uint8_t *)p++;
  }
  return ch;
}

size_t calculateNoOfWords(size_t packetSize){
  if (packetSize % WORD_SIZE == 0){
    return (packetSize / 4);
  } else {
    return (packetSize / 4) + 1;
  }
}

void convertTimeToString(char *timeStr, struct tm *timeInfo){
  strftime(timeStr, TIMESTR_LENGTH, "%A - %H:%M:%S", timeInfo);
 /* sprintf(timeStr, "%d/%d/%d %d:%d:%d",
          1900 + timeInfo->tm_year,
          timeInfo->tm_mon + 1,
          timeInfo->tm_mday,
          timeInfo->tm_hour,
          timeInfo->tm_min ,
          timeInfo->tm_sec);*/
}