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

#include "../dbt.h"

#define MAX_BUFFER 1000    //Also denotes query size for query modules

#define PORT 5432

extern "C" int getTuple(dbt* data);

int fdesc = -1;
int listensockfd = -1;
struct sockaddr_in listensock;

int init()
{
  int p;                /*general purpose    */
  int op;

  listensock.sin_family=AF_INET;
  listensock.sin_port=htons((unsigned short)PORT);
  listensock.sin_addr.s_addr=INADDR_ANY;

  /* create socket, terminate in
     case of failure */
  if((listensockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    perror("Error calling socket()");
    return -1;
  }

  if(bind(listensockfd, (struct sockaddr *)&listensock, sizeof(listensock)))
  {
    perror("Error calling bind()");
    return -1;
  }

  /* make socket listening for
   connections, terminate in
   case of failure */
  if(listen(listensockfd, 1))
  {
    perror("Error calling listen()");
    return -1;
  }

  p=sizeof(listensock);
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

void processMessage(dbt* data, char* buf)
{
  int numberOfColumns = 1;
  char dat[MAX_BUFFER];

  memcpy(dat, buf, strlen(buf));

  data->data = dat;
  data->datasz = strlen(buf);

  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  data->setTime(&tv);
}

int getTuple(dbt* data)
{
  int rc;

  printf("In getTuple for default query mod\n");

  if(fdesc < 0 && listensockfd < 0)
    rc = init();

  if(rc == -1)
    return -1;			// Error establishing connection

  if(fdesc < 0)
    tryAccept();

  if(fdesc < 0)
  {
    return 2;			//No data
  }
  char buf[MAX_BUFFER];

  int olen = read(fdesc, buf, sizeof(buf));

  if (olen == -1 && errno == EAGAIN)
    return 2;			// No data

  if(olen == 0)
    return 1;			// Connection Closed

  buf[olen] = '\0';
  processMessage(data, buf);

  return 0;			// Got data
}

int closeSocket()
{
  close(fdesc);
  return 0;
}
