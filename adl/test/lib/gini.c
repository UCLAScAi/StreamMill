//#include <dlfcn.h>
//#include "adllib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char * _allocateResultSpace(int size);
extern void _adl_dlm_init();
extern void _adl_dlm_delete();
int gini(int a)
{
  return a*a;
}


char* test_str(char s[4097]){
  char *r;
  int i;
  r= _allocateResultSpace(4097);
  for (i = 0; i < 4097; i++){
    if (s[i])
      r[i] = s[i] + 1;
    else
      r[i] = 0;
  }
  r[4096] = 0;
  return r;
}

char test_char(char s[4096]){
  return s[0];
}

int main(){
  char abc[4096];
  strcpy(abc, "a");
  //  _adl_dlm_init();
  printf("%d\n", gini(2));
  printf("%s\n", test_str(abc));
  //  _adl_dlm_delete();
}
 
 
