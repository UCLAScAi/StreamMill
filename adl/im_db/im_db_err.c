#include "im_db.h"

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

static void __im_db_errfile __P((const IM_DB_ENV *,
				 int, int, const char *, va_list));

/*
 * im_db_strerror --
 *	ANSI C strerror(3) for DB.
 */
char *
im_db_strerror(error)
int error;
{
    if (error == 0)
	return ("Successful return: 0");
    if (error > 0)
	return strerror(error);
#if 0
    /*
     * !!!
     * The Tcl API requires that some of these return strings be compared
     * against strings stored in application scripts.  So, any of these
     * errors that do not invariably result in a Tcl exception may not be
     * altered.
     */
    switch (error)
    {
    case EINVAL:
	return "EINVAL: Invalid value for arguments";
    case EFBIG:
	return "EFBIG: Argument too big to fit in";
    default:
    {
	/*
	 * !!!
	 * Room for a 64-bit number + slop.  This buffer is only used
	 * if we're given an unknown error, which should never happen.
	 * Note, however, we're no longer thread-safe if it does.
	 */
	static char ebuf[40];

	(void)snprintf(ebuf, sizeof(ebuf), "Unknown error: %d", error);
	return ebuf;
    }
    }
#endif
}

/*
 * __im_db_err --
 *	Standard DB error routine.  The same as __im_db_errx,
 *	except that we don't write to stderr if no output mechanism
 *	was specified.
 *
 * PUBLIC: #ifdef __STDC__
 * PUBLIC: void __im_db_err __P((const IM_DB_ENV *, const char *, ...));
 * PUBLIC: #else
 * PUBLIC: void __im_db_err();
 * PUBLIC: #endif
 */
void
#ifdef __STDC__
__im_db_err(const IM_DB_ENV *dbenv, const char *fmt, ...)
#else
__im_db_err(dbenv, fmt, va_alist)
const IM_DB_ENV *dbenv;
const char *fmt;
va_dcl
#endif
{
    va_list ap;

#ifdef __STDC__
    va_start(ap, fmt);
#else
    va_start(ap);
#endif
    __im_db_real_err(dbenv, 0, 0, fmt, ap);

    va_end(ap);
}

/*
 * __im_db_real_err --
 *	All the DB error routines end up here.
 *
 * PUBLIC: void __im_db_real_err
 * PUBLIC:     __P((const IM_DB_ENV *, int, int, const char *, va_list));
 */
void
__im_db_real_err(dbenv, error, error_set, fmt, ap)
const IM_DB_ENV *dbenv;
int error, error_set;
const char *fmt;
va_list ap;
{
    __im_db_errfile(dbenv, error, error_set, fmt, ap);
}

/*
 * __im_db_errfile --
 *	Do the error message work for FILE *s.
 */
static void
__im_db_errfile(dbenv, error, error_set, fmt, ap)
const IM_DB_ENV *dbenv;
int error, error_set;
const char *fmt;
va_list ap;
{
    FILE *fp;

    fp = (dbenv == NULL) ||
	dbenv->db_errfile == NULL? stderr : dbenv->db_errfile;

    if(dbenv != NULL && dbenv->db_errpfx != NULL)
	fprintf(fp, "%s: ", dbenv->db_errpfx);
    if(fmt != NULL)
    {
	vfprintf(fp, fmt, ap);
	if(error_set)
	    fprintf(fp, ": ");
    }
    if(error_set)
	fprintf(fp, "%s", im_db_strerror(error));
    fprintf(fp, "\n");
    fflush(fp);
}
