#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream.h>
#include <fstream.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

//#include <dbt.h>
//#include <buffer.h>

//using namespace ESL;
using namespace std;

#define debug 1

#define NUM_AUC_SEC 1/5
#define AUCTION_PORT 5555
#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

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

static struct timeval last_auction_tv;
static struct timeval first_auction_tv;
static bool first_auction = true;
static int next_auction_id = 0;

static auction_t auctions[2500];

static bool loaded = false;

char* AUCTION_SERVER = "nile.cs.ucla.edu";

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


int main ( int argc, int argv[] )
{
  cout << "start sending auction data" << endl;
  
  //load all auctions from file once
  if (!loaded) {
    ifstream in("auctions.txt");
    
    if(!in) {
      cout << "file not found: auctions.txt" << endl;
      return -1;
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

  struct hostent *h;
  h = gethostbyname(AUCTION_SERVER);
  if(h==NULL) {
    printf("unknown host '%s'\n", AUCTION_SERVER);
    return -1;
  }

  sockaddr_in m_addr, l_addr;
  int rc;

  //reuse the connection forever for now.
    memset ( &m_addr, 0, sizeof ( m_addr ) );
    memset ( &l_addr, 0, sizeof ( l_addr ) );
    
    int m_sock = socket ( AF_INET, SOCK_STREAM, 0 );
    if (m_sock < 0) {
      cout << "error creating socket" << endl;
      return -1;
    }
    
    int on = 1;
    if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) < 0) {
      cout << "error setting socket options" << endl;
      return -1;
    }
    
    // local bind any port number
    l_addr.sin_family = AF_INET;
    l_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    l_addr.sin_port = htons((unsigned short)0);

    //remote server
    m_addr.sin_family = h->h_addrtype;
    memcpy((char *) &m_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    m_addr.sin_port = htons ( (unsigned short) AUCTION_PORT );

    rc = bind ( m_sock, ( struct sockaddr * ) &l_addr, sizeof ( l_addr ) );

    if ( rc < 0 ) {
      cout << "error binding local port" << endl;
      return -1;
    }
    
    //connect to server, check if get connection, if not, sleep for a while before reconnect
    while ((rc = connect ( m_sock, ( struct sockaddr * ) &m_addr, sizeof ( m_addr ) )) < 0) {
      if (debug) cout << "can not get tcp connection, sleep and retry." << endl;
      sleep(1);
      continue;
    }    
  
  while (1) {
    
    struct timeval tv;
    struct timezone tz;
    
    srand( time(NULL) );
    
    int totalNum = 0;
    
    gettimeofday(&tv, &tz);	    
    
    if (first_auction) {
      totalNum = 5;
      gettimeofday(&last_auction_tv, &tz);
      gettimeofday(&first_auction_tv, &tz);
      first_auction = false;
    } else {
      gettimeofday(&tv, &tz);    
      totalNum = (tv.tv_sec - last_auction_tv.tv_sec)*NUM_AUC_SEC;
    }
    
    //restart if more than 2000 sec has elapsed
    //for now, only run for 2500 auctions, and start over again.    
    if ((tv.tv_sec - first_auction_tv.tv_sec) > 2000 || (next_auction_id) >= 2500) {
      first_auction = true;
      next_auction_id = 0;
    }

    if (totalNum <= 0) {
      sleep(1);
      continue; // no data
    }
    
    
    for (int i = 0; i < totalNum; i++) {
      gettimeofday(&tv, &tz);

      //cDBT data(500, &tv);
      char buf[500];
      char temp[100];
      
      int id = next_auction_id++;      
      //memcpy(data.data, (char*)&(auctions[id].id), sizeof(int)); //id
      sprintf(buf, "%d,",  auctions[id].id);
      
      //strcpy(data.data+sizeof(int), auctions[id].itemName); //item name
      sprintf(temp, "%s,",  auctions[id].itemName);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+10, (char*)&auctions[id].sellerId, sizeof(int)); //seller id
      sprintf(temp, "%d,",  auctions[id].sellerId);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+10+sizeof(int), (char*)&auctions[id].price, sizeof(int)); //price
      sprintf(temp, "%d,",  auctions[id].price);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+10+sizeof(int)+sizeof(int), (char*)&auctions[id].catId, sizeof(int)); //catId
      sprintf(temp, "%d,",  auctions[id].catId);
      strcat(buf, temp);
      
      struct timeval tv2;
      gettimeofday(&tv, &tz);    
      gettimeofday(&tv2, &tz);
      tv2.tv_sec += auctions[id].expDiff;
      //memcpy(data.data+sizeof(int)+10+sizeof(int)+sizeof(int)+sizeof(int), (char*)&tv2, sizeof(struct timeval)); //expire time
      char ts[60];
      struct tm* tmp = localtime(&tv2.tv_sec);
      strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
      sprintf(temp, ".%d,", tv2.tv_usec);
      strcat(ts, temp);
      strcat(buf, ts);
      
      //memcpy(data.data+sizeof(int)+10+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(struct timeval), (char*)&tv, sizeof(struct timeval)); //input time
      ts[0] = '\0';
      tmp = localtime(&tv.tv_sec);
      strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
      sprintf(temp, ".%d\n", tv.tv_usec);
      strcat(ts, temp);
      strcat(buf, ts);

      //data.setTime(&tv);
      //dest->put(&data);
      rc = send(m_sock, buf, strlen(buf), 0);
      if(rc<0) {
	if (debug) cout << "error sending data, retry later" << endl;
	sleep(1);
	break;
      } else {
	if (debug) cout << "sent " << buf << endl;
      }  
    }
    
    gettimeofday(&last_auction_tv, &tz);
    //rc = close(m_sock);
    //if(rc<0) {
    //cout << "closing socket." << endl;
    //return -1;
    //} 
    
  }

  cout << "error: abnormal end sending data" << endl;
  
  return 0;
}
