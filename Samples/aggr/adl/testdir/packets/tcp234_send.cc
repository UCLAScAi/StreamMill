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
//#define TCP1_PORT 5600
#define TCP2_PORT 5610
#define TCP3_PORT 5620
#define TCP4_PORT 5630
#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

#define TCP2_COUNT 259553
#define TCP3_COUNT 230926
#define TCP4_COUNT 311143

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

static tcp_t tcp2[TCP2_COUNT];
static tcp_t tcp3[TCP3_COUNT];
static tcp_t tcp4[TCP4_COUNT];

static bool loaded = false;

static struct timeval start_tv;

static int next_tcp2_count = 0;
static int next_tcp3_count = 0;
static int next_tcp4_count = 0;

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

//invoke with switches '2' (send tcp2 data), '3' (send tcp3 data), '4' (send tcp4 data), or nothing (send all data)
int main ( int argc, char** argv )
{

  bool send_tcp2 = false;
  bool send_tcp3 = false;
  bool send_tcp4 = false;

  //first get options
  if (argc < 2) {
    //default is to send all
    send_tcp2 = true;
    send_tcp3 = true;
    send_tcp4 = true;
  } else {
    for (int i = 1; i<argc; i++) {
      if (argv[i][0] == '2' || (argv[i][0] != '\0' && argv[i][1] == '2')) {
	send_tcp2 = true;
      } else if (argv[i][0] == '3' || (argv[i][0] != '\0' && argv[i][1] == '3')) {
	send_tcp3 = true;
      } else if (argv[i][0] == '4' || (argv[i][0] != '\0' && argv[i][1] == '4')) {
	send_tcp4 = true;
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


    start = true;
    count = 0;
    read[0] = '\0';
    if (send_tcp4) {

      cout << "loading tcp1 data files..." << endl;

      ifstream in("./tcp4.txt");
    
      if(!in) {
	cout << "file not found: tcp4.txt" << endl;
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
	    tcp4[count].a_time.tv_sec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp4[count].a_time.tv_usec = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp4[count].from_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp4[count].to_ip = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp4[count].s_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp4[count].d_port = atoi(result);
	  }
	  
	  result = strtok( NULL, delims );
	  if ( result != NULL ) {
	    tcp4[count].num_bytes = atoi(result);
	  }
	  
	  if ((count+1) % 1000 == 0) cout << "*";
	  if ((count+1) % 150000 == 0) cout << endl;
	  
	  count++;      
	}
      }
      cout << endl;
      in.close();
      cout << "tcp4 file loaded, with " << count << " packets" << endl;
      
    }

    loaded = true;
  }

  cout << "data files loaded." << endl;

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

  sockaddr_in m_addr_tcp4, l_addr_tcp4, m_addr_tcp2, l_addr_tcp2, m_addr_tcp3, l_addr_tcp3;
  int rc_tcp4, rc_tcp2, rc_tcp3;
  int m_sock_tcp4, m_sock_tcp2, m_sock_tcp3;
  int on_tcp4, on_tcp2, on_tcp3;

  if(send_tcp4) {
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
  double total_delay_in_send = 0;
  double max_delay_in_send = 0;

  cout.setf(ios_base::fixed);
  while (1) {
  
    struct timeval tv;
    struct timezone tz;
  
    gettimeofday(&tv, &tz);
    
    if (restart) {
      gettimeofday(&start_tv, &tz);
      if (debug) cout << "start time: " << start_tv.tv_sec << "." << start_tv.tv_usec << endl;
      next_tcp4_count = 0;
      next_tcp2_count = 0;
      next_tcp3_count = 0;
      cnt = 0;
      restart = false;
    }

    //restart if more than 1 hour has elapsed or any id exceed its limit
    if ((tv.tv_sec - start_tv.tv_sec) > 3600 || next_tcp4_count >= TCP4_COUNT || next_tcp2_count >= TCP2_COUNT || next_tcp3_count >= TCP3_COUNT) {
      restart = true;
    }
  
    char buf[500];
    char temp[100];
    char ts[60];
        
    //do a "mergesort", take the earliest of the packets from the three, and if time has exceeded when it is supposed to be sent, send it.    
    int minStream = 1;
    struct timeval minTime;
    tcp_t* p;
    minTime.tv_sec = tcp4[next_tcp4_count].a_time.tv_sec;
    minTime.tv_usec = tcp4[next_tcp4_count].a_time.tv_usec;
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
      p = &(tcp4[next_tcp4_count]);
      id = next_tcp4_count;
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
    //long tv_sec1 = (long)diff1;
    //long tv_usec1 = (long) ((diff1 - tv_sec1)*1000000);

    //the packet supposed time difference (with slowdown factor considered)
    //long tv_sec_pac = (long) ((p->a_time.tv_usec*SLOW_DOWN)/1000000);
    //long tv_usec_pac = p->a_time.tv_usec*SLOW_DOWN - (tv_sec_pac/SLOW_DOWN)*1000000;
    //tv_sec_pac += p->a_time.tv_sec*SLOW_DOWN;
    //double diff2 = (double)tv_sec_pac + (double)tv_usec_pac/1000000;
    double diff2 = (p->a_time.tv_sec+(double)p->a_time.tv_usec/1000000)*SLOW_DOWN;
    double delay = diff1 - diff2;
    
    //if ( tv_sec_pac < tv_sec1 || (tv_sec_pac == tv_sec1 && tv_usec_pac < tv_usec1)) {
    if (delay >= 0) {
      //if ((tv_sec_pac - tv_sec1) > 3) {
      //if (debug) {
      //  cout << "next sec is: " << tv_sec_pac << ", diff in sec between now and start is " << tv_sec1 << endl;
      //  cout << "next usec is: " << tv_usec_pac << ", diff in usec between now and start is " << tv_usec1 << endl;
      //  cout << "sent tcp" << minStream << " data late for more than 3 sec: " << tv_sec_pac - tv_sec1 << " sec."<< endl;
      //}
      //}

      //if (debug) {
	//cout << "next tcp4 #" << next_tcp4_count << ", time " << tcp4[next_tcp4_count].a_time.tv_sec << "." << tcp4[next_tcp4_count].a_time.tv_usec << endl;
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
	rc_tcp4 = send(m_sock_tcp4, buf, strlen(buf), 0);
	if(rc_tcp4<0) {
	  if (debug) cout << "error sending tcp4 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  //if (debug) {
	    //cout << "sent tcp4 #" << id << ", values ";
	    //p->print();
	    //cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    //cout << "sent tcp4, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 4 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  //}
	  next_tcp4_count++;
	}	
      } else if (minStream == 2) {
	rc_tcp2 = send(m_sock_tcp2, buf, strlen(buf), 0);
	if(rc_tcp2<0) {
	  if (debug) cout << "error sending tcp2 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  //if (debug) {
	    //cout << "sent tcp2 #" << id << ", values ";
	    //p->print();
	    //cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    //cout << "sent tcp2, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 2 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  //}
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
	  //if (debug) {
	    //cout << "sent tcp3 #" << id << ", values ";
	    //p->print();
	    //cout << "start time " << start_tv.tv_sec << "." << start_tv.tv_usec << ", current time " << tv.tv_sec << "." << tv.tv_usec << endl;
	    //cout << "sent tcp3, buffer: " << buf << endl;
	    //cout << "packet time diff after slowdown " << tv_sec_pac << "." << tv_usec_pac << ", sent time diff " << tv_sec1 << "." << tv_usec1 << endl;
	    //cout << cnt << "\t" << 3 << "\t" << tv_sec_pac << "." << tv_usec_pac << "\t" << tv_sec1 << "." << tv_usec1 << endl;
	  //}
	  next_tcp3_count++;
	}	
      }

      //cout << delay << " ";
      if (cnt%100 == 0) {
	cout << cnt << "\t" << total_delay_in_send/((double)100*SLOW_DOWN) << "\t" << max_delay_in_send/(double)SLOW_DOWN << endl;
	cout.flush();	
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
