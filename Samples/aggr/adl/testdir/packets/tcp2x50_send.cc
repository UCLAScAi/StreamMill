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
#define TCP10_PORT 5690

#define TCP11_PORT 5700
#define TCP12_PORT 5710
#define TCP13_PORT 5720
#define TCP14_PORT 5730
#define TCP15_PORT 5740
#define TCP16_PORT 5750
#define TCP17_PORT 5760
#define TCP18_PORT 5770
#define TCP19_PORT 5780
#define TCP20_PORT 5790

#define TCP21_PORT 5800
#define TCP22_PORT 5810
#define TCP23_PORT 5820
#define TCP24_PORT 5830
#define TCP25_PORT 5840
#define TCP26_PORT 5850
#define TCP27_PORT 5860
#define TCP28_PORT 5870
#define TCP29_PORT 5880
#define TCP30_PORT 5890

#define TCP31_PORT 5900
#define TCP32_PORT 5910
#define TCP33_PORT 5920
#define TCP34_PORT 5930
#define TCP35_PORT 5940
#define TCP36_PORT 5950
#define TCP37_PORT 5960
#define TCP38_PORT 5970
#define TCP39_PORT 5980
#define TCP40_PORT 5990

#define TCP41_PORT 6000
#define TCP42_PORT 6010
#define TCP43_PORT 6020
#define TCP44_PORT 6030
#define TCP45_PORT 6040
#define TCP46_PORT 6050
#define TCP47_PORT 6060
#define TCP48_PORT 6070
#define TCP49_PORT 6080
#define TCP50_PORT 6090

#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

//everything is from the same file: tcp2.
#define ALL_COUNT 2661931

#define SLOW_DOWN 175

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
//tcp1 is also tcp11, tcp21, tcp31, tcp41
static int tcp1_start_sec = 1000;
static int tcp1_next = 613174;
static int tcp1_end = 780668;

//tcp2 is index 2116017 to 2263462 (time 2900 - 3100)
//tcp2 is also tcp12, tcp22, tcp32, tcp42
static int tcp2_start_sec = 2900;
static int tcp2_next = 2116017;
static int tcp2_end = 2263462;

//tcp3 is index 2341049 to 2523152 (time 3200 - 3400)
//tcp3 is also tcp13, tcp23, tcp33, tcp43
static int tcp3_start_sec = 3200;
static int tcp3_next = 2341049;
static int tcp3_end = 2523152;

//tcp4 is index 432406 to 576532 (time 750 - 950)
//tcp4 is also tcp14, tcp24, tcp34, tcp44
static int tcp4_start_sec = 750;
static int tcp4_next = 432406;
static int tcp4_end = 576531;

//tcp5 is index 780669 to 930551 (time 1200 - 1400)
//tcp5 is also tcp15, tcp25, tcp35, tcp45
static int tcp5_start_sec = 1200;
static int tcp5_next = 780669;
static int tcp5_end = 930551;

//tcp6 is index 1083670 to 1245891 (time 1600 - 1800)
//tcp6 is also tcp16, tcp26, tcp36, tcp46
static int tcp6_start_sec = 1600;
static int tcp6_next = 1083670;
static int tcp6_end = 1245891;

//tcp7 is index 1537739 to 1705001 (time 2200 - 2400)
//tcp7 is also tcp17, tcp27, tcp37, tcp47
static int tcp7_start_sec = 2200;
static int tcp7_next = 1537739;
static int tcp7_end = 1705001;

//tcp8 is index 1705002 to 1875813 (time 2400 - 2600)
//tcp8 is also tcp18, tcp28, tcp38, tcp48, tcp10, tcp20, tcp30, tcp40, tcp50
static int tcp8_start_sec = 2400;
static int tcp8_next = 1705002;
static int tcp8_end = 1875813;

