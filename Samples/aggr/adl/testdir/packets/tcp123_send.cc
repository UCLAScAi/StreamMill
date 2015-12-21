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
#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

#define TCP1_COUNT 310408
#define TCP2_COUNT 334027
#define TCP3_COUNT 514607

//#define TCP1_COUNT 2153462
//#define TCP2_COUNT 2661931
//#define TCP3_COUNT 2873589

#define SLOW_DOWN 1
//#define SLOW_DOWN_2 10
//#define SLOW_DOWN_3 10

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

static tcp_t tcp1[TCP1_COUNT];
static tcp_t tcp2[TCP2_COUNT];
static tcp_t tcp3[TCP3_COUNT];

static bool loaded = false;

static struct timeval start_tv;

static int next_tcp1_count = 0;
static int next_tcp2_count = 0;
static int next_tcp3_count = 0;

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

//invoke with switches '1' (send tcp1 data), '2' (send tcp2 data), '3' (send tcp3 data), or nothing (send all data)
int main ( int argc, char** argv )
{

  bool send_tcp1 = false;
  bool send_tcp2 = false;
  bool send_tcp3 = false;

  //first get options
  if (argc < 2) {
    //default is to send all
    send_tcp1 = true;
    send_tcp2 = true;
    send_tcp3 = true;
  } else {
    for (int i = 1; i<argc; i++) {
      if (argv[i][0] == '1' || (argv[i][0] != '\0' && argv[i][1] == '1')) {
	send_tcp1 = true;
      } else if (argv[i][0] == '2' || (argv[i][0] != '\0' && argv[i][1] == '2')) {
	send_tcp2 = true;
      } else if (argv[i][0] == '3' || (argv[i][0] != '\0' && argv[i][1] == '3')) {
	send_tcp3 = true;
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

    if (send_tcp1) {

      cout << "loading tcp1 data files..." << endl;

      ifstream in("./tcp1.txt");
    
      if(!in) {
	cout << "file not found: tcp1.txt" << endl;
	return 2;
      }
    
      while(!in.eof() && (start || /*count < TCP1_COUNT ||*/ strlen(read) > 0)) {
	start = false;

	//while eof hasnt occured
	in.getline(read, 500);
	
	//cout << read << endl;
	if (strlen(read) > 0) {

	  char delims[] = ". \t";
	  char *result = NULL;
	  result = strtok( read, delims );
	  if ( result != NULL ) {
	    tcp1[count].a_time.tv_sec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp1[count].a_time.tv_usec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp1[count].from_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp1[count].to_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp1[count].s_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp1[count].d_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp1[count].num_bytes = atoi(result);
	  }
	  
	  if ((count+1) % 1000 == 0) cout << "*";
	  if ((count+1) % 150000 == 0) cout << endl;
	  
	  count++;      
	}
      }
      cout << endl;
      in.close();
      cout << "tcp1 file loaded, with " << count << " packets" << endl;
      
    }

    start = true;
    count = 0;
    read[0] = '\0';
    if (send_tcp2) {

      cout << "loading tcp2 data files..." << endl;

      ifstream in("./tcp2.txt");
    
      if(!in) {
	cout << "file not found: tcp2.txt" << endl;
	return 2;
      }
    
      while(!in.eof() && (start || /*count < TCP2_COUNT ||*/ strlen(read) > 0)) {
	start = false;

	//while eof hasnt occured
	in.getline(read, 500);
	
	//cout << read << endl;
	if (strlen(read) > 0) {

	  char delims[] = ". \t";
	  char *result = NULL;
	  result = strtok( read, delims );
	  if ( result != NULL ) {
	    tcp2[count].a_time.tv_sec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp2[count].a_time.tv_usec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp2[count].from_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp2[count].to_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp2[count].s_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp2[count].d_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp2[count].num_bytes = atoi(result);
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

    start = true;
    count = 0;
    read[0] = '\0';
    if (send_tcp3) {

      cout << "loading tcp3 data files..." << endl;

      ifstream in("./tcp3.txt");
    
      if(!in) {
	cout << "file not found: tcp3.txt" << endl;
	return 2;
      }
    
      while(!in.eof() && (start || /*count < TCP3_COUNT ||*/ strlen(read) > 0)) {
	start = false;

	//while eof hasnt occured
	in.getline(read, 500);
	
	//cout << read << endl;
	if (strlen(read) > 0) {

	  char delims[] = ". \t";
	  char *result = NULL;
	  result = strtok( read, delims );
	  if ( result != NULL ) {
	    tcp3[count].a_time.tv_sec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp3[count].a_time.tv_usec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp3[count].from_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp3[count].to_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp3[count].s_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp3[count].d_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp3[count].num_bytes = atoi(result);
	  }
	  
	  if ((count+1) % 1000 == 0) cout << "*";
	  if ((count+1) % 150000 == 0) cout << endl;
	  
	  count++;      
	}
      }
      cout << endl;
      in.close();
      cout << "tcp3 file loaded, with " << count << " packets" << endl;
      
    }

    loaded = true;
  }

  cout << "data files loaded." << endl;

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

  sockaddr_in m_addr_tcp1, l_addr_tcp1, m_addr_tcp2, l_addr_tcp2, m_addr_tcp3, l_addr_tcp3;
  int rc_tcp1, rc_tcp2, rc_tcp3;
  int m_sock_tcp1, m_sock_tcp2, m_sock_tcp3;
  int on_tcp1, on_tcp2, on_tcp3;

  if(send_tcp1) {
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
  } 

  if(send_tcp2) {
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
  } 

  if(send_tcp3) {
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
  } 

  cout << "start sending packets data" << endl;
  restart = true;

  long cnt = 0;

  while (1) {
  
    struct timeval tv;
    struct timezone tz;
  
    gettimeofday(&tv, &tz);
    
    if (restart) {
      gettimeofday(&start_tv, &tz);
      if (debug) cout << "start time: " << start_tv.tv_sec << "." << start_tv.tv_usec << endl;
      next_tcp1_count = 0;
      next_tcp2_count = 0;
      next_tcp3_count = 0;
      cnt = 0;
      restart = false;
    }

    //restart if more than 1 hour has elapsed or any id exceed its limit
    if ((tv.tv_sec - start_tv.tv_sec) > 3600 || next_tcp1_count >= TCP1_COUNT || next_tcp2_count >= TCP2_COUNT || next_tcp3_count >= TCP3_COUNT) {
      restart = true;
    }
  
    char buf[500];
    char temp[100];
    char ts[60];
        
    //do a "mergesort", take the earliest of the packets from the three, and if time has exceeded when it is supposed to be sent, send it.    
    int minStream = 1;
    struct timeval minTime;
    tcp_t* p;
    minTime.tv_sec = tcp1[next_tcp1_count].a_time.tv_sec;
    minTime.tv_usec = tcp1[next_tcp1_count].a_time.tv_usec;
    if ((tcp2[next_tcp2_count].a_time.tv_sec < minTime.tv_sec) || (tcp2[next_tcp2_count].a_time.tv_sec == minTime.tv_sec && tcp2[next_tcp2_count].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 2;
      minTime.tv_sec = tcp2[next_tcp2_count].a_time.tv_sec;
      minTime.tv_usec = tcp2[next_tcp2_count].a_time.tv_usec;      
    } 
    if ((tcp3[next_tcp3_count].a_time.tv_sec < minTime.tv_sec) || (tcp3[next_tcp3_count].a_time.tv_sec == minTime.tv_sec && tcp3[next_tcp3_count].a_time.tv_usec < minTime.tv_usec)) {
      minStream = 3;
      minTime.tv_sec = tcp3[next_tcp3_count].a_time.tv_sec;
      minTime.tv_usec = tcp3[next_tcp3_count].a_time.tv_usec;      
    } 

    int id;
    if (minStream == 1) {
      p = &(tcp1[next_tcp1_count]);
      id = next_tcp1_count;
    } else if (minStream == 2) {
      p = &(tcp2[next_tcp2_count]);
      id = next_tcp2_count;
    } else {
      p = &(tcp3[next_tcp3_count]);
      id = next_tcp3_count;
    } 

    gettimeofday(&tv, &tz);

    //the actually time difference since start
    double diff1 = timeval_subtract(tv, start_tv);
    long tv_sec1 = (long)diff1;
    long tv_usec1 = (long) ((diff1 - tv_sec1)*1000000);

    //the packet supposed time difference (with slowdown factor considered)
    long tv_sec_pac = (long) ((p->a_time.tv_usec*SLOW_DOWN)/1000000);
    long tv_usec_pac = p->a_time.tv_usec*SLOW_DOWN - (tv_sec_pac/SLOW_DOWN)*1000000;
    tv_sec_pac += p->a_time.tv_sec*SLOW_DOWN;

    if ( tv_sec_pac < tv_sec1 || (tv_sec_pac == tv_sec1 && tv_usec_pac < tv_usec1)) {
      //if ((tv_sec_pac - tv_sec1) > 3) {
      //if (debug) {
      //  cout << "next sec is: " << tv_sec_pac << ", diff in sec between now and start is " << tv_sec1 << endl;
      //  cout << "next usec is: " << tv_usec_pac << ", diff in usec between now and start is " << tv_usec1 << endl;
      //  cout << "sent tcp" << minStream << " data late for more than 3 sec: " << tv_sec_pac - tv_sec1 << " sec."<< endl;
      //}
      //}

      //if (debug) {
	//cout << "next tcp1 #" << next_tcp1_count << ", time " << tcp1[next_tcp1_count].a_time.tv_sec << "." << tcp1[next_tcp1_count].a_time.tv_usec << endl;
	//cout << "next tcp2 #" << next_tcp2_count << ", time " << tcp2[next_tcp2_count].a_time.tv_sec << "." << tcp2[next_tcp2_count].a_time.tv_usec << endl;
	//cout << "next tcp3 #" << next_tcp3_count << ", time " << tcp3[next_tcp3_count].a_time.tv_sec << "." << tcp3[next_tcp3_count].a_time.tv_usec << endl;
	//cout << "min time is from tcp" << minStream << ", count " << id << ", values " << endl;
	//p->print();
      //}

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
	  if (debug) {
	    //cout << "sent tcp1 #" << id << ", values ";
	    //p->print();
	    //cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    //cout << "sent tcp1, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    cout << cnt << "\t" << 1 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  next_tcp1_count++;
	}	
      } else if (minStream == 2) {
	rc_tcp2 = send(m_sock_tcp2, buf, strlen(buf), 0);
	if(rc_tcp2<0) {
	  if (debug) cout << "error sending tcp2 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  if (debug) {
	    //cout << "sent tcp2 #" << id << ", values ";
	    //p->print();
	    //cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    //cout << "sent tcp2, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  next_tcp2_count++;
	}	
      } else {
	rc_tcp3 = send(m_sock_tcp3, buf, strlen(buf), 0);
	if(rc_tcp3<0) {
	  if (debug) cout << "error sending tcp3 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  if (debug) {
	    //cout << "sent tcp3 #" << id << ", values ";
	    //p->print();
	    //cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    //cout << "sent tcp3, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    cout << cnt << "\t" << 3 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  }
	  next_tcp3_count++;
	}	
      } 
    }
    cout.flush();

  } //end while(1)
  
  return 0;                     // Got data
}
