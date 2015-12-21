#ifndef __SMLOG_H__
#define __SMLOG_H__

//#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include <stdlib.h>


//#include <basic.h>

//#include <adl_sys.h>
//#include <symbol.h>
//#include <absyn.h>
//#include <semant.h>
//#include <trans2C.h>
//#include <env.h>
//#include <err.h>
//#include <stmt.h>
//#include <driver.h>

//#define MAX_MSG_LEN 2048
//#define MAX_BUF_COUNT 25


//Hamid: for logging system
#include <stdarg.h>
#define SMLOG_LEVEL 12 
#define SMLOG_FILENAME "./SMLOG.log"
using namespace std;

namespace ESL{

	class SMLog{

		public:
		static void SMLOG(int level, const char* format, ...);
		

		private:
		SMLog();



	};

} // end of namespace ESL
#endif
