/* input.c */
#include <stdio.h>
#include <stdlib.h>
#include "nbc.h"

extern char domainName[];
extern char infoFile[];
extern int No_Features, No_Classes;
extern int linear[];
extern int className[], classIndex[];
extern INSTANCE trainData[], testData[];

error_exit (message1, message2)
char *message1, *message2;
{
  fprintf(stderr, "%s %s\n", message1, message2);
  exit(1);
}

separator(ch)
char ch;
{
  if (ch == ' ' || ch == ',' || ch == '\t')
    return(1);
  else
    return(0);
}

char *
read_next_value(char *into, char *from)
{
  int j;

  /* skip separators */
  while (separator(*from) && *from != '\n') from++;
  j=0; 
  while (!separator(*from) && *from != '\n') {
    into[j++]=*from; 
    from++; 
  }
  into[j]='\0';			/* into contains a value as a string */
  return from;
}

/* read domain dependent parameters from domain.info file */
void 
read_info()   
{
  FILE *fp;
  int f;
  char infoFile[MAX_NAME], Line[MAX_LINE], str[MAX_LINE], *line;

  strcpy(infoFile, domainName);
  strcat(infoFile, ".info");
  fp=fopen(infoFile,"r");
  if (fp==NULL)
    error_exit(infoFile, "file is not found...");

  while (fgets(Line,MAX_LINE,fp) != NULL) {
    if (sscanf(Line,"%s",str) <1) continue; /* ignore empty line */
    if (Line[0]==';') continue;	/* ignore comment line */

    line = read_next_value (str, Line);
    // printf("About to access features and str is %s \n", str);
    if (strcasecmp(str,"Features")==0) {
      /* read the number and type of features */
      No_Features = 0;
      // printf("Reading features!\n");
      while ((line=read_next_value(str,line)) && (*str != '\0')) {
	if (No_Features == MAX_FEATURES) 
	  error_exit ("No_Features has now reached to MAX_FEATURES\n",
		      "increase MAX_FEATURES in nbc.h, and recompile!");
        // printf("reading feature %s\n",str);
	if (*str == 'l')
	  linear[No_Features++] = TRUE;
	else
	  linear[No_Features++] = FALSE;
      }
    }
    else if (strcasecmp(str,"Classes")==0) {
      /* read number and names of classes */
      No_Classes = 0;
      while ((line=read_next_value(str,line)) && (*str != '\0')) {
	if (No_Classes == MAX_CLASSES)
	  error_exit ("No_Classes has now reached to MAX_CLASSES\n",
		      "increase MAX_CLASSES in nbc.h, and recompile!");
	className[++No_Classes] = atoi(str);
        // printf("reading class %i\n",atoi(str));
	classIndex[className[No_Classes]] = No_Classes;
        /* 0 < classIndex, 0 is used for UNDETERMINED class */
      }
    }
  }
  fclose(fp);
} /* read_info */

void 
read_an_instance(char *fname, INSTANCE *i, char *line)
{
  char value_in_string[MAX_LINE];
  int v;
  FEATURE f=0;
  CLASS c=0;

  /* read feature values */
  for (v = 0;  v < No_Features; v++) {
    // printf("p1\n");
    line = read_next_value(value_in_string, line);
    // printf("p2\n");
    if (value_in_string[0] == '?')
      i->status[f++]=UNKNOWN;
    else { 
    // printf("p3\n");
      sscanf(value_in_string, "%f", &(i->value[f]));
      // printf("p3.1\n");
      i->status[f++]=KNOWN;
      // printf("p3.2\n");
    }
  }
  /* read class value */
  line = read_next_value(value_in_string, line);
    // printf("p4\n");
  if (value_in_string[0] == '?') 
    error_exit ("Unknown (?) class value in", fname);

  sscanf(value_in_string, "%d", &c);
  // printf("classIndex[0] = %i but trying to access %i and value_in_string is %s\n", classIndex[0], c, value_in_string);
  if (classIndex[c] == 0) {
     printf ("value_in_string:%s<\n", value_in_string);
    error_exit ("Strange class name in", fname);
  }
  // printf("Still alive 1\n");
  i->Class = classIndex[c];
  // printf("Still alive 2\n");
} /* read_an_instance */

int 
read_train_data()
{
  FILE *fp;
  int n;
  char trainFile[MAX_NAME], line[MAX_LINE];

  strcpy(trainFile, domainName);
  strcat(trainFile, ".train");
  fp=fopen(trainFile, "r");
  if (fp==NULL) 
    error_exit ("cannot open training data file", trainFile);
   
  for (n=0; fgets(line, MAX_LINE, fp) != 0; n++){
    // printf("Start to read \n");
    read_an_instance(trainFile, &trainData[n%WINDOW_TR], line);
    // printf("finished reading \n");

  }
    // printf("Done with TS \n");

  fclose(fp);
  return(n);
} /* read_train_data */

int 
read_test_data()
{
  FILE *fp;
  int n;
  char testFile[MAX_NAME], line[MAX_LINE];

  strcpy(testFile, domainName);
  strcat(testFile, ".test");
  fp=fopen(testFile, "r");
  if (fp==NULL) 
    error_exit ("cannot open test data file", testFile);
  
  for (n=0; fgets(line, MAX_LINE, fp) != 0; n++)
    read_an_instance(testFile, &testData[n%WINDOW_TE], line);

  fclose(fp);
  return(n);
} /* read_test_data */
