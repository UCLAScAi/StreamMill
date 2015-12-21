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

#define NUM_BID_SEC 1/2

typedef struct bid_type
{
  int aucId;
  int price;
  int bidderId;  

  void print() 
  {
    cout << aucId << "," << price << "," << bidderId << endl;
  }
} bid_t;

extern "C" int getTuple(buffer* dest);
extern "C" int closeConnection();

static struct timeval first_bid_tv;
static struct timeval last_bid_tv;
static bool first_bid = true;
static int last_bid_count = 0;

static bid_t bids[10000];

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
    ifstream in("bids.txt");
    
    if(!in) {
      cout << "file not found: bids.txt" << endl;
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
	bids[count].aucId = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	bids[count].price = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	bids[count].bidderId = atoi(result);	
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
  if ((tv.tv_sec - first_bid_tv.tv_sec) > 2000) {
    first_bid = true;
    last_bid_count = 0;
  }

  if (first_bid) {
    totalNum = 10;
    gettimeofday(&last_bid_tv, &tz);
    gettimeofday(&first_bid_tv, &tz);
    first_bid = false;
  } else {
    gettimeofday(&tv, &tz);    
    totalNum = (tv.tv_sec - last_bid_tv.tv_sec)*NUM_BID_SEC;
  }

  if (totalNum <= 0 || (last_bid_count+1) >= 10000) return 2; // no data
  
  for (int i = 0; i < totalNum; i++) {
    gettimeofday(&tv, &tz);
    cDBT data(500, &tv);

    int count = ++last_bid_count;
    memcpy(data.data, (char*)&(bids[count].aucId), sizeof(int)); //auction id
    memcpy(data.data+sizeof(int), (char*)&bids[count].price, sizeof(int)); //price
    memcpy(data.data+sizeof(int)+sizeof(int), (char*)&bids[count].bidderId, sizeof(int)); //bidder id

    memcpy(data.data+sizeof(int)+sizeof(int)+sizeof(int), (char*)&tv, sizeof(struct timeval)); //bid time
    
    data.setTime(&tv);
    dest->put(&data);
  }
  
  gettimeofday(&last_bid_tv, &tz);

  return 0;                     // Got data
}

int closeConnection()
{
  return 0;
}
