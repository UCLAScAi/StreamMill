#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int (*putDataInBuffer)(char*);

  printf("@ Step 0\n");
  void* handle = dlopen("./sample_parser.so", RTLD_NOW);
  char error[100];
  printf("@ Step 1\n");
  if (!handle) {
    sprintf(error, "In addIOModule, %s \n", dlerror());
    printf("Error: %s\n", error);
    return 1;
  }
  printf("@ Step 2\n");
  putDataInBuffer = (int(*)(char*))dlsym(handle, "putDataInBuffer");
  printf("@ Step 3\n");
  char data[100];
  printf("@ Step 4\n");
  sprintf(data, "%s", "hi,bye");
  char* testData = (char*)malloc(100*sizeof(char));
  char* delim = (char*)malloc(100*sizeof(char));
  char myTestData[100];
  delim = "e";
  testData = "BeforeCall";
  printf("In main func with input %s\n", testData);
  strtok(myTestData, delim);
  putDataInBuffer(data);
  printf("@ Step 3\n");
  return 1;
}