//tcp9 is index 1875814 to 2042490 (time 2600 - 2800)
//tcp9 is also tcp19, tcp29, tcp39, tcp49
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

  struct hostent *h_tcp10;
  h_tcp10 = gethostbyname(SERVER);
  if(h_tcp10==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp11;
  h_tcp11 = gethostbyname(SERVER);
  if(h_tcp11==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp12;
  h_tcp12 = gethostbyname(SERVER);
  if(h_tcp12==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp13;
  h_tcp13 = gethostbyname(SERVER);
  if(h_tcp13==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp14;
  h_tcp14 = gethostbyname(SERVER);
  if(h_tcp14==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp15;
  h_tcp15 = gethostbyname(SERVER);
  if(h_tcp15==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp16;
  h_tcp16 = gethostbyname(SERVER);
  if(h_tcp16==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp17;
  h_tcp17 = gethostbyname(SERVER);
  if(h_tcp17==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp18;
  h_tcp18 = gethostbyname(SERVER);
  if(h_tcp18==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp19;
  h_tcp19 = gethostbyname(SERVER);
  if(h_tcp19==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp20;
  h_tcp20 = gethostbyname(SERVER);
  if(h_tcp20==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }
  struct hostent *h_tcp21;
  h_tcp21 = gethostbyname(SERVER);
  if(h_tcp21==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp22;
  h_tcp22 = gethostbyname(SERVER);
  if(h_tcp22==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp23;
  h_tcp23 = gethostbyname(SERVER);
  if(h_tcp23==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp24;
  h_tcp24 = gethostbyname(SERVER);
  if(h_tcp24==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp25;
  h_tcp25 = gethostbyname(SERVER);
  if(h_tcp25==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp26;
  h_tcp26 = gethostbyname(SERVER);
  if(h_tcp26==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp27;
  h_tcp27 = gethostbyname(SERVER);
  if(h_tcp27==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp28;
  h_tcp28 = gethostbyname(SERVER);
  if(h_tcp28==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp29;
  h_tcp29 = gethostbyname(SERVER);
  if(h_tcp29==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp30;
  h_tcp30 = gethostbyname(SERVER);
  if(h_tcp30==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }
  struct hostent *h_tcp31;
  h_tcp31 = gethostbyname(SERVER);
  if(h_tcp31==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp32;
  h_tcp32 = gethostbyname(SERVER);
  if(h_tcp32==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp33;
  h_tcp33 = gethostbyname(SERVER);
  if(h_tcp33==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp34;
  h_tcp34 = gethostbyname(SERVER);
  if(h_tcp34==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp35;
  h_tcp35 = gethostbyname(SERVER);
  if(h_tcp35==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp36;
  h_tcp36 = gethostbyname(SERVER);
  if(h_tcp36==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp37;
  h_tcp37 = gethostbyname(SERVER);
  if(h_tcp37==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp38;
  h_tcp38 = gethostbyname(SERVER);
  if(h_tcp38==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp39;
  h_tcp39 = gethostbyname(SERVER);
  if(h_tcp39==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp40;
  h_tcp40 = gethostbyname(SERVER);
  if(h_tcp40==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }
  struct hostent *h_tcp41;
  h_tcp41 = gethostbyname(SERVER);
  if(h_tcp41==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp42;
  h_tcp42 = gethostbyname(SERVER);
  if(h_tcp42==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp43;
  h_tcp43 = gethostbyname(SERVER);
  if(h_tcp43==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp44;
  h_tcp44 = gethostbyname(SERVER);
  if(h_tcp44==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp45;
  h_tcp45 = gethostbyname(SERVER);
  if(h_tcp45==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp46;
  h_tcp46 = gethostbyname(SERVER);
  if(h_tcp46==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp47;
  h_tcp47 = gethostbyname(SERVER);
  if(h_tcp47==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp48;
  h_tcp48 = gethostbyname(SERVER);
  if(h_tcp48==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp49;
  h_tcp49 = gethostbyname(SERVER);
  if(h_tcp49==NULL) {
    printf("unknown host '%s'\n", SERVER);
    return -1;
  }

  struct hostent *h_tcp50;
  h_tcp50 = gethostbyname(SERVER);
  if(h_tcp50==NULL) {
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

  sockaddr_in m_addr_tcp7, l_addr_tcp7, m_addr_tcp8, l_addr_tcp8, m_addr_tcp9, l_addr_tcp9, m_addr_tcp10, l_addr_tcp10;
  int rc_tcp7, rc_tcp8, rc_tcp9, rc_tcp10;
  int m_sock_tcp7, m_sock_tcp8, m_sock_tcp9, m_sock_tcp10;
  int on_tcp7, on_tcp8, on_tcp9, on_tcp10;

  sockaddr_in m_addr_tcp11, l_addr_tcp11, m_addr_tcp12, l_addr_tcp12, m_addr_tcp13, l_addr_tcp13;
  int rc_tcp11, rc_tcp12, rc_tcp13;
  int m_sock_tcp11, m_sock_tcp12, m_sock_tcp13;
  int on_tcp11, on_tcp12, on_tcp13;

  sockaddr_in m_addr_tcp14, l_addr_tcp14, m_addr_tcp15, l_addr_tcp15, m_addr_tcp16, l_addr_tcp16;
  int rc_tcp14, rc_tcp15, rc_tcp16;
  int m_sock_tcp14, m_sock_tcp15, m_sock_tcp16;
  int on_tcp14, on_tcp15, on_tcp16;

  sockaddr_in m_addr_tcp17, l_addr_tcp17, m_addr_tcp18, l_addr_tcp18, m_addr_tcp19, l_addr_tcp19, m_addr_tcp20, l_addr_tcp20;
  int rc_tcp17, rc_tcp18, rc_tcp19, rc_tcp20;
  int m_sock_tcp17, m_sock_tcp18, m_sock_tcp19, m_sock_tcp20;
  int on_tcp17, on_tcp18, on_tcp19, on_tcp20;

  sockaddr_in m_addr_tcp21, l_addr_tcp21, m_addr_tcp22, l_addr_tcp22, m_addr_tcp23, l_addr_tcp23;
  int rc_tcp21, rc_tcp22, rc_tcp23;
  int m_sock_tcp21, m_sock_tcp22, m_sock_tcp23;
  int on_tcp21, on_tcp22, on_tcp23;

  sockaddr_in m_addr_tcp24, l_addr_tcp24, m_addr_tcp25, l_addr_tcp25, m_addr_tcp26, l_addr_tcp26;
  int rc_tcp24, rc_tcp25, rc_tcp26;
  int m_sock_tcp24, m_sock_tcp25, m_sock_tcp26;
  int on_tcp24, on_tcp25, on_tcp26;

  sockaddr_in m_addr_tcp27, l_addr_tcp27, m_addr_tcp28, l_addr_tcp28, m_addr_tcp29, l_addr_tcp29, m_addr_tcp30, l_addr_tcp30;
  int rc_tcp27, rc_tcp28, rc_tcp29, rc_tcp30;
  int m_sock_tcp27, m_sock_tcp28, m_sock_tcp29, m_sock_tcp30;
  int on_tcp27, on_tcp28, on_tcp29, on_tcp30;

  sockaddr_in m_addr_tcp31, l_addr_tcp31, m_addr_tcp32, l_addr_tcp32, m_addr_tcp33, l_addr_tcp33;
  int rc_tcp31, rc_tcp32, rc_tcp33;
  int m_sock_tcp31, m_sock_tcp32, m_sock_tcp33;
  int on_tcp31, on_tcp32, on_tcp33;

  sockaddr_in m_addr_tcp34, l_addr_tcp34, m_addr_tcp35, l_addr_tcp35, m_addr_tcp36, l_addr_tcp36;
  int rc_tcp34, rc_tcp35, rc_tcp36;
  int m_sock_tcp34, m_sock_tcp35, m_sock_tcp36;
  int on_tcp34, on_tcp35, on_tcp36;

  sockaddr_in m_addr_tcp37, l_addr_tcp37, m_addr_tcp38, l_addr_tcp38, m_addr_tcp39, l_addr_tcp39, m_addr_tcp40, l_addr_tcp40;
  int rc_tcp37, rc_tcp38, rc_tcp39, rc_tcp40;
  int m_sock_tcp37, m_sock_tcp38, m_sock_tcp39, m_sock_tcp40;
  int on_tcp37, on_tcp38, on_tcp39, on_tcp40;

  sockaddr_in m_addr_tcp41, l_addr_tcp41, m_addr_tcp42, l_addr_tcp42, m_addr_tcp43, l_addr_tcp43;
  int rc_tcp41, rc_tcp42, rc_tcp43;
  int m_sock_tcp41, m_sock_tcp42, m_sock_tcp43;
  int on_tcp41, on_tcp42, on_tcp43;

  sockaddr_in m_addr_tcp44, l_addr_tcp44, m_addr_tcp45, l_addr_tcp45, m_addr_tcp46, l_addr_tcp46;
  int rc_tcp44, rc_tcp45, rc_tcp46;
  int m_sock_tcp44, m_sock_tcp45, m_sock_tcp46;
  int on_tcp44, on_tcp45, on_tcp46;

  sockaddr_in m_addr_tcp47, l_addr_tcp47, m_addr_tcp48, l_addr_tcp48, m_addr_tcp49, l_addr_tcp49, m_addr_tcp50, l_addr_tcp50;
  int rc_tcp47, rc_tcp48, rc_tcp49, rc_tcp50;
  int m_sock_tcp47, m_sock_tcp48, m_sock_tcp49, m_sock_tcp50;
  int on_tcp47, on_tcp48, on_tcp49, on_tcp50;

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

  //prepare connection for tcp10 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp10, 0, sizeof ( m_addr_tcp10 ) );
  memset ( &l_addr_tcp10, 0, sizeof ( l_addr_tcp10 ) );
  
  m_sock_tcp10 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp10 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp10 = 1;
  if ( setsockopt ( m_sock_tcp10, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp10, sizeof ( on_tcp10 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp10.sin_family = AF_INET;
  l_addr_tcp10.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp10.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp10.sin_family = h_tcp10->h_addrtype;
  memcpy((char *) &m_addr_tcp10.sin_addr.s_addr, h_tcp10->h_addr_list[0], h_tcp10->h_length);
  m_addr_tcp10.sin_port = htons ( (unsigned short) TCP10_PORT );
  
  rc_tcp10 = bind ( m_sock_tcp10, ( struct sockaddr * ) &l_addr_tcp10, sizeof ( l_addr_tcp10 ) );
  
  if ( rc_tcp10 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp10 = connect ( m_sock_tcp10, ( struct sockaddr * ) &m_addr_tcp10, sizeof ( m_addr_tcp10 ) )) < 0) {
    if (debug) cout << "can not get tcp10 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp11 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp11, 0, sizeof ( m_addr_tcp11 ) );
  memset ( &l_addr_tcp11, 0, sizeof ( l_addr_tcp11 ) );
  
  m_sock_tcp11 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp11 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp11 = 1;
  if ( setsockopt ( m_sock_tcp11, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp11, sizeof ( on_tcp11 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp11.sin_family = AF_INET;
  l_addr_tcp11.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp11.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp11.sin_family = h_tcp11->h_addrtype;
  memcpy((char *) &m_addr_tcp11.sin_addr.s_addr, h_tcp11->h_addr_list[0], h_tcp11->h_length);
  m_addr_tcp11.sin_port = htons ( (unsigned short) TCP11_PORT );
  
  rc_tcp11 = bind ( m_sock_tcp11, ( struct sockaddr * ) &l_addr_tcp11, sizeof ( l_addr_tcp11 ) );
  
  if ( rc_tcp11 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp11 = connect ( m_sock_tcp11, ( struct sockaddr * ) &m_addr_tcp11, sizeof ( m_addr_tcp11 ) )) < 0) {
    if (debug) cout << "can not get tcp11 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }  
  
  //prepare connection for tcp12 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp12, 0, sizeof ( m_addr_tcp12 ) );
  memset ( &l_addr_tcp12, 0, sizeof ( l_addr_tcp12 ) );
  
  m_sock_tcp12 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp12 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp12 = 1;
  if ( setsockopt ( m_sock_tcp12, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp12, sizeof ( on_tcp12 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp12.sin_family = AF_INET;
  l_addr_tcp12.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp12.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp12.sin_family = h_tcp12->h_addrtype;
  memcpy((char *) &m_addr_tcp12.sin_addr.s_addr, h_tcp12->h_addr_list[0], h_tcp12->h_length);
  m_addr_tcp12.sin_port = htons ( (unsigned short) TCP12_PORT );
  
  rc_tcp12 = bind ( m_sock_tcp12, ( struct sockaddr * ) &l_addr_tcp12, sizeof ( l_addr_tcp12 ) );
  
  if ( rc_tcp12 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp12 = connect ( m_sock_tcp12, ( struct sockaddr * ) &m_addr_tcp12, sizeof ( m_addr_tcp12 ) )) < 0) {
    if (debug) cout << "can not get tcp12 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp13 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp13, 0, sizeof ( m_addr_tcp13 ) );
  memset ( &l_addr_tcp13, 0, sizeof ( l_addr_tcp13 ) );
  
  m_sock_tcp13 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp13 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp13 = 1;
  if ( setsockopt ( m_sock_tcp13, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp13, sizeof ( on_tcp13 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp13.sin_family = AF_INET;
  l_addr_tcp13.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp13.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp13.sin_family = h_tcp13->h_addrtype;
  memcpy((char *) &m_addr_tcp13.sin_addr.s_addr, h_tcp13->h_addr_list[0], h_tcp13->h_length);
  m_addr_tcp13.sin_port = htons ( (unsigned short) TCP13_PORT );
  
  rc_tcp13 = bind ( m_sock_tcp13, ( struct sockaddr * ) &l_addr_tcp13, sizeof ( l_addr_tcp13 ) );
  
  if ( rc_tcp13 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp13 = connect ( m_sock_tcp13, ( struct sockaddr * ) &m_addr_tcp13, sizeof ( m_addr_tcp13 ) )) < 0) {
    if (debug) cout << "can not get tcp13 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp14 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp14, 0, sizeof ( m_addr_tcp14 ) );
  memset ( &l_addr_tcp14, 0, sizeof ( l_addr_tcp14 ) );
  
  m_sock_tcp14 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp14 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp14 = 1;
  if ( setsockopt ( m_sock_tcp14, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp14, sizeof ( on_tcp14 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp14.sin_family = AF_INET;
  l_addr_tcp14.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp14.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp14.sin_family = h_tcp14->h_addrtype;
  memcpy((char *) &m_addr_tcp14.sin_addr.s_addr, h_tcp14->h_addr_list[0], h_tcp14->h_length);
  m_addr_tcp14.sin_port = htons ( (unsigned short) TCP14_PORT );
  
  rc_tcp14 = bind ( m_sock_tcp14, ( struct sockaddr * ) &l_addr_tcp14, sizeof ( l_addr_tcp14 ) );
  
  if ( rc_tcp14 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp14 = connect ( m_sock_tcp14, ( struct sockaddr * ) &m_addr_tcp14, sizeof ( m_addr_tcp14 ) )) < 0) {
    if (debug) cout << "can not get tcp14 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp15 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp15, 0, sizeof ( m_addr_tcp15 ) );
  memset ( &l_addr_tcp15, 0, sizeof ( l_addr_tcp15 ) );
  
  m_sock_tcp15 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp15 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp15 = 1;
  if ( setsockopt ( m_sock_tcp15, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp15, sizeof ( on_tcp15 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp15.sin_family = AF_INET;
  l_addr_tcp15.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp15.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp15.sin_family = h_tcp15->h_addrtype;
  memcpy((char *) &m_addr_tcp15.sin_addr.s_addr, h_tcp15->h_addr_list[0], h_tcp15->h_length);
  m_addr_tcp15.sin_port = htons ( (unsigned short) TCP15_PORT );
  
  rc_tcp15 = bind ( m_sock_tcp15, ( struct sockaddr * ) &l_addr_tcp15, sizeof ( l_addr_tcp15 ) );
  
  if ( rc_tcp15 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp15 = connect ( m_sock_tcp15, ( struct sockaddr * ) &m_addr_tcp15, sizeof ( m_addr_tcp15 ) )) < 0) {
    if (debug) cout << "can not get tcp15 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp16 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp16, 0, sizeof ( m_addr_tcp16 ) );
  memset ( &l_addr_tcp16, 0, sizeof ( l_addr_tcp16 ) );
  
  m_sock_tcp16 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp16 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp16 = 1;
  if ( setsockopt ( m_sock_tcp16, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp16, sizeof ( on_tcp16 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp16.sin_family = AF_INET;
  l_addr_tcp16.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp16.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp16.sin_family = h_tcp16->h_addrtype;
  memcpy((char *) &m_addr_tcp16.sin_addr.s_addr, h_tcp16->h_addr_list[0], h_tcp16->h_length);
  m_addr_tcp16.sin_port = htons ( (unsigned short) TCP16_PORT );
  
  rc_tcp16 = bind ( m_sock_tcp16, ( struct sockaddr * ) &l_addr_tcp16, sizeof ( l_addr_tcp16 ) );
  
  if ( rc_tcp16 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp16 = connect ( m_sock_tcp16, ( struct sockaddr * ) &m_addr_tcp16, sizeof ( m_addr_tcp16 ) )) < 0) {
    if (debug) cout << "can not get tcp16 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp17 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp17, 0, sizeof ( m_addr_tcp17 ) );
  memset ( &l_addr_tcp17, 0, sizeof ( l_addr_tcp17 ) );
  
  m_sock_tcp17 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp17 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp17 = 1;
  if ( setsockopt ( m_sock_tcp17, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp17, sizeof ( on_tcp17 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp17.sin_family = AF_INET;
  l_addr_tcp17.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp17.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp17.sin_family = h_tcp17->h_addrtype;
  memcpy((char *) &m_addr_tcp17.sin_addr.s_addr, h_tcp17->h_addr_list[0], h_tcp17->h_length);
  m_addr_tcp17.sin_port = htons ( (unsigned short) TCP17_PORT );
  
  rc_tcp17 = bind ( m_sock_tcp17, ( struct sockaddr * ) &l_addr_tcp17, sizeof ( l_addr_tcp17 ) );
  
  if ( rc_tcp17 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp17 = connect ( m_sock_tcp17, ( struct sockaddr * ) &m_addr_tcp17, sizeof ( m_addr_tcp17 ) )) < 0) {
    if (debug) cout << "can not get tcp17 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp18 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp18, 0, sizeof ( m_addr_tcp18 ) );
  memset ( &l_addr_tcp18, 0, sizeof ( l_addr_tcp18 ) );
  
  m_sock_tcp18 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp18 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp18 = 1;
  if ( setsockopt ( m_sock_tcp18, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp18, sizeof ( on_tcp18 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp18.sin_family = AF_INET;
  l_addr_tcp18.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp18.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp18.sin_family = h_tcp18->h_addrtype;
  memcpy((char *) &m_addr_tcp18.sin_addr.s_addr, h_tcp18->h_addr_list[0], h_tcp18->h_length);
  m_addr_tcp18.sin_port = htons ( (unsigned short) TCP18_PORT );
  
  rc_tcp18 = bind ( m_sock_tcp18, ( struct sockaddr * ) &l_addr_tcp18, sizeof ( l_addr_tcp18 ) );
  
  if ( rc_tcp18 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp18 = connect ( m_sock_tcp18, ( struct sockaddr * ) &m_addr_tcp18, sizeof ( m_addr_tcp18 ) )) < 0) {
    if (debug) cout << "can not get tcp18 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp19 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp19, 0, sizeof ( m_addr_tcp19 ) );
  memset ( &l_addr_tcp19, 0, sizeof ( l_addr_tcp19 ) );
  
  m_sock_tcp19 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp19 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp19 = 1;
  if ( setsockopt ( m_sock_tcp19, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp19, sizeof ( on_tcp19 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp19.sin_family = AF_INET;
  l_addr_tcp19.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp19.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp19.sin_family = h_tcp19->h_addrtype;
  memcpy((char *) &m_addr_tcp19.sin_addr.s_addr, h_tcp19->h_addr_list[0], h_tcp19->h_length);
  m_addr_tcp19.sin_port = htons ( (unsigned short) TCP19_PORT );
  
  rc_tcp19 = bind ( m_sock_tcp19, ( struct sockaddr * ) &l_addr_tcp19, sizeof ( l_addr_tcp19 ) );
  
  if ( rc_tcp19 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp19 = connect ( m_sock_tcp19, ( struct sockaddr * ) &m_addr_tcp19, sizeof ( m_addr_tcp19 ) )) < 0) {
    if (debug) cout << "can not get tcp19 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp20 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp20, 0, sizeof ( m_addr_tcp20 ) );
  memset ( &l_addr_tcp20, 0, sizeof ( l_addr_tcp20 ) );
  
  m_sock_tcp20 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp20 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp20 = 1;
  if ( setsockopt ( m_sock_tcp20, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp20, sizeof ( on_tcp20 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp20.sin_family = AF_INET;
  l_addr_tcp20.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp20.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp20.sin_family = h_tcp20->h_addrtype;
  memcpy((char *) &m_addr_tcp20.sin_addr.s_addr, h_tcp20->h_addr_list[0], h_tcp20->h_length);
  m_addr_tcp20.sin_port = htons ( (unsigned short) TCP20_PORT );
  
  rc_tcp20 = bind ( m_sock_tcp20, ( struct sockaddr * ) &l_addr_tcp20, sizeof ( l_addr_tcp20 ) );
  
  if ( rc_tcp20 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp20 = connect ( m_sock_tcp20, ( struct sockaddr * ) &m_addr_tcp20, sizeof ( m_addr_tcp20 ) )) < 0) {
    if (debug) cout << "can not get tcp20 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp21 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp21, 0, sizeof ( m_addr_tcp21 ) );
  memset ( &l_addr_tcp21, 0, sizeof ( l_addr_tcp21 ) );
  
  m_sock_tcp21 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp21 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp21 = 1;
  if ( setsockopt ( m_sock_tcp21, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp21, sizeof ( on_tcp21 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp21.sin_family = AF_INET;
  l_addr_tcp21.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp21.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp21.sin_family = h_tcp21->h_addrtype;
  memcpy((char *) &m_addr_tcp21.sin_addr.s_addr, h_tcp21->h_addr_list[0], h_tcp21->h_length);
  m_addr_tcp21.sin_port = htons ( (unsigned short) TCP21_PORT );
  
  rc_tcp21 = bind ( m_sock_tcp21, ( struct sockaddr * ) &l_addr_tcp21, sizeof ( l_addr_tcp21 ) );
  
  if ( rc_tcp21 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp21 = connect ( m_sock_tcp21, ( struct sockaddr * ) &m_addr_tcp21, sizeof ( m_addr_tcp21 ) )) < 0) {
    if (debug) cout << "can not get tcp21 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }  
  
  //prepare connection for tcp22 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp22, 0, sizeof ( m_addr_tcp22 ) );
  memset ( &l_addr_tcp22, 0, sizeof ( l_addr_tcp22 ) );
  
  m_sock_tcp22 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp22 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp22 = 1;
  if ( setsockopt ( m_sock_tcp22, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp22, sizeof ( on_tcp22 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp22.sin_family = AF_INET;
  l_addr_tcp22.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp22.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp22.sin_family = h_tcp22->h_addrtype;
  memcpy((char *) &m_addr_tcp22.sin_addr.s_addr, h_tcp22->h_addr_list[0], h_tcp22->h_length);
  m_addr_tcp22.sin_port = htons ( (unsigned short) TCP22_PORT );
  
  rc_tcp22 = bind ( m_sock_tcp22, ( struct sockaddr * ) &l_addr_tcp22, sizeof ( l_addr_tcp22 ) );
  
  if ( rc_tcp22 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp22 = connect ( m_sock_tcp22, ( struct sockaddr * ) &m_addr_tcp22, sizeof ( m_addr_tcp22 ) )) < 0) {
    if (debug) cout << "can not get tcp22 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp23 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp23, 0, sizeof ( m_addr_tcp23 ) );
  memset ( &l_addr_tcp23, 0, sizeof ( l_addr_tcp23 ) );
  
  m_sock_tcp23 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp23 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp23 = 1;
  if ( setsockopt ( m_sock_tcp23, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp23, sizeof ( on_tcp23 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp23.sin_family = AF_INET;
  l_addr_tcp23.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp23.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp23.sin_family = h_tcp23->h_addrtype;
  memcpy((char *) &m_addr_tcp23.sin_addr.s_addr, h_tcp23->h_addr_list[0], h_tcp23->h_length);
  m_addr_tcp23.sin_port = htons ( (unsigned short) TCP23_PORT );
  
  rc_tcp23 = bind ( m_sock_tcp23, ( struct sockaddr * ) &l_addr_tcp23, sizeof ( l_addr_tcp23 ) );
  
  if ( rc_tcp23 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp23 = connect ( m_sock_tcp23, ( struct sockaddr * ) &m_addr_tcp23, sizeof ( m_addr_tcp23 ) )) < 0) {
    if (debug) cout << "can not get tcp23 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp24 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp24, 0, sizeof ( m_addr_tcp24 ) );
  memset ( &l_addr_tcp24, 0, sizeof ( l_addr_tcp24 ) );
  
  m_sock_tcp24 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp24 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp24 = 1;
  if ( setsockopt ( m_sock_tcp24, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp24, sizeof ( on_tcp24 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp24.sin_family = AF_INET;
  l_addr_tcp24.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp24.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp24.sin_family = h_tcp24->h_addrtype;
  memcpy((char *) &m_addr_tcp24.sin_addr.s_addr, h_tcp24->h_addr_list[0], h_tcp24->h_length);
  m_addr_tcp24.sin_port = htons ( (unsigned short) TCP24_PORT );
  
  rc_tcp24 = bind ( m_sock_tcp24, ( struct sockaddr * ) &l_addr_tcp24, sizeof ( l_addr_tcp24 ) );
  
  if ( rc_tcp24 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp24 = connect ( m_sock_tcp24, ( struct sockaddr * ) &m_addr_tcp24, sizeof ( m_addr_tcp24 ) )) < 0) {
    if (debug) cout << "can not get tcp24 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp25 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp25, 0, sizeof ( m_addr_tcp25 ) );
  memset ( &l_addr_tcp25, 0, sizeof ( l_addr_tcp25 ) );
  
  m_sock_tcp25 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp25 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp25 = 1;
  if ( setsockopt ( m_sock_tcp25, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp25, sizeof ( on_tcp25 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp25.sin_family = AF_INET;
  l_addr_tcp25.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp25.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp25.sin_family = h_tcp25->h_addrtype;
  memcpy((char *) &m_addr_tcp25.sin_addr.s_addr, h_tcp25->h_addr_list[0], h_tcp25->h_length);
  m_addr_tcp25.sin_port = htons ( (unsigned short) TCP25_PORT );
  
  rc_tcp25 = bind ( m_sock_tcp25, ( struct sockaddr * ) &l_addr_tcp25, sizeof ( l_addr_tcp25 ) );
  
  if ( rc_tcp25 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp25 = connect ( m_sock_tcp25, ( struct sockaddr * ) &m_addr_tcp25, sizeof ( m_addr_tcp25 ) )) < 0) {
    if (debug) cout << "can not get tcp25 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp26 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp26, 0, sizeof ( m_addr_tcp26 ) );
  memset ( &l_addr_tcp26, 0, sizeof ( l_addr_tcp26 ) );
  
  m_sock_tcp26 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp26 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp26 = 1;
  if ( setsockopt ( m_sock_tcp26, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp26, sizeof ( on_tcp26 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp26.sin_family = AF_INET;
  l_addr_tcp26.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp26.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp26.sin_family = h_tcp26->h_addrtype;
  memcpy((char *) &m_addr_tcp26.sin_addr.s_addr, h_tcp26->h_addr_list[0], h_tcp26->h_length);
  m_addr_tcp26.sin_port = htons ( (unsigned short) TCP26_PORT );
  
  rc_tcp26 = bind ( m_sock_tcp26, ( struct sockaddr * ) &l_addr_tcp26, sizeof ( l_addr_tcp26 ) );
  
  if ( rc_tcp26 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp26 = connect ( m_sock_tcp26, ( struct sockaddr * ) &m_addr_tcp26, sizeof ( m_addr_tcp26 ) )) < 0) {
    if (debug) cout << "can not get tcp26 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp27 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp27, 0, sizeof ( m_addr_tcp27 ) );
  memset ( &l_addr_tcp27, 0, sizeof ( l_addr_tcp27 ) );
  
  m_sock_tcp27 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp27 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp27 = 1;
  if ( setsockopt ( m_sock_tcp27, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp27, sizeof ( on_tcp27 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp27.sin_family = AF_INET;
  l_addr_tcp27.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp27.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp27.sin_family = h_tcp27->h_addrtype;
  memcpy((char *) &m_addr_tcp27.sin_addr.s_addr, h_tcp27->h_addr_list[0], h_tcp27->h_length);
  m_addr_tcp27.sin_port = htons ( (unsigned short) TCP27_PORT );
  
  rc_tcp27 = bind ( m_sock_tcp27, ( struct sockaddr * ) &l_addr_tcp27, sizeof ( l_addr_tcp27 ) );
  
  if ( rc_tcp27 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp27 = connect ( m_sock_tcp27, ( struct sockaddr * ) &m_addr_tcp27, sizeof ( m_addr_tcp27 ) )) < 0) {
    if (debug) cout << "can not get tcp27 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp28 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp28, 0, sizeof ( m_addr_tcp28 ) );
  memset ( &l_addr_tcp28, 0, sizeof ( l_addr_tcp28 ) );
  
  m_sock_tcp28 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp28 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp28 = 1;
  if ( setsockopt ( m_sock_tcp28, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp28, sizeof ( on_tcp28 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp28.sin_family = AF_INET;
  l_addr_tcp28.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp28.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp28.sin_family = h_tcp28->h_addrtype;
  memcpy((char *) &m_addr_tcp28.sin_addr.s_addr, h_tcp28->h_addr_list[0], h_tcp28->h_length);
  m_addr_tcp28.sin_port = htons ( (unsigned short) TCP28_PORT );
  
  rc_tcp28 = bind ( m_sock_tcp28, ( struct sockaddr * ) &l_addr_tcp28, sizeof ( l_addr_tcp28 ) );
  
  if ( rc_tcp28 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp28 = connect ( m_sock_tcp28, ( struct sockaddr * ) &m_addr_tcp28, sizeof ( m_addr_tcp28 ) )) < 0) {
    if (debug) cout << "can not get tcp28 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp29 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp29, 0, sizeof ( m_addr_tcp29 ) );
  memset ( &l_addr_tcp29, 0, sizeof ( l_addr_tcp29 ) );
  
  m_sock_tcp29 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp29 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp29 = 1;
  if ( setsockopt ( m_sock_tcp29, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp29, sizeof ( on_tcp29 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp29.sin_family = AF_INET;
  l_addr_tcp29.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp29.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp29.sin_family = h_tcp29->h_addrtype;
  memcpy((char *) &m_addr_tcp29.sin_addr.s_addr, h_tcp29->h_addr_list[0], h_tcp29->h_length);
  m_addr_tcp29.sin_port = htons ( (unsigned short) TCP29_PORT );
  
  rc_tcp29 = bind ( m_sock_tcp29, ( struct sockaddr * ) &l_addr_tcp29, sizeof ( l_addr_tcp29 ) );
  
  if ( rc_tcp29 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp29 = connect ( m_sock_tcp29, ( struct sockaddr * ) &m_addr_tcp29, sizeof ( m_addr_tcp29 ) )) < 0) {
    if (debug) cout << "can not get tcp29 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp30 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp30, 0, sizeof ( m_addr_tcp30 ) );
  memset ( &l_addr_tcp30, 0, sizeof ( l_addr_tcp30 ) );
  
  m_sock_tcp30 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp30 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp30 = 1;
  if ( setsockopt ( m_sock_tcp30, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp30, sizeof ( on_tcp30 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp30.sin_family = AF_INET;
  l_addr_tcp30.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp30.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp30.sin_family = h_tcp30->h_addrtype;
  memcpy((char *) &m_addr_tcp30.sin_addr.s_addr, h_tcp30->h_addr_list[0], h_tcp30->h_length);
  m_addr_tcp30.sin_port = htons ( (unsigned short) TCP30_PORT );
  
  rc_tcp30 = bind ( m_sock_tcp30, ( struct sockaddr * ) &l_addr_tcp30, sizeof ( l_addr_tcp30 ) );
  
  if ( rc_tcp30 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp30 = connect ( m_sock_tcp30, ( struct sockaddr * ) &m_addr_tcp30, sizeof ( m_addr_tcp30 ) )) < 0) {
    if (debug) cout << "can not get tcp30 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp31 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp31, 0, sizeof ( m_addr_tcp31 ) );
  memset ( &l_addr_tcp31, 0, sizeof ( l_addr_tcp31 ) );
  
  m_sock_tcp31 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp31 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp31 = 1;
  if ( setsockopt ( m_sock_tcp31, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp31, sizeof ( on_tcp31 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp31.sin_family = AF_INET;
  l_addr_tcp31.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp31.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp31.sin_family = h_tcp31->h_addrtype;
  memcpy((char *) &m_addr_tcp31.sin_addr.s_addr, h_tcp31->h_addr_list[0], h_tcp31->h_length);
  m_addr_tcp31.sin_port = htons ( (unsigned short) TCP31_PORT );
  
  rc_tcp31 = bind ( m_sock_tcp31, ( struct sockaddr * ) &l_addr_tcp31, sizeof ( l_addr_tcp31 ) );
  
  if ( rc_tcp31 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp31 = connect ( m_sock_tcp31, ( struct sockaddr * ) &m_addr_tcp31, sizeof ( m_addr_tcp31 ) )) < 0) {
    if (debug) cout << "can not get tcp31 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }  
  
  //prepare connection for tcp32 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp32, 0, sizeof ( m_addr_tcp32 ) );
  memset ( &l_addr_tcp32, 0, sizeof ( l_addr_tcp32 ) );
  
  m_sock_tcp32 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp32 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp32 = 1;
  if ( setsockopt ( m_sock_tcp32, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp32, sizeof ( on_tcp32 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp32.sin_family = AF_INET;
  l_addr_tcp32.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp32.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp32.sin_family = h_tcp32->h_addrtype;
  memcpy((char *) &m_addr_tcp32.sin_addr.s_addr, h_tcp32->h_addr_list[0], h_tcp32->h_length);
  m_addr_tcp32.sin_port = htons ( (unsigned short) TCP32_PORT );
  
  rc_tcp32 = bind ( m_sock_tcp32, ( struct sockaddr * ) &l_addr_tcp32, sizeof ( l_addr_tcp32 ) );
  
  if ( rc_tcp32 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp32 = connect ( m_sock_tcp32, ( struct sockaddr * ) &m_addr_tcp32, sizeof ( m_addr_tcp32 ) )) < 0) {
    if (debug) cout << "can not get tcp32 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp33 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp33, 0, sizeof ( m_addr_tcp33 ) );
  memset ( &l_addr_tcp33, 0, sizeof ( l_addr_tcp33 ) );
  
  m_sock_tcp33 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp33 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp33 = 1;
  if ( setsockopt ( m_sock_tcp33, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp33, sizeof ( on_tcp33 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp33.sin_family = AF_INET;
  l_addr_tcp33.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp33.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp33.sin_family = h_tcp33->h_addrtype;
  memcpy((char *) &m_addr_tcp33.sin_addr.s_addr, h_tcp33->h_addr_list[0], h_tcp33->h_length);
  m_addr_tcp33.sin_port = htons ( (unsigned short) TCP33_PORT );
  
  rc_tcp33 = bind ( m_sock_tcp33, ( struct sockaddr * ) &l_addr_tcp33, sizeof ( l_addr_tcp33 ) );
  
  if ( rc_tcp33 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp33 = connect ( m_sock_tcp33, ( struct sockaddr * ) &m_addr_tcp33, sizeof ( m_addr_tcp33 ) )) < 0) {
    if (debug) cout << "can not get tcp33 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp34 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp34, 0, sizeof ( m_addr_tcp34 ) );
  memset ( &l_addr_tcp34, 0, sizeof ( l_addr_tcp34 ) );
  
  m_sock_tcp34 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp34 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp34 = 1;
  if ( setsockopt ( m_sock_tcp34, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp34, sizeof ( on_tcp34 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp34.sin_family = AF_INET;
  l_addr_tcp34.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp34.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp34.sin_family = h_tcp34->h_addrtype;
  memcpy((char *) &m_addr_tcp34.sin_addr.s_addr, h_tcp34->h_addr_list[0], h_tcp34->h_length);
  m_addr_tcp34.sin_port = htons ( (unsigned short) TCP34_PORT );
  
  rc_tcp34 = bind ( m_sock_tcp34, ( struct sockaddr * ) &l_addr_tcp34, sizeof ( l_addr_tcp34 ) );
  
  if ( rc_tcp34 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp34 = connect ( m_sock_tcp34, ( struct sockaddr * ) &m_addr_tcp34, sizeof ( m_addr_tcp34 ) )) < 0) {
    if (debug) cout << "can not get tcp34 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp35 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp35, 0, sizeof ( m_addr_tcp35 ) );
  memset ( &l_addr_tcp35, 0, sizeof ( l_addr_tcp35 ) );
  
  m_sock_tcp35 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp35 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp35 = 1;
  if ( setsockopt ( m_sock_tcp35, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp35, sizeof ( on_tcp35 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp35.sin_family = AF_INET;
  l_addr_tcp35.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp35.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp35.sin_family = h_tcp35->h_addrtype;
  memcpy((char *) &m_addr_tcp35.sin_addr.s_addr, h_tcp35->h_addr_list[0], h_tcp35->h_length);
  m_addr_tcp35.sin_port = htons ( (unsigned short) TCP35_PORT );
  
  rc_tcp35 = bind ( m_sock_tcp35, ( struct sockaddr * ) &l_addr_tcp35, sizeof ( l_addr_tcp35 ) );
  
  if ( rc_tcp35 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp35 = connect ( m_sock_tcp35, ( struct sockaddr * ) &m_addr_tcp35, sizeof ( m_addr_tcp35 ) )) < 0) {
    if (debug) cout << "can not get tcp35 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp36 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp36, 0, sizeof ( m_addr_tcp36 ) );
  memset ( &l_addr_tcp36, 0, sizeof ( l_addr_tcp36 ) );
  
  m_sock_tcp36 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp36 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp36 = 1;
  if ( setsockopt ( m_sock_tcp36, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp36, sizeof ( on_tcp36 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp36.sin_family = AF_INET;
  l_addr_tcp36.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp36.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp36.sin_family = h_tcp36->h_addrtype;
  memcpy((char *) &m_addr_tcp36.sin_addr.s_addr, h_tcp36->h_addr_list[0], h_tcp36->h_length);
  m_addr_tcp36.sin_port = htons ( (unsigned short) TCP36_PORT );
  
  rc_tcp36 = bind ( m_sock_tcp36, ( struct sockaddr * ) &l_addr_tcp36, sizeof ( l_addr_tcp36 ) );
  
  if ( rc_tcp36 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp36 = connect ( m_sock_tcp36, ( struct sockaddr * ) &m_addr_tcp36, sizeof ( m_addr_tcp36 ) )) < 0) {
    if (debug) cout << "can not get tcp36 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp37 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp37, 0, sizeof ( m_addr_tcp37 ) );
  memset ( &l_addr_tcp37, 0, sizeof ( l_addr_tcp37 ) );
  
  m_sock_tcp37 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp37 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp37 = 1;
  if ( setsockopt ( m_sock_tcp37, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp37, sizeof ( on_tcp37 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp37.sin_family = AF_INET;
  l_addr_tcp37.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp37.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp37.sin_family = h_tcp37->h_addrtype;
  memcpy((char *) &m_addr_tcp37.sin_addr.s_addr, h_tcp37->h_addr_list[0], h_tcp37->h_length);
  m_addr_tcp37.sin_port = htons ( (unsigned short) TCP37_PORT );
  
  rc_tcp37 = bind ( m_sock_tcp37, ( struct sockaddr * ) &l_addr_tcp37, sizeof ( l_addr_tcp37 ) );
  
  if ( rc_tcp37 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp37 = connect ( m_sock_tcp37, ( struct sockaddr * ) &m_addr_tcp37, sizeof ( m_addr_tcp37 ) )) < 0) {
    if (debug) cout << "can not get tcp37 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp38 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp38, 0, sizeof ( m_addr_tcp38 ) );
  memset ( &l_addr_tcp38, 0, sizeof ( l_addr_tcp38 ) );
  
  m_sock_tcp38 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp38 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp38 = 1;
  if ( setsockopt ( m_sock_tcp38, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp38, sizeof ( on_tcp38 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp38.sin_family = AF_INET;
  l_addr_tcp38.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp38.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp38.sin_family = h_tcp38->h_addrtype;
  memcpy((char *) &m_addr_tcp38.sin_addr.s_addr, h_tcp38->h_addr_list[0], h_tcp38->h_length);
  m_addr_tcp38.sin_port = htons ( (unsigned short) TCP38_PORT );
  
  rc_tcp38 = bind ( m_sock_tcp38, ( struct sockaddr * ) &l_addr_tcp38, sizeof ( l_addr_tcp38 ) );
  
  if ( rc_tcp38 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp38 = connect ( m_sock_tcp38, ( struct sockaddr * ) &m_addr_tcp38, sizeof ( m_addr_tcp38 ) )) < 0) {
    if (debug) cout << "can not get tcp38 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp39 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp39, 0, sizeof ( m_addr_tcp39 ) );
  memset ( &l_addr_tcp39, 0, sizeof ( l_addr_tcp39 ) );
  
  m_sock_tcp39 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp39 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp39 = 1;
  if ( setsockopt ( m_sock_tcp39, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp39, sizeof ( on_tcp39 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp39.sin_family = AF_INET;
  l_addr_tcp39.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp39.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp39.sin_family = h_tcp39->h_addrtype;
  memcpy((char *) &m_addr_tcp39.sin_addr.s_addr, h_tcp39->h_addr_list[0], h_tcp39->h_length);
  m_addr_tcp39.sin_port = htons ( (unsigned short) TCP39_PORT );
  
  rc_tcp39 = bind ( m_sock_tcp39, ( struct sockaddr * ) &l_addr_tcp39, sizeof ( l_addr_tcp39 ) );
  
  if ( rc_tcp39 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp39 = connect ( m_sock_tcp39, ( struct sockaddr * ) &m_addr_tcp39, sizeof ( m_addr_tcp39 ) )) < 0) {
    if (debug) cout << "can not get tcp39 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp40 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp40, 0, sizeof ( m_addr_tcp40 ) );
  memset ( &l_addr_tcp40, 0, sizeof ( l_addr_tcp40 ) );
  
  m_sock_tcp40 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp40 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp40 = 1;
  if ( setsockopt ( m_sock_tcp40, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp40, sizeof ( on_tcp40 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp40.sin_family = AF_INET;
  l_addr_tcp40.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp40.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp40.sin_family = h_tcp40->h_addrtype;
  memcpy((char *) &m_addr_tcp40.sin_addr.s_addr, h_tcp40->h_addr_list[0], h_tcp40->h_length);
  m_addr_tcp40.sin_port = htons ( (unsigned short) TCP40_PORT );
  
  rc_tcp40 = bind ( m_sock_tcp40, ( struct sockaddr * ) &l_addr_tcp40, sizeof ( l_addr_tcp40 ) );
  
  if ( rc_tcp40 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp40 = connect ( m_sock_tcp40, ( struct sockaddr * ) &m_addr_tcp40, sizeof ( m_addr_tcp40 ) )) < 0) {
    if (debug) cout << "can not get tcp40 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp41 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp41, 0, sizeof ( m_addr_tcp41 ) );
  memset ( &l_addr_tcp41, 0, sizeof ( l_addr_tcp41 ) );
  
  m_sock_tcp41 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp41 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp41 = 1;
  if ( setsockopt ( m_sock_tcp41, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp41, sizeof ( on_tcp41 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp41.sin_family = AF_INET;
  l_addr_tcp41.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp41.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp41.sin_family = h_tcp41->h_addrtype;
  memcpy((char *) &m_addr_tcp41.sin_addr.s_addr, h_tcp41->h_addr_list[0], h_tcp41->h_length);
  m_addr_tcp41.sin_port = htons ( (unsigned short) TCP41_PORT );
  
  rc_tcp41 = bind ( m_sock_tcp41, ( struct sockaddr * ) &l_addr_tcp41, sizeof ( l_addr_tcp41 ) );
  
  if ( rc_tcp41 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp41 = connect ( m_sock_tcp41, ( struct sockaddr * ) &m_addr_tcp41, sizeof ( m_addr_tcp41 ) )) < 0) {
    if (debug) cout << "can not get tcp41 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }  
  
  //prepare connection for tcp42 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp42, 0, sizeof ( m_addr_tcp42 ) );
  memset ( &l_addr_tcp42, 0, sizeof ( l_addr_tcp42 ) );
  
  m_sock_tcp42 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp42 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp42 = 1;
  if ( setsockopt ( m_sock_tcp42, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp42, sizeof ( on_tcp42 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp42.sin_family = AF_INET;
  l_addr_tcp42.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp42.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp42.sin_family = h_tcp42->h_addrtype;
  memcpy((char *) &m_addr_tcp42.sin_addr.s_addr, h_tcp42->h_addr_list[0], h_tcp42->h_length);
  m_addr_tcp42.sin_port = htons ( (unsigned short) TCP42_PORT );
  
  rc_tcp42 = bind ( m_sock_tcp42, ( struct sockaddr * ) &l_addr_tcp42, sizeof ( l_addr_tcp42 ) );
  
  if ( rc_tcp42 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp42 = connect ( m_sock_tcp42, ( struct sockaddr * ) &m_addr_tcp42, sizeof ( m_addr_tcp42 ) )) < 0) {
    if (debug) cout << "can not get tcp42 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp43 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp43, 0, sizeof ( m_addr_tcp43 ) );
  memset ( &l_addr_tcp43, 0, sizeof ( l_addr_tcp43 ) );
  
  m_sock_tcp43 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp43 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp43 = 1;
  if ( setsockopt ( m_sock_tcp43, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp43, sizeof ( on_tcp43 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp43.sin_family = AF_INET;
  l_addr_tcp43.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp43.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp43.sin_family = h_tcp43->h_addrtype;
  memcpy((char *) &m_addr_tcp43.sin_addr.s_addr, h_tcp43->h_addr_list[0], h_tcp43->h_length);
  m_addr_tcp43.sin_port = htons ( (unsigned short) TCP43_PORT );
  
  rc_tcp43 = bind ( m_sock_tcp43, ( struct sockaddr * ) &l_addr_tcp43, sizeof ( l_addr_tcp43 ) );
  
  if ( rc_tcp43 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp43 = connect ( m_sock_tcp43, ( struct sockaddr * ) &m_addr_tcp43, sizeof ( m_addr_tcp43 ) )) < 0) {
    if (debug) cout << "can not get tcp43 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp44 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp44, 0, sizeof ( m_addr_tcp44 ) );
  memset ( &l_addr_tcp44, 0, sizeof ( l_addr_tcp44 ) );
  
  m_sock_tcp44 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp44 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp44 = 1;
  if ( setsockopt ( m_sock_tcp44, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp44, sizeof ( on_tcp44 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp44.sin_family = AF_INET;
  l_addr_tcp44.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp44.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp44.sin_family = h_tcp44->h_addrtype;
  memcpy((char *) &m_addr_tcp44.sin_addr.s_addr, h_tcp44->h_addr_list[0], h_tcp44->h_length);
  m_addr_tcp44.sin_port = htons ( (unsigned short) TCP44_PORT );
  
  rc_tcp44 = bind ( m_sock_tcp44, ( struct sockaddr * ) &l_addr_tcp44, sizeof ( l_addr_tcp44 ) );
  
  if ( rc_tcp44 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp44 = connect ( m_sock_tcp44, ( struct sockaddr * ) &m_addr_tcp44, sizeof ( m_addr_tcp44 ) )) < 0) {
    if (debug) cout << "can not get tcp44 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp45 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp45, 0, sizeof ( m_addr_tcp45 ) );
  memset ( &l_addr_tcp45, 0, sizeof ( l_addr_tcp45 ) );
  
  m_sock_tcp45 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp45 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp45 = 1;
  if ( setsockopt ( m_sock_tcp45, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp45, sizeof ( on_tcp45 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp45.sin_family = AF_INET;
  l_addr_tcp45.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp45.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp45.sin_family = h_tcp45->h_addrtype;
  memcpy((char *) &m_addr_tcp45.sin_addr.s_addr, h_tcp45->h_addr_list[0], h_tcp45->h_length);
  m_addr_tcp45.sin_port = htons ( (unsigned short) TCP45_PORT );
  
  rc_tcp45 = bind ( m_sock_tcp45, ( struct sockaddr * ) &l_addr_tcp45, sizeof ( l_addr_tcp45 ) );
  
  if ( rc_tcp45 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp45 = connect ( m_sock_tcp45, ( struct sockaddr * ) &m_addr_tcp45, sizeof ( m_addr_tcp45 ) )) < 0) {
    if (debug) cout << "can not get tcp45 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp46 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp46, 0, sizeof ( m_addr_tcp46 ) );
  memset ( &l_addr_tcp46, 0, sizeof ( l_addr_tcp46 ) );
  
  m_sock_tcp46 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp46 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp46 = 1;
  if ( setsockopt ( m_sock_tcp46, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp46, sizeof ( on_tcp46 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp46.sin_family = AF_INET;
  l_addr_tcp46.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp46.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp46.sin_family = h_tcp46->h_addrtype;
  memcpy((char *) &m_addr_tcp46.sin_addr.s_addr, h_tcp46->h_addr_list[0], h_tcp46->h_length);
  m_addr_tcp46.sin_port = htons ( (unsigned short) TCP46_PORT );
  
  rc_tcp46 = bind ( m_sock_tcp46, ( struct sockaddr * ) &l_addr_tcp46, sizeof ( l_addr_tcp46 ) );
  
  if ( rc_tcp46 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp46 = connect ( m_sock_tcp46, ( struct sockaddr * ) &m_addr_tcp46, sizeof ( m_addr_tcp46 ) )) < 0) {
    if (debug) cout << "can not get tcp46 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp47 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp47, 0, sizeof ( m_addr_tcp47 ) );
  memset ( &l_addr_tcp47, 0, sizeof ( l_addr_tcp47 ) );
  
  m_sock_tcp47 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp47 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp47 = 1;
  if ( setsockopt ( m_sock_tcp47, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp47, sizeof ( on_tcp47 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp47.sin_family = AF_INET;
  l_addr_tcp47.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp47.sin_port = htons((unsigned short)0);
  
    //remote server
  m_addr_tcp47.sin_family = h_tcp47->h_addrtype;
  memcpy((char *) &m_addr_tcp47.sin_addr.s_addr, h_tcp47->h_addr_list[0], h_tcp47->h_length);
  m_addr_tcp47.sin_port = htons ( (unsigned short) TCP47_PORT );
  
  rc_tcp47 = bind ( m_sock_tcp47, ( struct sockaddr * ) &l_addr_tcp47, sizeof ( l_addr_tcp47 ) );
  
  if ( rc_tcp47 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
    
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp47 = connect ( m_sock_tcp47, ( struct sockaddr * ) &m_addr_tcp47, sizeof ( m_addr_tcp47 ) )) < 0) {
    if (debug) cout << "can not get tcp47 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp48 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp48, 0, sizeof ( m_addr_tcp48 ) );
  memset ( &l_addr_tcp48, 0, sizeof ( l_addr_tcp48 ) );
  
  m_sock_tcp48 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp48 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp48 = 1;
  if ( setsockopt ( m_sock_tcp48, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp48, sizeof ( on_tcp48 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp48.sin_family = AF_INET;
  l_addr_tcp48.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp48.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp48.sin_family = h_tcp48->h_addrtype;
  memcpy((char *) &m_addr_tcp48.sin_addr.s_addr, h_tcp48->h_addr_list[0], h_tcp48->h_length);
  m_addr_tcp48.sin_port = htons ( (unsigned short) TCP48_PORT );
  
  rc_tcp48 = bind ( m_sock_tcp48, ( struct sockaddr * ) &l_addr_tcp48, sizeof ( l_addr_tcp48 ) );
  
  if ( rc_tcp48 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp48 = connect ( m_sock_tcp48, ( struct sockaddr * ) &m_addr_tcp48, sizeof ( m_addr_tcp48 ) )) < 0) {
    if (debug) cout << "can not get tcp48 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }
  
  //prepare connection for tcp49 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp49, 0, sizeof ( m_addr_tcp49 ) );
  memset ( &l_addr_tcp49, 0, sizeof ( l_addr_tcp49 ) );
  
  m_sock_tcp49 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp49 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp49 = 1;
  if ( setsockopt ( m_sock_tcp49, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp49, sizeof ( on_tcp49 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp49.sin_family = AF_INET;
  l_addr_tcp49.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp49.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp49.sin_family = h_tcp49->h_addrtype;
  memcpy((char *) &m_addr_tcp49.sin_addr.s_addr, h_tcp49->h_addr_list[0], h_tcp49->h_length);
  m_addr_tcp49.sin_port = htons ( (unsigned short) TCP49_PORT );
  
  rc_tcp49 = bind ( m_sock_tcp49, ( struct sockaddr * ) &l_addr_tcp49, sizeof ( l_addr_tcp49 ) );
  
  if ( rc_tcp49 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp49 = connect ( m_sock_tcp49, ( struct sockaddr * ) &m_addr_tcp49, sizeof ( m_addr_tcp49 ) )) < 0) {
    if (debug) cout << "can not get tcp49 connection, sleep and retry." << endl;
    sleep(1);
    continue;
  }

  //prepare connection for tcp50 data
  //reuse the connection forever for now
  memset ( &m_addr_tcp50, 0, sizeof ( m_addr_tcp50 ) );
  memset ( &l_addr_tcp50, 0, sizeof ( l_addr_tcp50 ) );
  
  m_sock_tcp50 = socket ( AF_INET, SOCK_STREAM, 0 );
  if (m_sock_tcp50 < 0) {
    cout << "error creating socket" << endl;
    return -1;
  }
  
  on_tcp50 = 1;
  if ( setsockopt ( m_sock_tcp50, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on_tcp50, sizeof ( on_tcp50 ) ) < 0) {
    cout << "error setting socket options" << endl;
    return -1;
  }
  
  // local bind any port number
  l_addr_tcp50.sin_family = AF_INET;
  l_addr_tcp50.sin_addr.s_addr = htonl(INADDR_ANY);
  l_addr_tcp50.sin_port = htons((unsigned short)0);
  
  //remote server
  m_addr_tcp50.sin_family = h_tcp50->h_addrtype;
  memcpy((char *) &m_addr_tcp50.sin_addr.s_addr, h_tcp50->h_addr_list[0], h_tcp50->h_length);
  m_addr_tcp50.sin_port = htons ( (unsigned short) TCP50_PORT );
  
  rc_tcp50 = bind ( m_sock_tcp50, ( struct sockaddr * ) &l_addr_tcp50, sizeof ( l_addr_tcp50 ) );
  
  if ( rc_tcp50 < 0 ) {
    cout << "error binding local port for tcp data" << endl;
    return -1;
  }
  
  //connect to server, check if get connection, if not, sleep for a while before reconnect
  while ((rc_tcp50 = connect ( m_sock_tcp50, ( struct sockaddr * ) &m_addr_tcp50, sizeof ( m_addr_tcp50 ) )) < 0) {
    if (debug) cout << "can not get tcp50 connection, sleep and retry." << endl;
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
    if ((tv.tv_sec - start_tv.tv_sec) > 3600 
	|| tcp1_next >= tcp1_end || tcp2_next >= tcp2_end || tcp3_next >= tcp3_end 
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

	rc_tcp11 = send(m_sock_tcp11, buf, strlen(buf), 0);
	if(rc_tcp11<0) {
	  if (debug) cout << "error sending tcp11 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp21 = send(m_sock_tcp21, buf, strlen(buf), 0);
	if(rc_tcp21<0) {
	  if (debug) cout << "error sending tcp21 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp31 = send(m_sock_tcp31, buf, strlen(buf), 0);
	if(rc_tcp31<0) {
	  if (debug) cout << "error sending tcp31 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp41 = send(m_sock_tcp41, buf, strlen(buf), 0);
	if(rc_tcp41<0) {
	  if (debug) cout << "error sending tcp41 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp12 = send(m_sock_tcp12, buf, strlen(buf), 0);
	if(rc_tcp12<0) {
	  if (debug) cout << "error sending tcp12 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	

	rc_tcp22 = send(m_sock_tcp22, buf, strlen(buf), 0);
	if(rc_tcp22<0) {
	  if (debug) cout << "error sending tcp22 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp32 = send(m_sock_tcp32, buf, strlen(buf), 0);
	if(rc_tcp32<0) {
	  if (debug) cout << "error sending tcp32 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp42 = send(m_sock_tcp42, buf, strlen(buf), 0);
	if(rc_tcp42<0) {
	  if (debug) cout << "error sending tcp42 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp13 = send(m_sock_tcp13, buf, strlen(buf), 0);
	if(rc_tcp13<0) {
	  if (debug) cout << "error sending tcp13 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp23 = send(m_sock_tcp23, buf, strlen(buf), 0);
	if(rc_tcp23<0) {
	  if (debug) cout << "error sending tcp23 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp33 = send(m_sock_tcp33, buf, strlen(buf), 0);
	if(rc_tcp33<0) {
	  if (debug) cout << "error sending tcp33 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp43 = send(m_sock_tcp43, buf, strlen(buf), 0);
	if(rc_tcp43<0) {
	  if (debug) cout << "error sending tcp43 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp14 = send(m_sock_tcp14, buf, strlen(buf), 0);
	if(rc_tcp14<0) {
	  if (debug) cout << "error sending tcp14 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp24 = send(m_sock_tcp24, buf, strlen(buf), 0);
	if(rc_tcp24<0) {
	  if (debug) cout << "error sending tcp24 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp34 = send(m_sock_tcp34, buf, strlen(buf), 0);
	if(rc_tcp34<0) {
	  if (debug) cout << "error sending tcp34 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp44 = send(m_sock_tcp44, buf, strlen(buf), 0);
	if(rc_tcp44<0) {
	  if (debug) cout << "error sending tcp44 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp15 = send(m_sock_tcp15, buf, strlen(buf), 0);
	if(rc_tcp15<0) {
	  if (debug) cout << "error sending tcp15 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp25 = send(m_sock_tcp25, buf, strlen(buf), 0);
	if(rc_tcp25<0) {
	  if (debug) cout << "error sending tcp25 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp35 = send(m_sock_tcp35, buf, strlen(buf), 0);
	if(rc_tcp35<0) {
	  if (debug) cout << "error sending tcp35 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp45 = send(m_sock_tcp45, buf, strlen(buf), 0);
	if(rc_tcp45<0) {
	  if (debug) cout << "error sending tcp45 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp16 = send(m_sock_tcp16, buf, strlen(buf), 0);
	if(rc_tcp16<0) {
	  if (debug) cout << "error sending tcp16 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp26 = send(m_sock_tcp26, buf, strlen(buf), 0);
	if(rc_tcp26<0) {
	  if (debug) cout << "error sending tcp26 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp36 = send(m_sock_tcp36, buf, strlen(buf), 0);
	if(rc_tcp36<0) {
	  if (debug) cout << "error sending tcp36 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp46 = send(m_sock_tcp46, buf, strlen(buf), 0);
	if(rc_tcp46<0) {
	  if (debug) cout << "error sending tcp46 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp17 = send(m_sock_tcp17, buf, strlen(buf), 0);
	if(rc_tcp17<0) {
	  if (debug) cout << "error sending tcp17 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp27 = send(m_sock_tcp27, buf, strlen(buf), 0);
	if(rc_tcp27<0) {
	  if (debug) cout << "error sending tcp27 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp37 = send(m_sock_tcp37, buf, strlen(buf), 0);
	if(rc_tcp37<0) {
	  if (debug) cout << "error sending tcp37 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp47 = send(m_sock_tcp47, buf, strlen(buf), 0);
	if(rc_tcp47<0) {
	  if (debug) cout << "error sending tcp47 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp18 = send(m_sock_tcp18, buf, strlen(buf), 0);
	if(rc_tcp18<0) {
	  if (debug) cout << "error sending tcp18 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp28 = send(m_sock_tcp28, buf, strlen(buf), 0);
	if(rc_tcp28<0) {
	  if (debug) cout << "error sending tcp28 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp38 = send(m_sock_tcp38, buf, strlen(buf), 0);
	if(rc_tcp38<0) {
	  if (debug) cout << "error sending tcp38 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp48 = send(m_sock_tcp48, buf, strlen(buf), 0);
	if(rc_tcp48<0) {
	  if (debug) cout << "error sending tcp48 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	

	rc_tcp10 = send(m_sock_tcp10, buf, strlen(buf), 0);
	if(rc_tcp10<0) {
	  if (debug) cout << "error sending tcp10 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp20 = send(m_sock_tcp20, buf, strlen(buf), 0);
	if(rc_tcp20<0) {
	  if (debug) cout << "error sending tcp20 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp30 = send(m_sock_tcp30, buf, strlen(buf), 0);
	if(rc_tcp30<0) {
	  if (debug) cout << "error sending tcp30 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp40 = send(m_sock_tcp40, buf, strlen(buf), 0);
	if(rc_tcp40<0) {
	  if (debug) cout << "error sending tcp40 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp50 = send(m_sock_tcp50, buf, strlen(buf), 0);
	if(rc_tcp50<0) {
	  if (debug) cout << "error sending tcp50 data, retry later" << endl;
	  sleep(1);
	  continue;
	} else {
	  cnt++;
	  sec_cnt++;
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

	rc_tcp19 = send(m_sock_tcp19, buf, strlen(buf), 0);
	if(rc_tcp19<0) {
	  if (debug) cout << "error sending tcp19 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp29 = send(m_sock_tcp29, buf, strlen(buf), 0);
	if(rc_tcp29<0) {
	  if (debug) cout << "error sending tcp29 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp39 = send(m_sock_tcp39, buf, strlen(buf), 0);
	if(rc_tcp39<0) {
	  if (debug) cout << "error sending tcp39 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	
	rc_tcp49 = send(m_sock_tcp49, buf, strlen(buf), 0);
	if(rc_tcp49<0) {
	  if (debug) cout << "error sending tcp49 data, retry later" << endl;
	  sleep(1);
	  continue;
	}  else {
	  cnt++;
	  sec_cnt++;
	}	

      }

      //cout << diff2 << "\t" << diff1 << "\t" << tv_sec_pac+(double)tv_usec_pac/1000000 << "\t" << tv_sec1+(double)tv_usec1/1000000 << endl;
      
    }    

  } //end while(1)
  
  return 0;                     // Got data
}
