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

#define NUM_BID_SEC 1/2
#define BID_PORT 5557
#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

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

static struct timeval first_bid_tv;
static struct timeval last_bid_tv;
static bool first_bid = true;
static int next_bid_id = 0;

static bid_t bids[10000];

static bool loaded = false;

char* BID_SERVER = "nile.cs.ucla.edu";

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
  cout << "start sending bid data" << endl;

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

  struct hostent *h;
  h = gethostbyname(BID_SERVER);
  if(h==NULL) {
    printf("unknown host '%s'\n", BID_SERVER);
    return -1;
  }

  sockaddr_in m_addr, l_addr;
  int rc;

  //reuse the connection forever for now
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
    m_addr.sin_port = htons ( (unsigned short) BID_PORT );

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
    
    if (first_bid) {
      totalNum = 10;
      gettimeofday(&last_bid_tv, &tz);
      gettimeofday(&first_bid_tv, &tz);
      first_bid = false;
    } else {
      gettimeofday(&tv, &tz);    
      totalNum = (tv.tv_sec - last_bid_tv.tv_sec)*NUM_BID_SEC;
    }

    //restart if more than 2000 sec has elapsed
    //for now, only run for 10000 bids, and start over again.        
    if ((tv.tv_sec - first_bid_tv.tv_sec) > 2000 || (next_bid_id) >= 10000) {
      first_bid = true;
      next_bid_id = 0;
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
      
      int count = next_bid_id++;
      //memcpy(data.data, (char*)&(bids[count].aucId), sizeof(int)); //auction id
      sprintf(buf, "%d,",  bids[count].aucId);
      
      //memcpy(data.data+sizeof(int), (char*)&bids[count].price, sizeof(int)); //price
      sprintf(temp, "%d,",  bids[count].price);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+sizeof(int), (char*)&bids[count].bidderId, sizeof(int)); //bidder id
      sprintf(temp, "%d,",  bids[count].bidderId);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+sizeof(int)+sizeof(int), (char*)&tv, sizeof(struct timeval)); //bid time
      char ts[60];
      struct tm* tmp = localtime(&tv.tv_sec);
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
      }  else {
	if (debug) cout << "sent " << buf << endl;
      }
    }
  
    gettimeofday(&last_bid_tv, &tz);
  }

  return 0;                     // Got data
}
