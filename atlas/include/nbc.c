/* a new nbc.c: Faster Naive Bayesian Classifier */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "nbc.h"

int No_Features, No_Classes, No_Instances;
int No_Trainers, No_Testers, Train_correct, Test_correct;
int linear[MAX_FEATURES];
int verbosity=0;
char domainName[MAX_NAME], resName[MAX_NAME];
int className[MAX_CLASSES], classIndex[MAX_CLASSES];
INSTANCE trainData[MAX_DATA], testData[MAX_DATA];
int classCount[MAX_CLASSES];
double g[MAX_CLASSES];
char trainDataFile[MAX_NAME], testDataFile[MAX_NAME];

VALCOUNT distribution[MAX_FEATURES][MAX_CLASSES][MAX_DATA];
int noValues[MAX_FEATURES][MAX_CLASSES];/*# of values of feature f of class c*/

FILE *logf;

struct timeval *start_train, *end_train, *end_test;
struct timezone *tz;
int train_time, test_time;

/* ------------------------------------------------------------------------ */
int
time_diff(struct timeval *finish, struct timeval *start)
/* time difference in miliseconds */
{ int time_in_sec, time_in_usec;

  time_in_sec = finish->tv_sec - start->tv_sec;
  if (time_in_sec < 0) time_in_sec += 60;
  /* Here, we assume time_diffrenece can not be more than a minute */
  time_in_usec = finish->tv_usec - start->tv_usec;
  return((time_in_sec * 1000) + (time_in_usec / 1000));
} /* time_diff */

/* ------------------------------------------------------------------------ */
float 
average(float *array, int size)
{
  float sum=0.0;
  int i;

  for (i=0; i < size; i++)
    sum += array[i];
  return sum / size;
} /* average */

/* ------------------------------------------------------------------------ */
parse_options(int argc, char *argv[])
{
  strcpy(domainName, argv[1]);
  if (argc >= 3 && (strcmp(argv[2], "-v")==0)) {
    if (argc == 3) verbosity = 1; /* default is the least verbosity */
    else sscanf(argv[3], "%d", &verbosity);
    open_log_file();
  }
} /* parse_options */

/* ------------------------------------------------------------------------ 
   --- inserts the new value on current dimension in increasing order   --- */
void
insert(INSTANCE x, FEATURE f)
{
  int i, j=0;
  int done = FALSE;
  CLASS c;

  for (j=0;( (j < noValues[f][x.Class]) && (!done) ); j++) {
    if ( distribution[f][x.Class][j].value >= x.value[f] )
      done = TRUE; /* a larger or equal value of x.value[f] is seen before */
  } /* j-1 is the index of that larger or equal value in the array */

  if ( done ) {
    if ( x.value[f] == distribution[f][x.Class][j-1].value )
      distribution[f][x.Class][j-1].count++; /*if x.value[f] has seen before*/
    else { /* a larger value than x.value[f] has seen before */
      for (i=noValues[f][x.Class]; i>=j; i--) { /* shift right */
	distribution[f][x.Class][i].value =distribution[f][x.Class][i-1].value;
	distribution[f][x.Class][i].count =distribution[f][x.Class][i-1].count;
      }
      distribution[f][x.Class][j-1].value = x.value[f];
      distribution[f][x.Class][j-1].count = 1;
      noValues[f][x.Class]++;
    }
  }
  else { /* x.value[f] is the largest value seen so far, put it at the end */
    distribution[f][x.Class][j].value = x.value[f];
    distribution[f][x.Class][j].count = 1;
    noValues[f][x.Class]++;
  }
} /* insert */

