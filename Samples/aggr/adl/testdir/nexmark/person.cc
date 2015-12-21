#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream.h>
#include <fstream.h>

#include <dbt.h>
#include <buffer.h>
using namespace ESL;
using namespace std;


#define NUM_PERSON_SEC 5/100

typedef struct person_type
{
  int id;
  char name[21];
  char email[21];
  int creditCard;
  char city[21];
  char state[3];
  //struct timeval reg_time;

  void print() 
  {
    cout << id << "," << name << "," << email << "," << creditCard << "," << city << "," << state << endl;
  }
} person_t;

extern "C" int getTuple(buffer* dest);
extern "C" int closeConnection();

static struct timeval last_person_tv;
static struct timeval first_person_tv;
static int last_person_id = 0;
static bool first_person = true;

static person_t persons[500];

static bool loaded = false;

void stringPad(char* src, char* dest, int length)
{
  int strlength = strlen(src);
  int i = 0;
  for(i; i < strlength; i++)
  {
    dest[i] = src[i];
  }
  for(i; i < length-1; i++)
  {
    dest[i] = ' ';
  }
  dest[length-1] = '\0';
}


int getTuple(buffer* dest)
{
  //cout << "person:getTuple called" << endl;

  //load all persons from file once
  if (!loaded) {
    ifstream in("persons.txt");
    
    if(!in) {
      cout << "file not found: persons.txt" << endl;
      return 2;
    }
    
    char read[200];
    int count = 0;
    while(!in.eof()) {
      //while eof hasnt occured
      in.getline(read, 200);
      char delims[] = ",";
      char *result = NULL;
      result = strtok( read, delims );
      if ( result != NULL ) {
	persons[count].id = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	stringPad(result, persons[count].name, 21);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	stringPad(result, persons[count].email, 21);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	persons[count].creditCard = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	stringPad(result, persons[count].city, 21);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	stringPad(result, persons[count].state, 3);
      }
      
      count++;      
    }
    
    loaded = true;
  }
  
  struct timeval tv;
  struct timezone tz;
  
  srand( time(NULL) );

  int totalNum = 0;
  
  gettimeofday(&tv, &tz);
  //restart if more than 2000 sec has elapsed
  if ((tv.tv_sec - first_person_tv.tv_sec) > 2000) {
    first_person = true;
    last_person_id = 0;
  }
  
  if (first_person) {
    totalNum = 3;
    gettimeofday(&last_person_tv, &tz);
    gettimeofday(&first_person_tv, &tz);
    first_person = false;
  } else {
    gettimeofday(&tv, &tz);    
    totalNum = (tv.tv_sec - last_person_tv.tv_sec)*NUM_PERSON_SEC;
  }

  //for now, only run for 500 persons, and stop producing data.
  if (totalNum <= 0 || (last_person_id+1) >= 500) return 2; // no data
  
  for (int i = 0; i < totalNum; i++) {
    gettimeofday(&tv, &tz);
    cDBT data(500, &tv);

    int id = ++last_person_id;
    memcpy(data.data, (char*)&(persons[id].id), sizeof(int)); //id
    strcpy(data.data+sizeof(int), persons[id].name); //name
    strcpy(data.data+sizeof(int)+20, persons[id].email); //email
    memcpy(data.data+sizeof(int)+40, (char*)&persons[id].creditCard, sizeof(int)); //credit_card
    strcpy(data.data+sizeof(int)+40+sizeof(int), persons[id].city); //city
    strcpy(data.data+sizeof(int)+40+sizeof(int)+20, persons[id].state); //state
    memcpy(data.data+sizeof(int)+100+sizeof(int)+22, (char*)&tv, sizeof(struct timeval)); //time
    data.setTime(&tv);
    dest->put(&data);
  }
  
  gettimeofday(&last_person_tv, &tz);

  return 0;                     // Got data
}

int closeConnection()
{
  return 0;
}
