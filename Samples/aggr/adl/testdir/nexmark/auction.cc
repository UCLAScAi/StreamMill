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

#define NUM_AUC_SEC 1/5

typedef struct auction_type
{
  int id;
  char itemName[11];
  int sellerId;
  int price;
  int catId;
  int expDiff;

  void print() 
  {
    cout << id << "," << itemName << "," << sellerId << "," << price << "," << catId << "," << expDiff << endl;
  }
} auction_t;

extern "C" int getTuple(buffer* dest);
extern "C" int closeConnection();

static struct timeval last_auction_tv;
static struct timeval first_auction_tv;
static bool first_auction = true;
static int last_auction_id = 0;

static auction_t auctions[2500];

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
  //cout << "auction:getTuple called" << endl;

  //load all auctions from file once
  if (!loaded) {
    ifstream in("auctions.txt");
    
    if(!in) {
      cout << "file not found: auctions.txt" << endl;
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
	auctions[count].id = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	stringPad(result, auctions[count].itemName, 11);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	auctions[count].sellerId = atoi(result);	
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	auctions[count].price = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	auctions[count].catId = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	//too long, make it to expire in 1-30 seconds
	auctions[count].expDiff = (atoi(result) % 30) + 1;	
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
  if ((tv.tv_sec - first_auction_tv.tv_sec) > 2000) {
    first_auction = true;
    last_auction_id = 0;
  }

  
  if (first_auction) {
    totalNum = 5;
    gettimeofday(&last_auction_tv, &tz);
    gettimeofday(&first_auction_tv, &tz);
    first_auction = false;
  } else {
    gettimeofday(&tv, &tz);    
    totalNum = (tv.tv_sec - last_auction_tv.tv_sec)*NUM_AUC_SEC;
  }

  if (totalNum <= 0 || (last_auction_id+1) >= 2500) return 2; // no data

    
  for (int i = 0; i < totalNum; i++) {
    gettimeofday(&tv, &tz);
    cDBT data(500, &tv);

    int id = ++last_auction_id;
    memcpy(data.data, (char*)&(auctions[id].id), sizeof(int)); //id
    strcpy(data.data+sizeof(int), auctions[id].itemName); //item name
    memcpy(data.data+sizeof(int)+10, (char*)&auctions[id].sellerId, sizeof(int)); //seller id
    memcpy(data.data+sizeof(int)+10+sizeof(int), (char*)&auctions[id].price, sizeof(int)); //price
    memcpy(data.data+sizeof(int)+10+sizeof(int)+sizeof(int), (char*)&auctions[id].catId, sizeof(int)); //catId

    struct timeval tv2;
    gettimeofday(&tv, &tz);    
    gettimeofday(&tv2, &tz);
    tv2.tv_sec += auctions[id].expDiff;
    memcpy(data.data+sizeof(int)+10+sizeof(int)+sizeof(int)+sizeof(int), (char*)&tv2, sizeof(struct timeval)); //expire time
    memcpy(data.data+sizeof(int)+10+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(struct timeval), (char*)&tv, sizeof(struct timeval)); //input time
    
    data.setTime(&tv);
    dest->put(&data);
  }
  
  gettimeofday(&last_auction_tv, &tz);

  return 0;                     // Got data
}

int closeConnection()
{
  return 0;
}