/* ------------------------------------------------------------------------ */
void
train ()
{
  FEATURE f;
  CLASS nowc;
  int index;
  
  for (index=0; index < No_Trainers; index++) {
    for (f=0; f < No_Features; f++) {
      if (trainData[index].status[f] == KNOWN) {
	nowc = trainData[index].Class;
	if (noValues[f][nowc] == 0) {  /* first value will be put */ 
	  distribution[f][nowc][0].value = trainData[index].value[f];
	  distribution[f][nowc][0].count = 1;
	  noValues[f][nowc] = 1;
	}
	else  /* not first value on the current [f, c] dimension */
	  insert(trainData[index], f);
      }
    }
  }
} /* train */

/* ------------------------------------------------------------------------ */
void 
initializeNBC()
{
  int i, f, c;

  read_info();

  if (verbosity > 0) {
    fprintf(logf,"No_Features: %d\n", No_Features);
    fprintf(logf,"No_Classes: %d\n", No_Classes);
    fflush(logf);
  }

  No_Trainers = read_train_data(); /* initializes trainData from domain.train*/
  
  for (i=0; i<No_Trainers; i++)  /* count classes in the train data */
    classCount[trainData[i].Class]++;

  for (f=0; f<No_Features; f++)
    for (c=0; c<=No_Classes; c++) {
      noValues[f][c] = 0; /* set no. of value of f for class c to 0 */
      for (i=0; i<No_Trainers; i++)  
	distribution[f][c][i].count = 0;  /* set count for each f and c to 0 */
    }

  No_Testers  = read_test_data();  /* initializes testData  from domain.test */

  if (verbosity > 3) {
    fprintf(logf, "train data:\n");
    log_data(trainData, No_Trainers); 
    fprintf(logf, "test data:\n");
    log_data(testData, No_Testers); 
  }
  if (verbosity > 0) {
    fprintf(logf,"No_Trainers: %d\n", No_Trainers);
    fprintf(logf,"No_Testers: %d\n", No_Testers);
    for (c=0; c <= No_Classes; c++) 
      fprintf(logf, "classCount[%d]:%d ", className[c], classCount[c]);
    fprintf(logf, "\n");
    fflush(logf);
  }

  train ();  /* computes the distributions */

} /* initialize */


float 
p(VALUE x, FEATURE f, CLASS c)
{
  int i, f1, c1;    /* instance, feature, and class index */
  float density=0.0;
  int done=FALSE;

  if ( (classCount[c] == 0) || (noValues[f][c] == 0) ) return(0.0);

  if ( !linear[f] ) {  /* nominal */
      for (i=0; i<noValues[f][c]; i++) {
	if ( x == distribution[f][c][i].value )
	  density = (float) distribution[f][c][i].count / classCount[c];
      }
    } 
  else { /* linear */
    for (i=0; (i<noValues[f][c] && !done); i++)  {
      if ( x == distribution[f][c][i].value ) {
	done = TRUE;
	if ( (i==0) && (noValues[f][c]==1) ) /*x is the only value for this feature*/
	  density = 1.0;
	else if ( i==0 ) /* x is the smallest value */
	  density =distribution[f][c][i].count/(distribution[f][c][i+1].value-x)/
	    classCount[c];
	else if ( i==noValues[f][c]-1 ) /* x is the largest value */
	  density =distribution[f][c][i].count/(x-distribution[f][c][i-1].value)/
	    classCount[c];
	else /* x is in the middle of some values */
	  density = distribution[f][c][i].count / 
	    (distribution[f][c][i+1].value-distribution[f][c][i-1].value) * 2 /
	      classCount[c];
      }
      else if ( x < distribution[f][c][i].value ) {
	done = TRUE;
	if ( i==0 ) /* largest smaller is -INFINITE */
	  density =distribution[f][c][i].count/(distribution[f][c][i].value-x) /
	    classCount[c];
	else  /* sml_larger is distribution[f][c][i].value, lrg_smaller exists */
	  density = (distribution[f][c][i].count+distribution[f][c][i-1].count)/2.0/ 
	    classCount[c]/(distribution[f][c][i].value-distribution[f][c][i-1].value);
      }
    }

    if ( !done ) /* eqc=0 and sml_larger is INFINITE */
      density = distribution[f][c][i-1].count/(x-distribution[f][c][i-1].value) /
	                                     classCount[c];
  } /* else linear  */
  if (verbosity>2) 
    fprintf(logf, "c: %d, f: %d density: %g\n", className[c], f, density);

  if (verbosity>3) {
    for (f1=0; f1 < No_Features; f1++) {
      for (c1=1; c1<=No_Classes; c1++) {
	for (i=0; i<noValues[f][c]; i++)
	  fprintf(logf, "Feature %d has %d of value %5.2g in CLASS %d\n", f1+1, 
		  distribution[f1][c1][i].count, distribution[f1][c1][i].value, c1);
      }
      fprintf(logf, "\n"); 
    } 
  }

  return(density * 100);
} /* p */

