#include <winbuf.h>
#include <iostream>
#include <driver.h>
#include "querySchdl.h"
#include "ios/ios.h"
#include "buffer.h"

extern "C"{
#include <mm.h>
#include "dbug.h"
}
#include <getopt.h>

using namespace ESL;
using namespace std;

// ESL Main program

bufferMngr *bm;

static const char *default_dbug_option="d:t:i";
static struct option long_options[] =
{
#ifndef DBUG_OFF
  {"debug",	optional_argument, 0, '#'},
#endif
  {"help",	no_argument,	   0, '?'},
  {"db2",	no_argument,	   0, 'd'},
  {"file",	required_argument, 0, 'f'},
  {"port",	required_argument, 0, 'p'},
  {"version",	no_argument,	   0, 'V'},
  {"verbose",	no_argument,	   0, 'v'},
  {0, 0, 0, 0}
};


#ifndef DBUG_OFF
extern	int yydebug;
#endif
//  DBUG_EXECUTE("yydebug", yydebug=1;);
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

querySchdl *qs=querySchdl::getInstance();
int verbose=0;

int main(int argc, char**argv){
  err_t ERR_NONE;
  char program_dir[120];
  char *program_name;
  char output_file[120];
  char *buf, *p;
  int c, option_index=0;
  int db2=0;
  int rc = s_success;
  int port = 5430;

  //yydebug = 1;
  cout<<"ESL started!\n";
  /* get option */
  strcpy(program_dir, argv[0]); 
  if ((p = strrchr(program_dir, '/'))) {
    program_name = p+1;
    *p='\0';
  } else {
    program_name = argv[0];
    strcpy(program_dir,".");
  }

  *output_file = '\0';

  while ((c=getopt_long(argc,argv,"hfp:h#yV:vcu",
			long_options, &option_index)) != -1) {
    switch (c) {
    case 'd':
      db2 = 1;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'f':
      strcpy(output_file, optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
#ifndef DBUG_OFF
    case '#':
      DBUG_PUSH(optarg ? optarg : default_dbug_option);
       break;
#endif
    case 'V':
      break;
    case 'h':
    default:
      usage (program_name);
      exit(1);
    }
  }
    
  DBUG_EXECUTE("yydebug", yydebug=1;);

  if (optind >= argc) {
    //usage(program_name);
  }

  //DBUG_PUSH(default_dbug_option);
  sDBT::createSM();
  //  sharedBuf::createSM();
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  printf("System started at %s\n", ctime(&tv.tv_sec));
  bm= bufferMngr::getInstance();
  bm->create("_queryBuffer",
	     SHARED);
  bm->create("_ioBuffer",
	     SHARED);
  Ios* ios = Ios::getInstance();

  tempdb_init();
  hashgb_init();
  //_adl_dlm_init();

  if (fork()){ // Query Schduler
    cout<<"Query Scheduler"<<endl;
    qs->setVerbose(verbose);
    qs->run();
    sleep(10);
    sDBT::destroySM();
  }
  else { // I/O Scheduler
    sleep(1);
    cout<<"I/O Scheduler"<<endl;
    ios->run(port);
    
  }
  return 0;

};
