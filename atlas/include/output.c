/* output.c */
#include <stdio.h>
#include <stdlib.h>
#include "nbc.h"

extern int No_Features, No_Classes;
extern int linear[];
extern int className[], classIndex[];
extern char domainName[];
extern FILE *logf;

FILE *fopen();

void 
open_log_file()
{
  char logFile[MAX_NAME];
 
  strcpy(logFile, domainName);
  strcat(logFile, ".log");
  logf = fopen(logFile, "w");
} /* open_log_file */

void 
log_instance(INSTANCE inst)
{
  FEATURE f;

  for (f = 0; f < No_Features; f++)
    if (inst.status[f] == KNOWN) 
      fprintf(logf, "%g ", inst.value[f]);
    else
      fprintf(logf, "%?????? ");
  fprintf(logf, ": %d\n", className[inst.Class]);
} /* log_instance */

void log_data(Data, size)  /* for diagnostic purposes */
INSTANCE *Data;
int size;
{
  int i;

  for (i = 0; i < size; i++) 
    log_instance(Data[i]);
}

void show_instance(inst) 
INSTANCE inst;
{
  FEATURE f;

  for (f = 0; f < No_Features; f++)
    if (inst.status[f] == KNOWN) 
      printf("%6g ", inst.value[f]);
    else
      printf("%?????? ");
  printf(": %d\n", className[inst.Class]);
}
  
void show_data(Data, size)  /* for diagnostic purposes */
INSTANCE *Data;
int size;
{
  int i;

  for (i = 0; i < size; i++) 
    show_instance(Data[i]);
}
