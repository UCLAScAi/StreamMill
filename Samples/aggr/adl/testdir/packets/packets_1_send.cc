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

//udp and sf NOT WORKING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define SERVER "nile.cs.ucla.edu"
#define TCP_PORT 5600
#define UDP_PORT 5601
#define SF_PORT 5602
#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

//#define TCP_COUNT 2153462
#define TCP_COUNT 310408
#define UDP_COUNT 829759
#define SF_COUNT 79780

#define SLOW_DOWN 50

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
    cout << "tcp1: " << a_time.tv_sec << "." << a_time.tv_usec << "," << from_ip << "," << to_ip << "," << s_port << "," << d_port << "," << num_bytes << endl;
  }
} tcp_t;

typedef struct udp_type
{
  struct timeval a_time;
  int from_ip;
  int to_ip;
  int s_port;
  int d_port;

  void print() 
  {
    cout << "udp1: " << a_time.tv_sec << "." << a_time.tv_usec << "," << from_ip << "," << to_ip << "," << s_port << "," << d_port << endl;
  }
} udp_t;

typedef struct sf_type
{
  struct timeval a_time;
  int from_ip;
  int to_ip;
  int s_port;
  int d_port;
  char flags[3];

  void print() 
  {
    cout << "sin/fin1: " << a_time.tv_sec << "." << a_time.tv_usec << "," << from_ip << "," << to_ip << "," << s_port << "," << d_port << "," << flags << endl;
  }
} sf_t;

static tcp_t tcps[TCP_COUNT];
static udp_t udps[UDP_COUNT];
static sf_t sfs[SF_COUNT];

static bool loaded = false;

static struct timeval start_tv;

static int next_tcp_count = 0;
static int next_udp_count = 0;
static int next_sf_count = 0;

static bool restart = true;

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

