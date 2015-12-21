#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sql/adl_obj.h>
#include <sql/adl_sys.h>
#include <sql/err.h>
#include "const.h"
#include <string.h>

err_t readFileIntoBuffer(char* filename, char *&buf)
{
  err_t rc = ERR_NONE;
  FILE *in;
  char tmp[4096];
  int n;
  int capacity=0;
  int size=0;
    
  if (!(in = fopen(filename, "rt")) ) {
    rc = ERR_OPEN_FILE;
    EM_error(0, rc, __LINE__, __FILE__, filename);
    goto exit;
  } 

  buf = (char*)0;
  while ( (n=fread(tmp, 1, 4096, in)) > 0) {
    if (size+n > capacity) {
      capacity+=4096;
      buf = (char *)realloc(buf, capacity+1);
    }
    strncpy(buf+size, tmp, n);
    size+=n;
  }
  if (size>0)
    buf[size]='\0';
        
  fclose(in);

 exit:
  return rc;
}

/*************************************************************
  A stream is either a FILE or a  STRING.
  FILE   :  type is O_NUM, 
	    str  is filename, 
	    opt  is BINARY_DISPLAY, INPUT_STREAM
  STRING :  type is O_STRING, 
	    str  is allocated buffer, 
	    opt  is BINARY_DISPLAY
*************************************************************/
nt_obj_t* makeStream(int type, char *str, int opt)
{
  nt_obj_t *stream = (nt_obj_t*)0;
  //  int fd;
  FILE *fd;

  switch (type) {
  case O_NUM:
    fd = fopen(str, (opt & INPUT_STREAM)? "rb":"w+b");
    /*    fd = open(str, (opt & INPUT_STREAM)? 
	  O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC, 0644);*/
    
    if (fd) {
      stream = newNumObj((int)fd);
      stream->flag |= opt;
    }
    break;
  case O_STRING:
    stream = newStrObj( str);
    stream->flag |= opt;
    break;
  }
  return stream;
}
void closeStream(nt_obj_t *stream)
{
  if (stream->type == O_NUM)
    fclose((FILE*)NumObj(stream));
  deleteObj(stream);
}
void seekStream(nt_obj_t *stream, int delta)
{
  switch (stream->type) {
  case O_NUM:
    //lseek(NumObj(stream), delta, SEEK_CUR);
    fseek((FILE*)NumObj(stream), delta, SEEK_CUR);
    break;
  case O_STRING:
    stream->value += delta;
    break;
  }
}
/************************************************************
                      READING 
************************************************************/
int ntRead(void *n, nt_obj_t *stream, int len)
{
  switch (stream->type) {
  case O_NUM:
    //    return read(NumObj(stream), n, len);
    return fread(n, 1, len, (FILE*)NumObj(stream));
  case O_STRING:
    memcpy(n, StrObj(stream), len);
    stream->value +=len;
    return len;
  }
}
int ntReadNum(nt_obj_t *stream)
{
  int n;
  ntRead((void*)&n, stream, sizeof(int));
  return n;
}
float ntReadRealNum(nt_obj_t *stream)
{
  float f;
  ntRead((void*)&f, stream, sizeof(float));
  return f;
}
char *ntReadStr(nt_obj_t *stream, int len)
{
  char *n = (char*)ntMalloc(len);
  ntRead(n, stream, len);
  return n;
}

/*
void sscanfObj(char *s, char *fmt, nt_obj_t *stream, ...)
{
  char *p;
  va_list ap;

  va_start(ap, stream);
  for (p=fmt; *p; p++) {
    switch (*p) {
    case 'o':
      (nt_obj_t*)s = readObj(stream);
      break;
    case 's':
      s = ntReadStr(stream, va_arg(ap, int));
      break;
    case 'd':
      *(int*)s = ntReadNum(stream);
      break;
    }
    s=s+4;
  }
  va_end(ap);
}
*/
/************************************************************
                      WRITING 
************************************************************/
void ntWrite(nt_obj_t* stream, void *buf, int size) 
{
  switch (stream->type)    {
  case O_NUM:			/* file */
    //    write(NumObj(stream), buf, size);
    fwrite(buf, 1, size, (FILE*)NumObj(stream));
    break;
  case O_STRING:		/* dynamic string */
    memcpy(StrObj(stream), buf, size);
    stream->value += size;
    break;
  default:
    break;
  }
}
void ntWriteStr(nt_obj_t* stream, char *src)
{
  ntWrite(stream, (void*)src, strlen(src));
}

static void ntvPrintf(nt_obj_t* stream, va_list ap, char *fmt, int indent)
{
  char *p;
  int len;
  char buf[100];

  for (p=fmt; *p; p++) {
    if (*p=='%' && *(++p) && *p != '%') {
      switch (*p) {
      case 'x':
	{
	  void *p = va_arg(ap, void*);
	  sprintf(buf, "%x", p);
	  ntWrite(stream, buf, strlen(buf));
	}
	break;
      case 'f':
	{
	  double num = va_arg(ap,double);
	  if (stream->flag & BINARY_STREAM) 
	    ntWrite(stream, &num, sizeof(double));
	  else {
	    sprintf(buf, "%f", num);
	    ntWrite(stream, buf, strlen(buf));
	  }
	}
	break;
      case 'o':
	displayObj(stream, va_arg(ap, nt_obj_t*));
	break;
      case 's':
	{
	  char *str = va_arg(ap,char*);
	  ntWrite(stream, str, strlen(str));
	}
	break;
      case 'd': 
	{
	  int num = va_arg(ap,int);
	  if (stream->flag & BINARY_STREAM) 
	    ntWrite(stream, &num, sizeof(int));
	  else {
	    sprintf(buf, "%d", num);
	    ntWrite(stream, buf, strlen(buf));
	  }
	}
	break;
      case '*':
	switch (*++p) {
	case 's':		/* fixed size string */
	  { 
	    int len = va_arg(ap,int);
	    ntWrite(stream, va_arg(ap, char*), len);
	  }
	break;
	case 'l':		/* deliminated list */
	  {
	    char *del = va_arg(ap, char*);
	    A_list list = va_arg(ap, A_list);
	    displayList(stream, list, del);
	  }
	break;
	case 'd':		/* fixed size num */
	  {
	    int len = va_arg(ap, int);
	    sprintf(buf, "%*d", len, va_arg(ap, int));
	    ntWrite(stream, buf, len);
	  }
	  break;
	}
	break;
      }
    } else {
      ntWrite(stream, p, 1);
      if (*p=='\n') {
	for (int i=0; i<indent; i++)
	  ntPrintf(stream, "  ");
      }
    }
  }
}

/*************************************************************
ntPrintf format:

	%o	nt_obj_t
	%s	string without leading blanks
	%*l	list obj
*************************************************************/
void ntIndentPrintf(nt_obj_t *stream, int indent, char *fmt, ...)
{
  va_list ap;
  char newfmt[MAX_STR_LEN], buf[MAX_STR_LEN];
  char *s=fmt, *t=newfmt;

  va_start(ap, fmt);
  while ((*t++=*s++)) {
    if (*(s-1)=='\n') {
      for (int i=0; i<indent; i++) {
	*t++=' '; *t++=' '; *t++=' ';
      }
    }
  }
  vsprintf(buf, newfmt, ap);
  ntWrite(stream, buf, strlen(buf));
  va_end(ap);
}
void ntPrintf(nt_obj_t* stream, char *fmt, ...)
{
  char buf[4096];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(buf, fmt, ap); 
  ntWrite(stream, buf, strlen(buf));
//    ntvPrintf(stream, ap, fmt, 0);
  va_end(ap);
}





