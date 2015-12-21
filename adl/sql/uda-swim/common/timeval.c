#include <sys/timeb.h>
#include <sys/time.h>

struct timeb record,lasttime;

void resetTime(){
	ftime(&record);
	lasttime = record;
}

unsigned int howlong()
{
	ftime(&record);
	return (record.time - lasttime.time)*1000 +
	 	(record.millitm - lasttime.millitm);
	lasttime = record;
}

struct timeval finer_time;
struct timezone tz;

unsigned int tellMeTime()
{
	gettimeofday(&finer_time, &tz);
	return (finer_time.tv_sec % 100)*1000000 + finer_time.tv_usec;
}

/*
struct timeb {
	time_t   time;
	unsigned short millitm;
	short    timezone;
	short    dstflag;
};
*/

