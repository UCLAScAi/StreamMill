#ifndef __QUERYSCHDL_H__
#define __QUERYSCHDL_H__

//#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include <stdlib.h>


//#include <basic.h>

#include <adl_sys.h>
#include <symbol.h>
#include <absyn.h>
#include <semant.h>
#include <trans2C.h>
#include <env.h>
#include <err.h>
#include <stmt.h>
#include <driver.h>

#define MAX_MSG_LEN 2048
#define MAX_BUF_COUNT 25


//Hamid: for logging system
#include <stdarg.h>
#define SMLOG_LEVEL 10

using namespace std;
using namespace __gnu_cxx;

namespace ESL{

	//performance buffer names

	static const char* TOTAL_OUTPUT_TUPLE = "total_output_tuple";
	static const char* AVG_LATENCY = "avg_latency";
	static const char* MAX_LATENCY = "max_latency";
	static const char* LAST_TOTAL_BUF_BYTES = "last_total_buf_bytes";

	typedef struct stack_item_t
	{
		int level;
		b_entry entry;
		stmt* statement;
	} stack_item;

	class querySchdl{
		DrvMgr *dm;

		char* qsLog;
		FILE* stdoutLog;
		FILE* stderrLog;

		public:
		stmt *s;  // to hold return values from Atlas compiler

		static querySchdl *getInstance();

		int run();

		static int verbose;
		void setVerbose(int verbose)  { this->verbose = verbose; }

		bool bufferInUse(char* bname);
		bool stmtInUse(char* name);

		//Added By Hamid for better logging
		// 7/9/2009 
		//FIXME: move it to a new class later
		static void SMLOG(int level, const char* format, ...);
		

		private:
		querySchdl();
		int init();
		static int destroy();

		int compile(char* id);
		run_rc runDriver(Driver* drv);

		list<stmt*> freeStmts;
		static querySchdl *_instance;

		//below for initial tests only
		int simpleTest();
		int forkTest();
		int tUnionTest();
		int tlUnionTest();
		int splitTest();
		int compTest(); 

		// handlers for different command
		// these functions are added by Hamid to break the run function
		// 7/9/2009
		void SNAP_SHOT_CMD_CODE_hndl		(bufferMngr* bm, char* id );
		void ADD_QUERY_CMD_CODE_hndl		(bufferMngr* bm, char* id );
		void ADD_DECLARE_CMD_CODE_hndl		(bufferMngr* bm, buffer* qBuf, char * id, int code );
		void ACTIVATE_QUERY_CMD_CODE_hndl	(bufferMngr* bm, char* id );
		void DEACTIVATE_QUERY_CMD_CODE_hndl	(bufferMngr* bm, char* id );
		void DROP_QUERY_CMD_CODE_hndl		(bufferMngr* bm, char* id );
		void DROP_DECLARE_CMD_CODE_hndl		(bufferMngr* bm, char* id );
		void GET_COMPONENTS_hndl		(bufferMngr* bm, char* id );
		void VIEW_COMPONENT_DETAILS_hndl	(bufferMngr* bm, char* id );
		void SET_COMPONENT_PRIORITY_hndl	(bufferMngr* bm, char* id, char* id2 );
		void MOVE_STMT_TO_COMPONENT_hndl	(bufferMngr* bm, char* id, char* id2, char* id3 );
		void BREAK_COMPONENT_hndl		(bufferMngr* bm, char* id, char* id2 );
		void JOIN_COMPONENTS_hndl		(bufferMngr* bm, char* id, char* id2 );
		void MONITOR_PERFORMANCE_BUFFER_hndl	(bufferMngr* bm, char* id );
		void UNMONITOR_PERFORMANCE_BUFFER_hndl	(bufferMngr* bm, char* id ); 




	};

} // end of namespace ESL
#endif
