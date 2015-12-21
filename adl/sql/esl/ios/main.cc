#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

int main()
{

  void* handle = dlopen("./iomodule.so", RTLD_NOW);
  char* error;

  if(!handle)
  {
    printf("%s\n", dlerror());
    exit(1);
  }

  /*iomodule->getTuple = (int(*)(dbt*))dlsym(handle, "getTuple");
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    exit(1);
  }*/
  printf("Here\n");

}