/* ------------------------------------------------------------------------ */
float 
nbc ()
{
  int i;
  FEATURE f;
  CLASS c, prediction;
  int correct_count=0;
  float test_acc=0;

  for (i=0; i< No_Testers; i++) {
    prediction = 0;
    if (verbosity > 0) fprintf(logf,"\n");
    if (verbosity > 2) {
      fprintf (logf, "Test instance no: %d\n", i);
      for (f=0; f< No_Features; f++) 
	fprintf(logf, "%g ", testData[i].value[f]);
      fprintf(logf, "\n");
      fflush(logf);
    }

    for (c=0; c <= No_Classes; c++) {
      g[c] = (float)classCount[c] / No_Trainers; 
      for (f=0; f< No_Features; f++) 
	if (testData[i].status[f] == KNOWN)
	  g[c] = g[c] * p(testData[i].value[f], f, c);
      if (g[c] > g[prediction]) prediction = c; /* find c with max g[c] */
    }

    if (prediction == testData[i].Class) correct_count++;

    if (verbosity > 1) {
      for (c=0; c <= No_Classes; c++) 
	fprintf(logf, "g[%d]:%g ", className[c], g[c]);
      fprintf(logf, "\n");
    }
    if (verbosity > 0) {
      fprintf(logf, "%d: prediction: %d actual: %d\n", 
	      i, className[prediction], className[testData[i].Class]);
      fflush(logf);
    }
  }
  
  test_acc = (float)correct_count/No_Testers;
  if (verbosity>0) 
    fprintf (logf, "Test accuracy: %g\n", test_acc);

  return (test_acc);
} /* nbc */
  
/* ------------------------------------------------------------------------ */
main(argc, argv)
int argc;
char *argv[];
{

  FILE *fopen(), *resultf;
  float accuracy;
  int c, i, f;
  /*
  if (argc == 1) {
    printf("nbc domain -v verbosity\n");
    exit(1);
  }
  else
    parse_options(argc,argv);
  */
  start_train = (struct timeval *) malloc(sizeof(struct timeval));
  end_train   = (struct timeval *) malloc(sizeof(struct timeval));
  end_test    = (struct timeval *) malloc(sizeof(struct timeval));
  tz = (struct timezone *) malloc(sizeof(struct timezone)); 

  gettimeofday(start_train, tz);
  initializeNBC("data");
  gettimeofday(end_train, tz);

  accuracy = nbc(); 
  gettimeofday(end_test, tz);

  train_time = time_diff (end_train, start_train);
  test_time  = time_diff (end_test, end_train);

  sprintf(resName,"%s.res", domainName);
  resultf = fopen(resName, "w");
  fprintf (resultf, "Accuracy: %5.3f Train: %d Test: %d msec.\n", 
	   accuracy, train_time, test_time);
  if (verbosity>0)
    fprintf (logf,"Accuracy: %5.3f Train: %d Test: %d msec.\n", 
	   accuracy, train_time, test_time); 
  fclose(resultf);
  if (verbosity>0) fclose(logf);
  return 0;
}  /* main */







