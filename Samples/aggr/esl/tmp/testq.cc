extern "C"{
#include <im_db.h>
}
#include <rtree.h>

#include <compiler.h>
#include "buffer.h"
#include "dbt.h"
#include <iostream>
#include <dlfcn.h>
#include <sys/time.h>
#include <string>
#include <driver.h>
#include <querySchdl.h>

using namespace ESL;
using namespace std;


FILE* stdoutLog=NULL;
FILE* stderrLog=NULL;
char logFileName[100];


#include <getopt.h>

extern "C"{

#include <dbug.h>
}
static const char *default_dbug_option="d:t:i";
static struct option long_options[] =
{
#ifndef DBUG_OFF
  {"debug",	optional_argument, 0, '#'},
#endif
  {"help",	no_argument,	   0, '?'},
  {"db2",	no_argument,	   0, 'd'},
  {"file",	required_argument, 0, 'f'},
  {"version",	no_argument,	   0, 'V'},
  {"verbose",	no_argument,	   0, 'v'},
  {0, 0, 0, 0}
};

#ifndef DBUG_OFF
extern	int yydebug;
#endif
static void usage(char *program_name)
{
  fprintf(stderr, "Usage: %s <options> <source files>\n", program_name);
  fprintf(stderr, "\nwhere options includes:\n"
	  "\t-h --help         this message\n"
	  "\t-V --version      version info\n"
	  "\t-v --verbose      verbose mode\n"
#ifndef DBUG_OFF
	  "\t-# --debug[=...]  debug option. Default is '%s'\n",
	  default_dbug_option
#endif
	  );
}
bufferMngr *bm;
querySchdl *qs = querySchdl::getInstance();
int main(int argc, char **argv){
  int c, option_index = 0;
  while ((c=getopt_long(argc,argv,"hf:h#yV:vcu",
			long_options, &option_index)) != -1) {
    switch (c) {
#ifndef DBUG_OFF
    case '#':
      DBUG_PUSH(optarg ? optarg : default_dbug_option);
       break;
#endif
    case 'V':
      break;
    case 'h':
    default:
      //usage (program_name);
      exit(1);
    }
  }
    
  DBUG_EXECUTE("yydebug", yydebug=1;);

  if (optind >= argc) {
    usage(argv[0]);
    exit(1);
  }


  //compiler* cc = compiler::getInstance();
  //cc->compile(argv[optind]);  
  

  void *handle;
  int (*func)(bufferMngr*, int);
  string so_name("../exe/");
  so_name += argv[optind];
  so_name += ".so";
  if (!(handle = dlopen (so_name.c_str(), RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((func) = (int (*)(bufferMngr *, int))dlsym(handle, argv[optind]))) {
    fprintf(stderr, "dlsym: %s\n", dlerror());
    exit(1);
  }

  sDBT::createSM();
  bm = bufferMngr::getInstance();
  bm->create("bids");
  bm->create("stdout", SHARED);
  bm->create("_ioBuffer", SHARED);
  bm->create("_queryBuffer", SHARED);

  bm->create("items", BUF_BTREE);
  timestamp tv;
  gettimeofday(&tv,NULL);
  cDBT d(8, &tv);
  int j = 2;
  int i=1234;
  memcpy(d.data, &j, sizeof(int));
  memcpy(d.data+sizeof(int), &i, sizeof(int));
  bm->put("bids", &d);

  j = 1;
  i = 4567;
  memcpy(d.data, &j, sizeof(int));
  memcpy(d.data+sizeof(int), &i, sizeof(int));
  bm->put("bids", &d);
  cout<<func(bm, 0)<<endl;
  cout<<func(bm, 0)<<endl;
  sDBT::destroySM();
  
}
