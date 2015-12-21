#ifndef NT_IO_H
#define NT_IO_H

#include <adl_obj.h>
#include <err.h>

err_t readFileIntoBuffer(char *filename, char*&buf);
nt_obj_t* makeStream(int type, char *name, int opt); 
void closeStream(nt_obj_t *stream);
void seekStream(nt_obj_t *stream, int delta);
/************************************************************
                      READING 
*************************************************************/
int ntRead(void *obj, nt_obj_t *stream, int len);
int ntReadNum(nt_obj_t *stream);
float ntReadRealNum(nt_obj_t *stream);
char *ntReadStr(nt_obj_t *stream, int len);

/************************************************************
                      WRITING 
************************************************************/
void ntWrite(nt_obj_t* obj, void *buf, int size); 
void ntWriteStr(nt_obj_t* stream, char *src);
void ntPrintf(nt_obj_t *stream, char *fmt, ...);
void ntIndentPrintf(nt_obj_t *stream, int indent, char *fmt, ...);
#define ntWriteNum(s, n) { int i=n; ntWrite(s, (char*)(&i), sizeof(int));}

#endif



