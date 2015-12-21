/* eslc.cc -- ESL compiler for debug purpose
 *
 * Web Information System Lab (http://wis.cs.ucla.edu)
 * Computer Science Dept.
 * University of California, Los Angeles
 * 2004/2
 */


#include <iostream>
#include <driver.h>
#include "querySchdl.h"
//#include "ios.h"
#include "buffer.h"
extern "C"{
#include <mm.h>
#include "dbug.h"
}
#include <getopt.h>
#include <compiler.h>
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
  {"aggr",	no_argument,	   0, 'a'},
  {"help",	no_argument,	   0, '?'},
  {"db2",	no_argument,	   0, 'd'},
  {"file",	required_argument, 0, 'f'},
  {"port",	required_argument, 0, 'p'},
  {"snapshot",	no_argument,	   0, 's'},
  {"user",      required_argument, 0, 'u'},
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

DrvMgr *dm;

int main(int argc, char**argv){
  err_t ERR_NONE;
  char program_dir[120];
  char *program_name;
  char output_file[120];
  char *buf, *p;
  int c, option_index=0;
  int verbose=0;
  int db2=0;
  int rc;
  compile_opt opt = cmp_simple;
  int port;

  //yydebug = 1;
  cout<<"Compiling...\n";
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

  while ((c=getopt_long(argc,argv,"hsf:hu:h#ayV:vcu",
			long_options, &option_index)) != -1) {
    switch (c) {
    case 'a':
      opt = cmp_aggr;
      break;
    case 's':
      opt = cmp_snapshot;
      break;
    case 'd':
      db2 = 1;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'f':
      strcpy(output_file, optarg);
      break;
    case 'u':
      setUserName(strdup(optarg));
      break;
    case 'p':
      port = atoi(optarg);
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
    usage(program_name);
    exit(1);
  }

  //DBUG_PUSH(default_dbug_option);
  sDBT::createSM();
  bm = bufferMngr::getInstance();
  bm->create("stdout", SHARED);
  bm->create("_ioBuffer", SHARED);
  dm = DrvMgr::getInstance();
  compiler *cc=compiler::getInstance();
  //printf("Calling compile with %s %d\n", argv[optind], opt);
  rc = cc->compile(argv[optind], opt);
  sDBT::destroySM();
  cout<<"Done!\n";
  return rc;
};


































