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

#define NUM_PERSON_SEC 5/100
#define PERSON_PORT 5556
#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"

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

static struct timeval last_person_tv;
static struct timeval first_person_tv;
static int next_person_id = 0;
static bool first_person = true;

static person_t persons[500];

static bool loaded = false;

char* PERSON_SERVER = "nile.cs.ucla.edu";

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
  cout << "start sending person data" << endl;

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

  struct hostent *h;
  h = gethostbyname(PERSON_SERVER);
  if(h==NULL) {
    printf("unknown host '%s'\n", PERSON_SERVER);
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
    m_addr.sin_port = htons ( (unsigned short) PERSON_PORT );

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
    
    if (first_person) {
      totalNum = 3;
      gettimeofday(&last_person_tv, &tz);
      gettimeofday(&first_person_tv, &tz);
      first_person = false;
    } else {
      gettimeofday(&tv, &tz);    
      totalNum = (tv.tv_sec - last_person_tv.tv_sec)*NUM_PERSON_SEC;
    }

    //restart if more than 2000 sec has elapsed
    //for now, only run for 500 persons, and start over again.
    if ((tv.tv_sec - first_person_tv.tv_sec) > 2000 || (next_person_id) >= 500) {
      first_person = true;
      next_person_id = 0;
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
      
      int id = next_person_id++;
      //memcpy(data.data, (char*)&(persons[id].id), sizeof(int)); //id
      sprintf(buf, "%d,",  persons[id].id);
      
      //strcpy(data.data+sizeof(int), persons[id].name); //name
      sprintf(temp, "%s,",  persons[id].name);
      strcat(buf, temp);
      
      //strcpy(data.data+sizeof(int)+20, persons[id].email); //email
      sprintf(temp, "%s,",  persons[id].email);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+40, (char*)&persons[id].creditCard, sizeof(int)); //credit_card
      sprintf(temp, "%d,",  persons[id].creditCard);
      strcat(buf, temp);
      
      //strcpy(data.data+sizeof(int)+40+sizeof(int), persons[id].city); //city
      sprintf(temp, "%s,",  persons[id].city);
      strcat(buf, temp);
      
      //strcpy(data.data+sizeof(int)+40+sizeof(int)+20, persons[id].state); //state
      sprintf(temp, "%s,",  persons[id].state);
      strcat(buf, temp);
      
      //memcpy(data.data+sizeof(int)+100+sizeof(int)+22, (char*)&tv, sizeof(struct timeval)); //time
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
    
    gettimeofday(&last_person_tv, &tz);
  
  }
  
  return 0;                     // Got data
}
