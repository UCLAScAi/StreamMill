#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

// #include <sys/socket.h>
// #include <netinet/in.h>
#include <stdlib.h>

extern "C" { 
#include <dbug/dbug.h>
}
#include <sql/basic.h>

#include <sql/adl_sys.h>
#include <sql/symbol.h>
#include <sql/absyn.h>
#include <sql/semant.h>
#include <sql/trans2C.h>
#include <sql/env.h>
#include <sql/err.h>

extern int sqlInitScanner(char *buf);
extern int yyparse();

/************************************************************
  OPTIONS
 ************************************************************/
#include <sql/getopt.h>

extern char *optarg;
extern int optind;
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

/************************************************************

 ************************************************************/
#ifndef DBUG_OFF
extern	int yydebug;
#endif

/* The following global variables are defined in nt_sys.cc */
extern system_t *ntsys;
extern S_table E_base_tenv;
extern S_table E_base_venv;

extern A_exp abs_adl ;  //define in ../esl/compiler.cc

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s <options> <source files>\n", program_name);
	fprintf(stderr, "\nwhere options includes:\n"
			"\t-h --help         this message\n"
			"\t-f --file=...     output .cc file\n"
			"\t-m --userName     only used for esl\n"
			"\t-d --db2          generate UDF for use with db2\n"
			"\t-V --version      version info\n"
			"\t-v --verbose      verbose mode\n"
#ifndef DBUG_OFF
			"\t-# --debug[=...]  debug option. Default is '%s'\n",
			default_dbug_option
#endif
		   );
}

void genOutputFilename(char* in, char *out)
{
	int len = strlen(in);

	if (len>4 && strcasecmp(in+len-4, ".adl") == 0) {
		strncpy(out, in, len-4);
		out[len-4]='\0';
	} else {
		strcpy(out, in);
	}
	strcat(out, ".cc");    
}
int main(int argc, char*argv[])
{
	err_t ERR_NONE;
	char program_dir[120];
	char *program_name;
	char output_file[120];
	char userName[120];
	userName[0] = '\0';
	char *buf, *p;
	int c, option_index=0;
	int verbose=0;
	int db2=0;
	int rc;

	setQueryName(""); // identifying Atlas, not ESL.  see comments in semant.h

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
	while ((c=getopt_long(argc,argv,"hf:m:h#yV:vcu",
					long_options, &option_index)) != -1) {
		switch (c) {
			case 'd':
				db2 = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'm':
				strcpy(userName, optarg);
				break;
			case 'f':
				strcpy(output_file, optarg);
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
		usage(program_name);
	}

	while (optind < argc) {

		ntSysInit(program_dir, program_name);
		ntsys->verbose = verbose;
		ntsys->db2 = db2;

		if(userName[0] == '\0')
			setUserName("__user__");
		else
			setUserName(userName);

		rc = readFileIntoBuffer(argv[optind], buf);
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "main", "readFileIntoBuffer");
			goto exit;
		}

		/* Lexical Analysis */
		sqlInitScanner(buf);

		/* Parsing */
		rc = yyparse();

		if (abs_adl != (A_exp)0) {
			/* Semantic Analysis & Code Generation */
			if (*output_file == '\0')
				genOutputFilename(argv[optind], output_file);
			rc = trans2C(E_base_venv, E_base_tenv, abs_adl, output_file);
		}

		ntSysQuit();

		optind++;
		free(buf);
	}

exit:
	return rc;
}






