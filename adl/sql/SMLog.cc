#include <SMLog.h>
#include <basic.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <adllib.h>

// When enable_smlog = 1, then we write a lot more logs
int enable_smlog = 0;


SMLog::SMLog()
{
}

void SMLog::SMLOG(int level, const char* Format, ...)
{
	if (enable_smlog == 0) {
          return;
        }

	if (level > SMLOG_LEVEL)
		return;

	//   since SM redirects stdout to different files
	//   it might be kind of risky to prompt everything
	//   into it. so you may have to use following pices 
	//   of code to write them into a file.
	
	FILE *fp;
	if((fp=fopen(SMLOG_FILENAME, "a")) == NULL) 
	{
		printf("Cannot open file: %s \n", SMLOG_FILENAME );
		return;
	}
	//FIXME: add date and time information
	//FIXME: write the log in bunches
	va_list Arguments;
	va_start(Arguments, Format);
	vfprintf(fp, Format, Arguments);
	fprintf(fp, "\n");
	va_end(Arguments);

	fclose(fp);
}

