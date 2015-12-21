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

//using namespace ESL;
using namespace std;

#define debug 1


#define SERVER "localhost"
#define TCP1_PORT 5600
#define TCP2_PORT 5610
#define TCP3_PORT 5620
#define TCP4_PORT 5630
#define TCP5_PORT 5640
#define TCP6_PORT 5650
#define TCP7_PORT 5660
#define TCP8_PORT 5670
#define TCP9_PORT 5680

#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

//everything is from the same file: tcp2.
#define ALL_COUNT 2661931

#define SLOW_DOWN 2.45

typedef struct tcp_type
{
  struct timeval a_time;
  int from_ip;
  int to_ip;
  int s_port;
  int d_port;
  int num_bytes;

  void print() 
  {
    cout << "tcp: " << a_time.tv_sec << "." << a_time.tv_usec << "," << from_ip << "," << to_ip << "," << s_port << "," << d_port << "," << num_bytes << endl;
  }
} tcp_t;

static tcp_t tcp[ALL_COUNT];

static bool loaded = false;

static struct timeval start_tv;

//tcp1 is index 613174 to 780668 (time 1000 - 1200)
static int tcp1_start_sec = 1000;
static int tcp1_next = 613174;
static int tcp1_end = 780668;

//tcp2 is index 2116017 to 2263462 (time 2900 - 3100)
static int tcp2_start_sec = 2900;
static int tcp2_next = 2116017;
static int tcp2_end = 2263462;

//tcp3 is index 2341049 to 2523152 (time 3200 - 3400)
static int tcp3_start_sec = 3200;
static int tcp3_next = 2341049;
static int tcp3_end = 2523152;

//tcp4 is index 432406 to 576532 (time 750 - 950)
static int tcp4_start_sec = 750;
static int tcp4_next = 432406;
static int tcp4_end = 576531;

//tcp5 is index 780669 to 930551 (time 1200 - 1400)
static int tcp5_start_sec = 1200;
static int tcp5_next = 780669;
static int tcp5_end = 930551;

//tcp6 is index 1083670 to 1245891 (time 1600 - 1800)
static int tcp6_start_sec = 1600;
static int tcp6_next = 1083670;
static int tcp6_end = 1245891;

//tcp7 is index 1537739 to 1705001 (time 2200 - 2400)
static int tcp7_start_sec = 2200;
static int tcp7_next = 1537739;
static int tcp7_end = 1705001;

//tcp8 is index 1705002 to 1875813 (time 2400 - 2600)
static int tcp8_start_sec = 2400;
static int tcp8_next = 1705002;
static int tcp8_end = 1875813;

//tcp9 is index 1875814 to 2042490 (time 2600 - 2800)
static int tcp9_start_sec = 2600;
static int tcp9_next = 1875814;
static int tcp9_end = 2042490;

static bool restart = true;

double
timeval_subtract(struct timeval x, struct timeval y)
{
  double result = x.tv_sec;
  result = result + ((double)x.tv_usec/1000000);
  result = result - y.tv_sec;
  result = result - ((double)y.tv_usec/1000000);

  return result;
}

