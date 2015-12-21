#include <errno.h>

#include "im_db.h"

static void __im_dbenv_err(const IM_DB_ENV *, int, const char *, ...);
static void __im_dbenv_errx(const IM_DB_ENV *, const char *, ...);
static void __im_dbenv_set_errfile(IM_DB_ENV *, FILE *);
static void __im_dbenv_set_errpfx(IM_DB_ENV *, const char *);

/*
 * db_env_create --
 *	DB_ENV constructor.
 */
int
#ifdef __STDC__
im_db_env_create(IM_DB_ENV **dbenvpp, u_int32_t flags)
#else
im_db_env_create(dbenvpp, flags)
IM_DB_ENV **dbenvpp;
u_int32_t flags;
#endif
{
    IM_DB_ENV *dbenv;
    int ret;

    /*
     * !!!
     * We can't call the flags-checking routines, we don't have an
     * environment yet.
     */
    if (flags != 0)
	return EINVAL;

    if ((ret = __my_calloc(1, sizeof(*dbenv), &dbenv)) != 0)
	return ret;

    if ((ret = __im_dbenv_init(dbenv)) != 0)
    {
	free(dbenv);
	return ret;
    }

    *dbenvpp = dbenv;
    return (0);
}

/*
 * __dbenv_init --
 *	Initialize a DB_ENV structure.
 *
 * PUBLIC: int  __im_dbenv_init __P((IM_DB_ENV *));
 */
int
__im_dbenv_init(dbenv)
IM_DB_ENV *dbenv;
{
    dbenv->err = __im_dbenv_err;
    dbenv->errx = __im_dbenv_errx;

    dbenv->set_errfile = __im_dbenv_set_errfile;
    dbenv->set_errpfx = __im_dbenv_set_errpfx;

    return (0);
}

/*
 * __im_dbenv_err --
 *	Error message, including the standard error string.
 */
static void
#ifdef __STDC__
__im_dbenv_err(const IM_DB_ENV *dbenv, int error, const char *fmt, ...)
#else
__dbenv_err(dbenv, error, fmt, va_alist)
const IM_DB_ENV *dbenv;
int error;
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
    __im_db_real_err(dbenv, error, 1, fmt, ap);

    va_end(ap);
}

/*
 * __im_dbenv_errx --
 *	Error message.
 */
static void
#ifdef __STDC__
__im_dbenv_errx(const IM_DB_ENV *dbenv, const char *fmt, ...)
#else
__im_dbenv_errx(dbenv, fmt, va_alist)
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

static void
__im_dbenv_set_errfile(dbenv, errfile)
IM_DB_ENV *dbenv;
FILE *errfile;
{
    dbenv->db_errfile = errfile;
}

static void
__im_dbenv_set_errpfx(dbenv, errpfx)
IM_DB_ENV *dbenv;
const char *errpfx;
{
	dbenv->db_errpfx = errpfx;
}
