#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
  char buf[100];
  sprintf(buf, "%s", "smm__traffic,10,1\n");
  char real_buf[100];
  char* tmp = strdup(buf);
  char* tok = strtok(tmp, ",\n");
  memcpy(real_buf, &buf[strlen(tmp) + 1], strlen(buf));
  printf("tok is: %s and buf is %s and tmp is %s buf 2 is %s\n", tok, buf, tmp, real_buf);
}
