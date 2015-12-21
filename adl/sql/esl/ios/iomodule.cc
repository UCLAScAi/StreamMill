#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <dbt.h>
#include <buffer.h>
using namespace ESL;

#define MAX_BUFFER 65536

extern "C" int getTuple(buffer* dest);
extern "C" int closeConnection();

int fdesc = -1;
int listensockfd = -1;
struct sockaddr_in listensock;

int init()
{
  int p;                /*general purpose    */
  int op;

  listensock.sin_family=AF_INET;
  listensock.sin_port=htons((unsigned short)5533);
  listensock.sin_addr.s_addr=INADDR_ANY;

  /* create socket, terminate in
     case of failure */
  printf("In getTuple calling socket\n");
  if((listensockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    perror("Error calling socket()");
    return -1;
  }

  printf("In getTuple calling bind\n");
  if(bind(listensockfd, (struct sockaddr *)&listensock, sizeof(listensock)))
  {
    perror("Error calling bind()");
    return -1;
  }

  /* make socket listening for
   connections, terminate in
   case of failure */
  printf("In getTuple calling listen\n");
  if(listen(listensockfd, 1))
  {
    perror("Error calling listen()");
    return -1;
  }

  op = fcntl(listensockfd, F_GETFL, 0);
  if (op != -1) {
    op |= O_NONBLOCK;
    fcntl(listensockfd, F_SETFL, op);
  }

  p=sizeof(listensock);
  printf("In getTuple calling accept\n");
  if((fdesc=accept(listensockfd, (struct sockaddr *)&listensock, (socklen_t *)&p)) == -1)
  {
    return 1;
  }

  op = fcntl(fdesc, F_GETFL, 0);
  if (op != -1) {
    op |= O_NONBLOCK;
    fcntl(fdesc, F_SETFL, op);
  }

  return 0;
}

int tryAccept()
{
  int p, op;

  p = sizeof(listensock);
  if((fdesc=accept(listensockfd, (struct sockaddr *)&listensock, (socklen_t *)&p)) == -1)
  {
    return 1;
  }

  op = fcntl(fdesc, F_GETFL, 0);
  if (op != -1) {
    op |= O_NONBLOCK;
    fcntl(fdesc, F_SETFL, op);
  }

  return 0;
}

void stringPad(char* src, char* dest)
{
  int strlength = strlen(src);
  int i = 0;
  for(i; i < strlength; i++)
  {
    dest[i] = src[i];
  }
  for(i; i < 9; i++)
  {
    dest[i] = ' ';
  }
  dest[9] = '\0';
}

void processMessage(dbt* data, char* buf)
{
  int numberOfColumns = 3;
  char* val;
  char val1[50];
  int curColumn = 0;
  int offset = 0;
  int pad = 0;
  int temp;

  val = (char*)strtok(buf, ",");
  temp = atoi(val);
  memcpy(data->data, (char*)&temp, sizeof(int));
  val = strtok(NULL, ",");
  stringPad(val, val1);
  strcpy(data->data+sizeof(int), val1);
  val = strtok(NULL, ",");
  temp = atoi(val);
  memcpy(data->data+sizeof(int)+10, (char*)&temp, sizeof(int));

  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  memcpy(data->data+sizeof(int)+10+sizeof(int), (char*)&tv, sizeof(struct timeval));

  data->setTime(&tv);
}


void putDataInBuffer(char* dataBuf, buffer* dest)
{
  char *tupleStr = NULL;
  struct timeval tv;

  char* dataCopy = strdup(dataBuf);

  tupleStr = strsep(&dataCopy, "\n");
  while(tupleStr != NULL && strlen(tupleStr) > 0)
  {
    cDBT tuple(500, &tv);
    int tupleStrSize = strlen(tupleStr);
    processMessage(&tuple, tupleStr);
    dest->put(&tuple);
    tupleStr = strsep(&dataCopy, "\n");
  }
}

int getTuple(buffer* dest)
{
  char buf[MAX_BUFFER];
  int rc;
  
  printf("In getTuple\n");

  if(fdesc < 0 && listensockfd < 0)
    rc = init();

  if(rc == -1)
    return -1;                  // Error establishing connection

  printf("In getTuple after init\n");

  if(fdesc < 0)
    tryAccept();
  printf("In getTuple after tryAccept\n");

  if(fdesc < 0)
  {
    return 2;                   //No data
  }


  printf("In gettuple calling read\n");
  int olen = read(fdesc, buf, sizeof(buf));

  if (olen == -1 && errno == EAGAIN)
    return 2;                   // No data

  if(olen == 0)
  {
    return 1;                   // Connection Closed
  }

  buf[olen] = '\0';
  putDataInBuffer(buf, dest);

  return 0;                     // Got data
}

int closeConnection()
{
  printf("Closing connection %d, %d\n", fdesc, listensockfd);
  if(fdesc >0)
    close(fdesc);
  if(listensockfd >0)
    close(listensockfd);
  fdesc = -1;
  listensockfd = -1;
  return 0;
}