int main ( int argc, char** argv )
{
  cout << "loading packets data from file..." << endl;

  //load all packets from file once
  if (!loaded) {

    char read[500];
    bool start = true;
    int count = 0;
    read[0] = '\0';
    
    cout << "loading tcp2 data file..." << endl;
    
    ifstream in("./tcp2.txt");
    
    if(!in) {
      cout << "file not found: tcp2.txt" << endl;
      return 2;
    }
    
    while(!in.eof() && (start || strlen(read) > 0)) {
      start = false;
      
      //while eof hasnt occured
      in.getline(read, 500);
      
      //cout << read << endl;
      if (strlen(read) > 0) {
	
	char delims[] = ". \t";
	char *result = NULL;
	result = strtok( read, delims );
	if ( result != NULL ) {
	  tcp[count].a_time.tv_sec = atoi(result);
	}
	  
	result = strtok( NULL, delims );
	if ( result != NULL ) {
	  tcp[count].a_time.tv_usec = atoi(result);
	}
	
	result = strtok( NULL, delims );
	if ( result != NULL ) {
	  tcp[count].from_ip = atoi(result);
	}
	  
	result = strtok( NULL, delims );
	if ( result != NULL ) {
	  tcp[count].to_ip = atoi(result);
	}
	  
	result = strtok( NULL, delims );
	if ( result != NULL ) {
	  tcp[count].s_port = atoi(result);
	}
	  
	result = strtok( NULL, delims );
	if ( result != NULL ) {
	  tcp[count].d_port = atoi(result);
	}
	
	result = strtok( NULL, delims );
	if ( result != NULL ) {
	  tcp[count].num_bytes = atoi(result);
	}
	  
	if ((count+1) % 1000 == 0) cout << "*";
	if ((count+1) % 150000 == 0) cout << endl;
	  
	count++;      
      }
    }
    cout << endl;
    in.close();
    cout << "tcp2 file loaded, with " << count << " packets" << endl;
      
  }

  cout << "data file loaded." << endl;

  struct hostent *h_tcp1;
  h_tcp1 = gethostbyname(SERVER);
  if(h_tcp1==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp2;
  h_tcp2 = gethostbyname(SERVER);
  if(h_tcp2==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp3;
  h_tcp3 = gethostbyname(SERVER);
  if(h_tcp3==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp4;
  h_tcp4 = gethostbyname(SERVER);
  if(h_tcp4==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp5;
  h_tcp5 = gethostbyname(SERVER);
  if(h_tcp5==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp6;
  h_tcp6 = gethostbyname(SERVER);
  if(h_tcp6==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp7;
  h_tcp7 = gethostbyname(SERVER);
  if(h_tcp7==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp8;
  h_tcp8 = gethostbyname(SERVER);
  if(h_tcp8==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp9;
  h_tcp9 = gethostbyname(SERVER);
  if(h_tcp9==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }


  sockaddr_in m_addr_tcp1, l_addr_tcp1, m_addr_tcp2, l_addr_tcp2, m_addr_tcp3, l_addr_tcp3;
  int rc_tcp1, rc_tcp2, rc_tcp3;
  int m_sock_tcp1, m_sock_tcp2, m_sock_tcp3;
  int on_tcp1, on_tcp2, on_tcp3;

  sockaddr_in m_addr_tcp4, l_addr_tcp4, m_addr_tcp5, l_addr_tcp5, m_addr_tcp6, l_addr_tcp6;
  int rc_tcp4, rc_tcp5, rc_tcp6;
  int m_sock_tcp4, m_sock_tcp5, m_sock_tcp6;
  int on_tcp4, on_tcp5, on_tcp6;

  sockaddr_in m_addr_tcp7, l_addr_tcp7, m_addr_tcp8, l_addr_tcp8, m_addr_tcp9, l_addr_tcp9;
  int rc_tcp7, rc_tcp8, rc_tcp9;
  int m_sock_tcp7, m_sock_tcp8, m_sock_tcp9;
  int on_tcp7, on_tcp8, on_tcp9;

  //prepare connection for tcp1 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp1, 0, sizeof ( m_addr_tcp1 ) );
  memset ( &l_addr_tcp1, 0, sizeof ( l_addr_tcp1 ) );
  
  m_sock_tcp1 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp1 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp1 = 1;
  if ( setsockopt ( m_sock_tcp1, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp1, sizeof ( on_tcp1 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp1.sin_family = AF_INET;
  l_addr_tcp1.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp1.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp1.sin_family = h_tcp1->h_addrtype;
  memcpy((char *) &m_addr_tcp1.sin_addr.s_addr, h_tcp1->h_addr_list[0], h_tcp1->h_length);
  m_addr_tcp1.sin_port = htons ( (unsigned short) TCP1_PORT );
  
  rc_tcp1 = bind ( m_sock_tcp1, ( struct sockaddr * ) &l_addr_tcp1, sizeof ( l_addr_tcp1 ) );
  
  if ( rc_tcp1 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp1 = connect ( m_sock_tcp1, ( struct sockaddr * ) &m_addr_tcp1, sizeof ( m_addr_tcp1 ) )) < 0) {
    if (debug) cout << "can not get tcp1 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }  
  
  //prepare connection for tcp2 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp2, 0, sizeof ( m_addr_tcp2 ) );
  memset ( &l_addr_tcp2, 0, sizeof ( l_addr_tcp2 ) );
  
  m_sock_tcp2 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp2 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp2 = 1;
  if ( setsockopt ( m_sock_tcp2, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp2, sizeof ( on_tcp2 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp2.sin_family = AF_INET;
  l_addr_tcp2.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp2.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp2.sin_family = h_tcp2->h_addrtype;
  memcpy((char *) &m_addr_tcp2.sin_addr.s_addr, h_tcp2->h_addr_list[0], h_tcp2->h_length);
  m_addr_tcp2.sin_port = htons ( (unsigned short) TCP2_PORT );
  
  rc_tcp2 = bind ( m_sock_tcp2, ( struct sockaddr * ) &l_addr_tcp2, sizeof ( l_addr_tcp2 ) );
  
  if ( rc_tcp2 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp2 = connect ( m_sock_tcp2, ( struct sockaddr * ) &m_addr_tcp2, sizeof ( m_addr_tcp2 ) )) < 0) {
    if (debug) cout << "can not get tcp2 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp3 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp3, 0, sizeof ( m_addr_tcp3 ) );
  memset ( &l_addr_tcp3, 0, sizeof ( l_addr_tcp3 ) );
  
  m_sock_tcp3 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp3 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp3 = 1;
  if ( setsockopt ( m_sock_tcp3, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp3, sizeof ( on_tcp3 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp3.sin_family = AF_INET;
  l_addr_tcp3.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp3.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp3.sin_family = h_tcp3->h_addrtype;
  memcpy((char *) &m_addr_tcp3.sin_addr.s_addr, h_tcp3->h_addr_list[0], h_tcp3->h_length);
  m_addr_tcp3.sin_port = htons ( (unsigned short) TCP3_PORT );
  
  rc_tcp3 = bind ( m_sock_tcp3, ( struct sockaddr * ) &l_addr_tcp3, sizeof ( l_addr_tcp3 ) );
  
  if ( rc_tcp3 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp3 = connect ( m_sock_tcp3, ( struct sockaddr * ) &m_addr_tcp3, sizeof ( m_addr_tcp3 ) )) < 0) {
    if (debug) cout << "can not get tcp3 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp4 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp4, 0, sizeof ( m_addr_tcp4 ) );
  memset ( &l_addr_tcp4, 0, sizeof ( l_addr_tcp4 ) );
  
  m_sock_tcp4 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp4 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp4 = 1;
  if ( setsockopt ( m_sock_tcp4, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp4, sizeof ( on_tcp4 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp4.sin_family = AF_INET;
  l_addr_tcp4.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp4.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp4.sin_family = h_tcp4->h_addrtype;
  memcpy((char *) &m_addr_tcp4.sin_addr.s_addr, h_tcp4->h_addr_list[0], h_tcp4->h_length);
  m_addr_tcp4.sin_port = htons ( (unsigned short) TCP4_PORT );
  
  rc_tcp4 = bind ( m_sock_tcp4, ( struct sockaddr * ) &l_addr_tcp4, sizeof ( l_addr_tcp4 ) );
  
  if ( rc_tcp4 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp4 = connect ( m_sock_tcp4, ( struct sockaddr * ) &m_addr_tcp4, sizeof ( m_addr_tcp4 ) )) < 0) {
    if (debug) cout << "can not get tcp4 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp5 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp5, 0, sizeof ( m_addr_tcp5 ) );
  memset ( &l_addr_tcp5, 0, sizeof ( l_addr_tcp5 ) );
  
  m_sock_tcp5 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp5 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp5 = 1;
  if ( setsockopt ( m_sock_tcp5, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp5, sizeof ( on_tcp5 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp5.sin_family = AF_INET;
  l_addr_tcp5.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp5.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp5.sin_family = h_tcp5->h_addrtype;
  memcpy((char *) &m_addr_tcp5.sin_addr.s_addr, h_tcp5->h_addr_list[0], h_tcp5->h_length);
  m_addr_tcp5.sin_port = htons ( (unsigned short) TCP5_PORT );
  
  rc_tcp5 = bind ( m_sock_tcp5, ( struct sockaddr * ) &l_addr_tcp5, sizeof ( l_addr_tcp5 ) );
  
  if ( rc_tcp5 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp5 = connect ( m_sock_tcp5, ( struct sockaddr * ) &m_addr_tcp5, sizeof ( m_addr_tcp5 ) )) < 0) {
    if (debug) cout << "can not get tcp5 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp6 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp6, 0, sizeof ( m_addr_tcp6 ) );
  memset ( &l_addr_tcp6, 0, sizeof ( l_addr_tcp6 ) );
  
  m_sock_tcp6 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp6 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp6 = 1;
  if ( setsockopt ( m_sock_tcp6, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp6, sizeof ( on_tcp6 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp6.sin_family = AF_INET;
  l_addr_tcp6.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp6.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp6.sin_family = h_tcp6->h_addrtype;
  memcpy((char *) &m_addr_tcp6.sin_addr.s_addr, h_tcp6->h_addr_list[0], h_tcp6->h_length);
  m_addr_tcp6.sin_port = htons ( (unsigned short) TCP6_PORT );
  
  rc_tcp6 = bind ( m_sock_tcp6, ( struct sockaddr * ) &l_addr_tcp6, sizeof ( l_addr_tcp6 ) );
  
  if ( rc_tcp6 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp6 = connect ( m_sock_tcp6, ( struct sockaddr * ) &m_addr_tcp6, sizeof ( m_addr_tcp6 ) )) < 0) {
    if (debug) cout << "can not get tcp6 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp7 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp7, 0, sizeof ( m_addr_tcp7 ) );
  memset ( &l_addr_tcp7, 0, sizeof ( l_addr_tcp7 ) );
  
  m_sock_tcp7 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp7 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp7 = 1;
  if ( setsockopt ( m_sock_tcp7, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp7, sizeof ( on_tcp7 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp7.sin_family = AF_INET;
  l_addr_tcp7.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp7.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp7.sin_family = h_tcp7->h_addrtype;
  memcpy((char *) &m_addr_tcp7.sin_addr.s_addr, h_tcp7->h_addr_list[0], h_tcp7->h_length);
  m_addr_tcp7.sin_port = htons ( (unsigned short) TCP7_PORT );
  
  rc_tcp7 = bind ( m_sock_tcp7, ( struct sockaddr * ) &l_addr_tcp7, sizeof ( l_addr_tcp7 ) );
  
  if ( rc_tcp7 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp7 = connect ( m_sock_tcp7, ( struct sockaddr * ) &m_addr_tcp7, sizeof ( m_addr_tcp7 ) )) < 0) {
    if (debug) cout << "can not get tcp7 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp8 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp8, 0, sizeof ( m_addr_tcp8 ) );
  memset ( &l_addr_tcp8, 0, sizeof ( l_addr_tcp8 ) );
  
  m_sock_tcp8 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp8 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp8 = 1;
  if ( setsockopt ( m_sock_tcp8, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp8, sizeof ( on_tcp8 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp8.sin_family = AF_INET;
  l_addr_tcp8.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp8.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp8.sin_family = h_tcp8->h_addrtype;
  memcpy((char *) &m_addr_tcp8.sin_addr.s_addr, h_tcp8->h_addr_list[0], h_tcp8->h_length);
  m_addr_tcp8.sin_port = htons ( (unsigned short) TCP8_PORT );
  
  rc_tcp8 = bind ( m_sock_tcp8, ( struct sockaddr * ) &l_addr_tcp8, sizeof ( l_addr_tcp8 ) );
  
  if ( rc_tcp8 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp8 = connect ( m_sock_tcp8, ( struct sockaddr * ) &m_addr_tcp8, sizeof ( m_addr_tcp8 ) )) < 0) {
    if (debug) cout << "can not get tcp8 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp9 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp9, 0, sizeof ( m_addr_tcp9 ) );
  memset ( &l_addr_tcp9, 0, sizeof ( l_addr_tcp9 ) );
  
  m_sock_tcp9 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp9 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp9 = 1;
  if ( setsockopt ( m_sock_tcp9, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp9, sizeof ( on_tcp9 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp9.sin_family = AF_INET;
  l_addr_tcp9.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp9.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp9.sin_family = h_tcp9->h_addrtype;
  memcpy((char *) &m_addr_tcp9.sin_addr.s_addr, h_tcp9->h_addr_list[0], h_tcp9->h_length);
  m_addr_tcp9.sin_port = htons ( (unsigned short) TCP9_PORT );
  
  rc_tcp9 = bind ( m_sock_tcp9, ( struct sockaddr * ) &l_addr_tcp9, sizeof ( l_addr_tcp9 ) );
  
  if ( rc_tcp9 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp9 = connect ( m_sock_tcp9, ( struct sockaddr * ) &m_addr_tcp9, sizeof ( m_addr_tcp9 ) )) < 0) {
    if (debug) cout << "can not get tcp9 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  cout << "start sending packets data" << endl;
  restart = true;

  int sec = 0;
  int sec_cnt = 0;
  long cnt = 0;
  double total_delay_in_send = 0;
  double max_delay_in_send = 0;

  cout.setf(ios_base::fixed);
  while (1) {
  
    struct timeval tv;
    struct timezone tz;
    
    if (restart) {
      gettimeofday(&start_tv, &tz);
      if (debug) cout << "start time: " << start_tv.tv_sec << "." << start_tv.tv_usec << endl;
      tcp1_next = 613174;
      tcp2_next = 2116017;
      tcp3_next = 2341049;
      tcp4_next = 432406;
      tcp5_next = 780669;
      tcp6_next = 1083670;
      tcp7_next = 1537739;
      tcp8_next = 1705002;
      tcp9_next = 1875814;

      cnt = 0;
      sec = 0;
      sec_cnt = 0;
      restart = false;
    }
    gettimeofday(&tv, &tz);

    //restart if more than 1 hour has elapsed or any id exceed its limit
    if ((tv.tv_sec - start_tv.tv_sec) > 3600 || tcp1_next >= tcp1_end || tcp2_next >= tcp2_end || tcp3_next >= tcp3_end 
	|| tcp4_next >= tcp4_end || tcp5_next >= tcp5_end || tcp6_next >= tcp6_end 
	|| tcp7_next >= tcp7_end || tcp8_next >= tcp8_end || tcp9_next >= tcp9_end) {
      restart = true;
      continue;
    }
  
    char buf[500];
    char temp[100];
    char ts[60];
        
    //do a "merge sort", take the earliest of the packets from the nine, and if time has exceeded when it is supposed to be sent, send it.    
    int minStream = 1;
    struct timeval minTime;
    tcp_t* p;
    minTime.tv_sec = tcp[tcp1_next].a_time.tv_sec - tcp1_start_sec;
    minTime.tv_usec = tcp[tcp1_next].a_time.tv_usec;
    if (((tcp[tcp2_next].a_time.tv_sec - tcp2_start_sec) < minTime.tv_sec) || ((tcp[tcp2_next].a_time.tv_sec - tcp2_start_sec) == minTime.tv_sec && tcp[tcp2_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 2;
      minTime.tv_sec = tcp[tcp2_next].a_time.tv_sec  - tcp2_start_sec;
      minTime.tv_usec = tcp[tcp2_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp3_next].a_time.tv_sec - tcp3_start_sec) < minTime.tv_sec) || ((tcp[tcp3_next].a_time.tv_sec - tcp3_start_sec) == minTime.tv_sec && tcp[tcp3_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 3;
      minTime.tv_sec = tcp[tcp3_next].a_time.tv_sec - tcp3_start_sec;
      minTime.tv_usec = tcp[tcp3_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp4_next].a_time.tv_sec - tcp4_start_sec) < minTime.tv_sec) || ((tcp[tcp4_next].a_time.tv_sec - tcp4_start_sec) == minTime.tv_sec && tcp[tcp4_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 4;
      minTime.tv_sec = tcp[tcp4_next].a_time.tv_sec  - tcp4_start_sec;
      minTime.tv_usec = tcp[tcp4_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp5_next].a_time.tv_sec - tcp5_start_sec) < minTime.tv_sec) || ((tcp[tcp5_next].a_time.tv_sec - tcp5_start_sec) == minTime.tv_sec && tcp[tcp5_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 5;
      minTime.tv_sec = tcp[tcp5_next].a_time.tv_sec - tcp5_start_sec;
      minTime.tv_usec = tcp[tcp5_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp6_next].a_time.tv_sec - tcp6_start_sec) < minTime.tv_sec) || ((tcp[tcp6_next].a_time.tv_sec - tcp6_start_sec) == minTime.tv_sec && tcp[tcp6_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 6;
      minTime.tv_sec = tcp[tcp6_next].a_time.tv_sec  - tcp6_start_sec;
      minTime.tv_usec = tcp[tcp6_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp7_next].a_time.tv_sec - tcp7_start_sec) < minTime.tv_sec) || ((tcp[tcp7_next].a_time.tv_sec - tcp7_start_sec) == minTime.tv_sec && tcp[tcp7_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 7;
      minTime.tv_sec = tcp[tcp7_next].a_time.tv_sec - tcp7_start_sec;
      minTime.tv_usec = tcp[tcp7_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp8_next].a_time.tv_sec - tcp8_start_sec) < minTime.tv_sec) || ((tcp[tcp8_next].a_time.tv_sec - tcp8_start_sec) == minTime.tv_sec && tcp[tcp8_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 8;
      minTime.tv_sec = tcp[tcp8_next].a_time.tv_sec  - tcp8_start_sec;
      minTime.tv_usec = tcp[tcp8_next].a_time.tv_usec;      
    } 
    if (((tcp[tcp9_next].a_time.tv_sec - tcp9_start_sec) < minTime.tv_sec) || ((tcp[tcp9_next].a_time.tv_sec - tcp9_start_sec) == minTime.tv_sec && tcp[tcp9_next].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 9;
      minTime.tv_sec = tcp[tcp9_next].a_time.tv_sec - tcp9_start_sec;
      minTime.tv_usec = tcp[tcp9_next].a_time.tv_usec;      
    } 

    int id;
    if (minStream == 1) {
      p = &(tcp[tcp1_next]);
      id = tcp1_next;
    } else if (minStream == 2) {
      p = &(tcp[tcp2_next]);
      id = tcp2_next;
    } else if (minStream == 3) {
      p = &(tcp[tcp3_next]);
      id = tcp3_next;
    } else if (minStream == 4) {
      p = &(tcp[tcp4_next]);
      id = tcp4_next;
    } else if (minStream == 5) {
      p = &(tcp[tcp5_next]);
      id = tcp5_next;
    } else if (minStream == 6) {
      p = &(tcp[tcp6_next]);
      id = tcp6_next;
    } else if (minStream == 7) {
      p = &(tcp[tcp7_next]);
      id = tcp7_next;
    } else if (minStream == 8) {
      p = &(tcp[tcp8_next]);
      id = tcp8_next;
    } else {
      p = &(tcp[tcp9_next]);
      id = tcp9_next;
    }    

    gettimeofday(&tv, &tz);

    //the actual time difference since start
    double diff1 = timeval_subtract(tv, start_tv);
    int new_sec = (int) diff1;
    
    if (new_sec > sec) {
      //output # sent for last second, and initialize for new second
      cout << sec << "\t" << sec_cnt << endl;
      cout.flush();	
      sec = new_sec;
      sec_cnt = 0;
    }
    
    double diff2 = (p->a_time.tv_sec+(double)p->a_time.tv_usec/1000000)*SLOW_DOWN;
    if (minStream == 1) {
      diff2 -= tcp1_start_sec*SLOW_DOWN;
    } else if (minStream == 2) {
      diff2 -= tcp2_start_sec*SLOW_DOWN;
    } else if (minStream == 3) {
      diff2 -= tcp3_start_sec*SLOW_DOWN;
    } else if (minStream == 4) {
      diff2 -= tcp4_start_sec*SLOW_DOWN;
    } else if (minStream == 5) {
      diff2 -= tcp5_start_sec*SLOW_DOWN;
    } else if (minStream == 6) {
      diff2 -= tcp6_start_sec*SLOW_DOWN;
    } else if (minStream == 7) {
      diff2 -= tcp7_start_sec*SLOW_DOWN;
    } else if (minStream == 8) {
      diff2 -= tcp8_start_sec*SLOW_DOWN;
    } else {
      diff2 -= tcp9_start_sec*SLOW_DOWN;
    }    
    
    double delay = diff1 - diff2;
    /*
    if (debug) {
      cout << "diff1 is " << diff1 << ", diff2 is " << diff2 << ", delay is " << delay << ", data is " << endl;
      p->print();
    }
    */

    if (delay >= 0) {
      //if (debug) cout << "minStream is " << minStream << ", index is " << id << endl;

      sprintf(buf, "%d,",  p->from_ip);
      //strcat(buf, temp);
      
      sprintf(temp, "%d,",  p->to_ip);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  p->s_port);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  p->d_port);
      strcat(buf, temp);
      
      sprintf(temp, "%d",  p->num_bytes);
      strcat(buf, temp);
      
      struct tm* tmp = localtime(&tv.tv_sec);
      strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
      sprintf(temp, ".%d\n", tv.tv_usec);
      strcat(ts, temp);
      strcat(buf, ts);

      if (minStream == 1) {
	rc_tcp1 = send(m_sock_tcp1, buf, strlen(buf), 0);
	if(rc_tcp1<0) {
	  if (debug) cout << "error sending tcp1 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp1 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp1, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 1 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp1_next++;
	}	
      } else if (minStream == 2) {
	rc_tcp2 = send(m_sock_tcp2, buf, strlen(buf), 0);
	if(rc_tcp2<0) {
	  if (debug) cout << "error sending tcp2 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp2 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp2, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp2_next++;
	}	
      } else if (minStream == 3) {
	rc_tcp3 = send(m_sock_tcp3, buf, strlen(buf), 0);
	if(rc_tcp3<0) {
	  if (debug) cout << "error sending tcp3 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp3 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp3, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp3_next++;
	}	
      } else if (minStream == 4) {
	rc_tcp4 = send(m_sock_tcp4, buf, strlen(buf), 0);
	if(rc_tcp4<0) {
	  if (debug) cout << "error sending tcp4 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp4 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp4, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp4_next++;
	}	
      } else if (minStream == 5) {
	rc_tcp5 = send(m_sock_tcp5, buf, strlen(buf), 0);
	if(rc_tcp5<0) {
	  if (debug) cout << "error sending tcp5 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp5 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp5, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp5_next++;
	}	
      } else if (minStream == 6) {
	rc_tcp6 = send(m_sock_tcp6, buf, strlen(buf), 0);
	if(rc_tcp6<0) {
	  if (debug) cout << "error sending tcp6 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp6 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp6, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp6_next++;
	}	
      } else if (minStream == 7) {
	rc_tcp7 = send(m_sock_tcp7, buf, strlen(buf), 0);
	if(rc_tcp7<0) {
	  if (debug) cout << "error sending tcp7 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp7 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp7, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp7_next++;
	}	
      } else if (minStream == 8) {
	rc_tcp8 = send(m_sock_tcp8, buf, strlen(buf), 0);
	if(rc_tcp8<0) {
	  if (debug) cout << "error sending tcp8 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp8 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp8, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp8_next++;
	}	
      } else {
	rc_tcp9 = send(m_sock_tcp9, buf, strlen(buf), 0);
	if(rc_tcp9<0) {
	  if (debug) cout << "error sending tcp9 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	  /*
	  if (debug) {
	    cout << "sent tcp9 #" << id << ", values ";
	    p->print();
	    cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    cout << "sent tcp9, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 3 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  */
	  tcp9_next++;
	}	
      }

      //cout << delay << " ";
      if (cnt%100 == 0) {
	//cout << cnt << "\t" << total_delay_in_send/((double)100*SLOW_DOWN) << "\t" << max_delay_in_send/(double)SLOW_DOWN << endl;
	//cout.flush();	
	total_delay_in_send = 0;
	max_delay_in_send = 0;
      } else {
	total_delay_in_send += delay;
	if (max_delay_in_send < delay) max_delay_in_send = delay;
      }

      //cout << diff2 << "\t" << diff1 << "\t" << tv_sec_pac+(double)tv_usec_pac/1000000 << "\t" << tv_sec1+(double)tv_usec1/1000000 << endl;
      
    }    

  } //end while(1)
  
  return 0;                     // Got data
}
