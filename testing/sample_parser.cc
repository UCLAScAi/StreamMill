#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" int putDataInBuffer(char* dataStr) {
  char* testData = (char*)malloc(100*sizeof(char));
  testData = "hello";
  printf("In putDataBuf with input %s\n", dataStr);
  char* tok = strtok(dataStr, "i");
  printf("String %s Tok: %s\n", dataStr, tok);

  while(tok) {
    printf("String %s Tok: %s\n", dataStr, tok);
    tok = strtok(NULL, ",\n");

    if (!tok) {
      printf("data format error\n");
      return 1;
    }
    tok = strtok(NULL, ",\n");
  }
  printf("Parsing complete.");
  return 1;
}