//invoke with switches '-t' (send tcp data), '-u' (send udp data), '-s' (send sin/fin data), or nothing (send all data)
int main ( int argc, char** argv )
{

  bool send_tcp = false;
  bool send_udp = false;
  bool send_sf = false;

  //first get options
  if (argc < 2) {
    //default is to send all
    send_tcp = true;
    send_udp = true;
    send_sf = true;
  } else {
    for (int i = 1; i<argc; i++) {
      if (argv[i][0] == 't' || (argv[i][0] != '\0' && argv[i][1] == 't')) {
	send_tcp = true;
      } else if (argv[i][0] == 'u' || (argv[i][0] != '\0' && argv[i][1] == 'u')) {
	send_udp = true;
      } else if (argv[i][0] == 's' || (argv[i][0] != '\0' && argv[i][1] == 's')) {
	send_sf = true;
      }
    }
  }

  cout << "loading packets data from files..." << endl;

  //load all packets from file once
  if (!loaded) {

    char read[500];
    bool start = true;
    int count = 0;
    read[0] = '\0';

    if (send_tcp) {

    cout << "loading tcp1 data files..." << endl;

    ifstream in("./tcp1.txt");
    
    if(!in) {
      cout << "file not found: tcp1.txt" << endl;
      return 2;
    }
    
    while(!in.eof() && (start || /*count < TCP_COUNT ||*/ strlen(read) > 0)) {
      start = false;

      //while eof hasnt occured
      in.getline(read, 500);

      //cout << read << endl;
      if (strlen(read) > 0) {

      char delims[] = ". \t";
      char *result = NULL;
      result = strtok( read, delims );
      if ( result != NULL ) {
	tcps[count].a_time.tv_sec = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	tcps[count].a_time.tv_usec = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	tcps[count].from_ip = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	tcps[count].to_ip = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	tcps[count].s_port = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	tcps[count].d_port = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	tcps[count].num_bytes = atoi(result);
      }

      if ((count+1) % 1000 == 0) cout << "*";
      if ((count+1) % 150000 == 0) cout << endl;
      
      count++;      
      }
    }
    in.close();
    cout << "tcp1 file loaded, with " << count << " packets" << endl;

    }

    if (send_udp) {

    cout << "loading udp data files..." << endl;

    ifstream in2("./udp1.txt");
    
    if(!in2) {
      cout << "file not found: udp1.txt" << endl;
      return 2;
    }
    
    count = 0;
    start = true;
    read[0] = '\0';
    while(!in2.eof() && (start || /*count < UDP_COUNT ||*/ strlen(read) > 0)) {
      start = false;

      //while eof hasnt occured
      in2.getline(read, 500);

      //cout << read << endl;

      if (strlen(read) > 0) {

      char delims[] = ". \t";
      char *result = NULL;
      result = strtok( read, delims );
      if ( result != NULL ) {
	udps[count].a_time.tv_sec = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	udps[count].a_time.tv_usec = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	udps[count].from_ip = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	udps[count].to_ip = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	udps[count].s_port = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	udps[count].d_port = atoi(result);
      }

      if ((count+1) % 1000 == 0) cout << "*";
      if ((count+1) % 150000 == 0) cout << endl;      

      count++;      
      }
    }
    in2.close();
    cout << "udp file loaded, with " << count << " packets" << endl;

    }

    if (send_sf) {

    cout << "loading sf data files..." << endl;

    ifstream in3("./sf1.txt");
    
    if(!in3) {
      cout << "file not found: sf1.txt" << endl;
      return 2;
    }

    count = 0;
    start = true;
    read[0] = '\0';
    while(!in3.eof() && (start || /*count < SF_COUNT ||*/ strlen(read) > 0)) {
      start = false;

      //while eof hasnt occured
      in3.getline(read, 500);

      //cout << read << endl;

      if (strlen(read) > 0) {

      char delims[] = ". \t";
      char *result = NULL;
      result = strtok( read, delims );
      if ( result != NULL ) {
	sfs[count].a_time.tv_sec = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	sfs[count].a_time.tv_usec = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	sfs[count].from_ip = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	sfs[count].to_ip = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	sfs[count].s_port = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	sfs[count].d_port = atoi(result);
      }

      result = strtok( NULL, delims );
      if ( result != NULL ) {
	stringPad(result, sfs[count].flags, 3);
      }

      //sfs[count].print();

      if ((count+1) % 1000 == 0) cout << "*";
      if ((count+1) % 150000 == 0) cout << endl;

      count++;      
      }
    }
    in3.close();
    cout << "sf file loaded, with " << count << " packets" << endl;

    }

    loaded = true;
  }

  cout << "data files loaded." << endl;

  struct hostent *h_tcp;
  h_tcp = gethostbyname(SERVER);
  if(h_tcp==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_udp;
  h_udp = gethostbyname(SERVER);
  if(h_udp==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_sf;
  h_sf = gethostbyname(SERVER);
  if(h_sf==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  sockaddr_in m_addr_tcp, l_addr_tcp, m_addr_udp, l_addr_udp, m_addr_sf, l_addr_sf;
  int rc_tcp, rc_udp, rc_sf;
  int m_sock_tcp, m_sock_udp, m_sock_sf;
  int on_tcp, on_udp, on_sf;

  if(send_tcp) {
    //prepare connection for tcp data
    //reuse the connection forever for now
    memset ( &m_addr_tcp, 0, sizeof ( m_addr_tcp ) );
    memset ( &l_addr_tcp, 0, sizeof ( l_addr_tcp ) );
    
    m_sock_tcp = socket ( AF_INET, SOCK_STREAM, 0 );
    if (m_sock_tcp < 0) {
      cout << "error creating socket" << endl;
      return -1;
    }
    
    on_tcp = 1;
    if ( setsockopt ( m_sock_tcp, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp, sizeof ( on_tcp ) ) < 0) {
      cout << "error setting socket options" << endl;
      return -1;
    }
    
    // local bind any port number
    l_addr_tcp.sin_family = AF_INET;
    l_addr_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
    l_addr_tcp.sin_port = htons((unsigned short)0);
    
    //remote server
    m_addr_tcp.sin_family = h_tcp->h_addrtype;
    memcpy((char *) &m_addr_tcp.sin_addr.s_addr, h_tcp->h_addr_list[0], h_tcp->h_length);
    m_addr_tcp.sin_port = htons ( (unsigned short) TCP_PORT );
    
    rc_tcp = bind ( m_sock_tcp, ( struct sockaddr * ) &l_addr_tcp, sizeof ( l_addr_tcp ) );
    
    if ( rc_tcp < 0 ) {
      cout << "error binding local port for tcp data" << endl;
      return -1;
    }
    
    //connect to server, check if get connection, if not, sleep for a while before reconnect
    while ((rc_tcp = connect ( m_sock_tcp, ( struct sockaddr * ) &m_addr_tcp, sizeof ( m_addr_tcp ) )) < 0) {
      if (debug) cout << "can not get tcp connection, sleep and retry." << endl;
      sleep(1);
      continue;
    }
  } 

  if(send_udp) {
    //prepare connection for udp data
    //reuse the connection forever for now
    memset ( &m_addr_udp, 0, sizeof ( m_addr_udp ) );
    memset ( &l_addr_udp, 0, sizeof ( l_addr_udp ) );
    
    m_sock_udp = socket ( AF_INET, SOCK_STREAM, 0 );
    if (m_sock_udp < 0) {
      cout << "error creating socket" << endl;
      return -1;
    }
    
    on_udp = 1;
    if ( setsockopt ( m_sock_udp, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_udp, sizeof ( on_udp ) ) < 0) {
      cout << "error setting socket options" << endl;
      return -1;
    }
    
    // local bind any port number
    l_addr_udp.sin_family = AF_INET;
    l_addr_udp.sin_addr.s_addr = htonl(INADDR_ANY);
    l_addr_udp.sin_port = htons((unsigned short)0);
    
    //remote server
    m_addr_udp.sin_family = h_udp->h_addrtype;
    memcpy((char *) &m_addr_udp.sin_addr.s_addr, h_udp->h_addr_list[0], h_udp->h_length);
    m_addr_udp.sin_port = htons ( (unsigned short) UDP_PORT );
    
    rc_udp = bind ( m_sock_udp, ( struct sockaddr * ) &l_addr_udp, sizeof ( l_addr_udp ) );
    
    if ( rc_udp < 0 ) {
      cout << "error binding local port for udp data" << endl;
      return -1;
    }
    
    //connect to server, check if get connection, if not, sleep for a while before reconnect
    while ((rc_udp = connect ( m_sock_udp, ( struct sockaddr * ) &m_addr_udp, sizeof ( m_addr_udp ) )) < 0) {
      if (debug) cout << "can not get udp connection, sleep and retry." << endl;
      sleep(1);
      continue;
    }
  } 

 if(send_sf) {
    //prepare connection for sf data
    //reuse the connection forever for now
    memset ( &m_addr_sf, 0, sizeof ( m_addr_sf ) );
    memset ( &l_addr_sf, 0, sizeof ( l_addr_sf ) );
    
    m_sock_sf = socket ( AF_INET, SOCK_STREAM, 0 );
    if (m_sock_sf < 0) {
      cout << "error creating socket" << endl;
      return -1;
    }
    
    on_sf = 1;
    if ( setsockopt ( m_sock_sf, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_sf, sizeof ( on_sf ) ) < 0) {
      cout << "error setting socket options" << endl;
      return -1;
    }
    
    // local bind any port number
    l_addr_sf.sin_family = AF_INET;
    l_addr_sf.sin_addr.s_addr = htonl(INADDR_ANY);
    l_addr_sf.sin_port = htons((unsigned short)0);
    
    //remote server
    m_addr_sf.sin_family = h_sf->h_addrtype;
    memcpy((char *) &m_addr_sf.sin_addr.s_addr, h_sf->h_addr_list[0], h_sf->h_length);
    m_addr_sf.sin_port = htons ( (unsigned short) SF_PORT );
    
    rc_sf = bind ( m_sock_sf, ( struct sockaddr * ) &l_addr_sf, sizeof ( l_addr_sf ) );
    
    if ( rc_sf < 0 ) {
      cout << "error binding local port for sf data" << endl;
      return -1;
    }
    
    //connect to server, check if get connection, if not, sleep for a while before reconnect
    while ((rc_sf = connect ( m_sock_sf, ( struct sockaddr * ) &m_addr_sf, sizeof ( m_addr_sf ) )) < 0) {
      if (debug) cout << "can not get sf connection, sleep and retry." << endl;
      sleep(1);
      continue;
    }
  }



  cout << "start sending packets data" << endl;
  restart = true;

  while (1) {
  
    struct timeval tv;
    struct timezone tz;
  
    gettimeofday(&tv, &tz);
    
    if (restart) {
      if (debug) cout << "start time: " << start_tv.tv_sec << "." << start_tv.tv_usec << endl;
      gettimeofday(&start_tv, &tz);
      next_tcp_count = 0;
      next_udp_count = 0;
      next_sf_count = 0;
      restart = false;
    }

    //restart if more than 1 hour has elapsed or any id exceed its limit
    if ((tv.tv_sec - start_tv.tv_sec) > 3600 || next_tcp_count >= TCP_COUNT || next_udp_count >= UDP_COUNT || next_sf_count >= SF_COUNT  ) {
      restart = true;
    }
  
    char buf[500];
    char temp[100];
    char ts[60];
    
    //take all packets that should be sent before this time, and send them over
    //emit warning message if the delay is more than 3 sec for any packet
    gettimeofday(&tv, &tz);
    while (send_tcp && 
	   (SLOW_DOWN*tcps[next_tcp_count].a_time.tv_sec < (tv.tv_sec - start_tv.tv_sec)
	    || (SLOW_DOWN*tcps[next_tcp_count].a_time.tv_sec == (tv.tv_sec - start_tv.tv_sec) && (SLOW_DOWN*tcps[next_tcp_count].a_time.tv_usec < (tv.tv_usec - start_tv.tv_usec))))) {
      if (SLOW_DOWN*tcps[next_tcp_count].a_time.tv_sec - (tv.tv_sec - start_tv.tv_sec) > 3) {
	if (debug) {
	  cout << "next sec is: " << SLOW_DOWN*tcps[next_tcp_count].a_time.tv_sec << ", diff in sec between now and start is " << (tv.tv_sec - start_tv.tv_sec) << endl;
	  cout << "next usec is: " << SLOW_DOWN*tcps[next_tcp_count].a_time.tv_usec << ", diff in usec between now and start is " << (tv.tv_usec - start_tv.tv_usec) << endl;
	  cout << "sent tcp1 data late for more than 3 sec, delay is: " << SLOW_DOWN*tcps[next_tcp_count].a_time.tv_sec - (tv.tv_sec - start_tv.tv_sec)  << " sec."<< endl;
	}
      }

      int id = next_tcp_count++;
            
      sprintf(buf, "%d,",  tcps[id].from_ip);
      //strcat(buf, temp);
      
      sprintf(temp, "%d,",  tcps[id].to_ip);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  tcps[id].s_port);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  tcps[id].d_port);
      strcat(buf, temp);
      
      sprintf(temp, "%d",  tcps[id].num_bytes);
      strcat(buf, temp);
      
      struct tm* tmp = localtime(&tv.tv_sec);
      strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
      sprintf(temp, ".%d\n", tv.tv_usec);
      strcat(ts, temp);
      strcat(buf, ts);

      rc_tcp = send(m_sock_tcp, buf, strlen(buf), 0);
      if(rc_tcp<0) {
	if (debug) cout << "error sending tcp data, retry later" << endl;
	sleep(1);
	break;
      }  else {
	if (debug) {
	  //cout << "sent tcp1 #" << id << ", values ";
	  //tcps[id].print();
	  cout << "sent tcp1 #" << id << ", buffer: " << buf << endl;
	}
      }
    }
    cout.flush();

    gettimeofday(&tv, &tz);
    while (send_udp && 
	   ((start_tv.tv_sec + udps[next_udp_count].a_time.tv_sec) > tv.tv_sec 
	    || ((start_tv.tv_sec + udps[next_udp_count].a_time.tv_sec) == tv.tv_sec && (start_tv.tv_usec + udps[next_udp_count].a_time.tv_usec) > tv.tv_usec))) {
      if ((start_tv.tv_sec + udps[next_udp_count].a_time.tv_sec) - tv.tv_sec > 3) {
	if (debug) cout << "sent udp data late for more than 3 sec " << ((start_tv.tv_sec + udps[next_udp_count].a_time.tv_sec) - tv.tv_sec) << endl;
      }

      int id = next_udp_count++;
      
      struct tm* tmp = localtime(&tv.tv_sec);
      strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
      sprintf(temp, ".%d", tv.tv_usec);
      strcat(ts, temp);
      sprintf(buf, "%s,",  ts);
      
      sprintf(temp, "%d,",  udps[id].from_ip);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  udps[id].to_ip);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  udps[id].s_port);
      strcat(buf, temp);
      
      sprintf(temp, "%d\n",  udps[id].d_port);
      strcat(buf, temp);
      
      rc_udp = send(m_sock_udp, buf, strlen(buf), 0);
      if(rc_udp<0) {
	if (debug) cout << "error sending udp data, retry later" << endl;
	sleep(1);
	break;
      }  else {
	if (debug) cout << "sent udp: " << buf << endl;
      }
    }

    gettimeofday(&tv, &tz);
    while (send_sf && 
	   ((start_tv.tv_sec + sfs[next_sf_count].a_time.tv_sec) > tv.tv_sec 
	    || ((start_tv.tv_sec + sfs[next_sf_count].a_time.tv_sec) == tv.tv_sec && (start_tv.tv_usec + sfs[next_sf_count].a_time.tv_usec) > tv.tv_usec))) {
      if ((start_tv.tv_sec + sfs[next_sf_count].a_time.tv_sec) - tv.tv_sec > 3) {
	if (debug) cout << "sent sf data late for more than 3 sec " << ((start_tv.tv_sec + sfs[next_sf_count].a_time.tv_sec) - tv.tv_sec) << endl;
      }

      int id = next_sf_count++;
      
      struct tm* tmp = localtime(&tv.tv_sec);
      strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
      sprintf(temp, ".%d", tv.tv_usec);
      strcat(ts, temp);
      sprintf(buf, "%s,",  ts);
      
      sprintf(temp, "%d,",  sfs[id].from_ip);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  sfs[id].to_ip);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  sfs[id].s_port);
      strcat(buf, temp);
      
      sprintf(temp, "%d,",  sfs[id].d_port);
      strcat(buf, temp);
      
      sprintf(temp, "%s\n",  sfs[id].flags);
      strcat(buf, temp);
      
      rc_sf = send(m_sock_sf, buf, strlen(buf), 0);
      if(rc_sf<0) {
	if (debug) cout << "error sending sf data, retry later" << endl;
	sleep(1);
	break;
      }  else {
	if (debug) cout << "sent sf: " << buf << endl;
      }
    }
  
  }
  
  return 0;                     // Got data
}
