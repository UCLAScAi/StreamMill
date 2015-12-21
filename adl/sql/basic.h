/*
 *
 * Common constants and type declarations for Sprite.
 */

#ifndef _BASIC_H
#define _BASIC_H

#include <config.h>
#include <limits.h>

/*
 * A boolean type is defined as an integer, not an enum. This allows a
 * boolean argument to be an expression that isn't strictly 0 or 1 valued.
 */

typedef int Boolean;
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/*
 * Functions that must return a status can return a ReturnStatus to
 * indicate success or type of failure.
 */

typedef int  ReturnStatus;

/*
 * The following statuses overlap with the first 2 generic statuses 
 * defined in status.h:
 *
 * SUCCESS			There was no error.
 * FAILURE			There was a general error.
 */

#define	SUCCESS			0x00000000
#define	FAILURE			0x00000001


/*
 * A nil pointer must be something that will cause an exception if 
 * referenced.  There are two nils: the kernels nil and the nil used
 * by user processes.
 */

#define NIL 		(~0)
#define USER_NIL 	0
#ifndef NULL
#define NULL	 	0
#endif 

/*
 * An address is just a pointer in C.  It is defined as a character pointer
 * so that address arithmetic will work properly, a byte at a time.
 */

typedef char *Address;

/*
 * ClientData is an uninterpreted word.  It is defined as an int so that
 * kdbx will not interpret client data as a string.  Unlike an "Address",
 * client data will generally not be used in arithmetic.
 */

typedef int *ClientData;


/*
**	portability
**
*/
#ifndef HAVE_BCOPY
/*  #  undef 	bzero */
/*  #  undef 	bcopy */
/*  #  undef 	bcmp */
/*  #  define	bzero(a,l)	memset((void *)a,0,(size_t)l) */
/*  #  define	bcopy(s,d,l)	memcpy(d,s,(size_t)l) */
/*  #  define	bcmp		memcmp */
#endif

#ifndef HAVE_RINDEX
#  undef	index
#  undef	rindex
#  define	index		strchr
#  define	rindex		strrchr
#endif

#ifndef HAVE_RANDOM
#  undef	random
#  undef	srandom
#  define	random		rand
#  define	srandom		srand
#endif

#ifdef HAVE_SELECT_H
	/*
	** AIX has a struct fd_set and can be distinguished by
	** its needing <select.h>
	*/
	typedef struct fd_set fd_set
#endif

/*
#ifndef HAVE_U_INT
	typedef	unsigned int u_int;
#endif

#ifndef HAVE_SSIZE_T
	typedef int  	ssize_t; 
#endif
*/

#ifndef HAVE_FTRUNCATE
	/*
	** SCO ODT doesn't have ftruncate() !!! Have to use old Xenix stuff
	*/
#       undef	ftruncate
#  	define 	ftruncate	chsize
#endif


#ifndef BYTE_ORDER
#	define LITTLE_ENDIAN   1234            /* LSB first: i386, vax */
#	define BIG_ENDIAN      4321            /* MSB first: 68000, ibm, net */
#	define BYTE_ORDER      BIG_ENDIAN      /* Set for your system. */
#endif


#ifndef USHRT_MAX
#	define USHRT_MAX               0xFFFF
#	define ULONG_MAX               0xFFFFFFFF
#endif

#ifndef O_ACCMODE                       /* POSIX 1003.1 access mode mask. */
#	define O_ACCMODE       (O_RDONLY|O_WRONLY|O_RDWR)
#endif

#ifndef _POSIX2_RE_DUP_MAX              /* POSIX 1003.2 RE limit. */
#	define _POSIX2_RE_DUP_MAX      255
#endif

/*
 * If you can't provide lock values in the open(2) call.  Note, this
 * allows races to happen.
 */
#ifndef O_EXLOCK                        /* 4.4BSD extension. */
#	define O_EXLOCK        0
#endif

#ifndef O_SHLOCK                        /* 4.4BSD extension. */
#	define O_SHLOCK        0
#endif

#ifndef EFTYPE
#	define EFTYPE          EINVAL   /* POSIX 1003.1 format errno. */
#endif

#ifndef WCOREDUMP                       /* 4.4BSD extension */
#	define WCOREDUMP(a)    0
#endif

#ifndef STDERR_FILENO
#	define STDIN_FILENO    0               /* ANSI C #defines */
#	define STDOUT_FILENO   1
#	define STDERR_FILENO   2
#endif

#ifndef SEEK_END
#	define SEEK_SET        0               /* POSIX 1003.1 seek values */
#	define SEEK_CUR        1
#	define SEEK_END        2
#endif

#ifndef _POSIX_VDISABLE                 /* POSIX 1003.1 disabling char.  */
#	define _POSIX_VDISABLE 0               /* Some systems used 0. */
#endif

#ifndef TCSASOFT                        /* 4.4BSD extension. */
#	define TCSASOFT        0
#endif

#ifndef _POSIX2_RE_DUP_MAX              /* POSIX 1003.2 values. */
#	define _POSIX2_RE_DUP_MAX      255
#endif

#ifndef MAX                             /* Usually found in <sys/param.h>. */
#	define MAX(_a,_b)      ((_a)<(_b)?(_b):(_a))
#endif
#ifndef MIN                             /* Usually found in <sys/param.h>. */
#	define MIN(_a,_b)      ((_a)<(_b)?(_a):(_b))
#endif

/* Default file permissions. */
#ifndef DEFFILEMODE                     /* 4.4BSD extension. */
#	define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

#ifndef S_ISDIR                         /* POSIX 1003.1 file type tests.  */
#	define S_ISDIR(m)      ((m & 0170000) == 0040000)    /* directory */
#	define S_ISCHR(m)      ((m & 0170000) == 0020000)    /* char special*/
#	define S_ISBLK(m)      ((m & 0170000) == 0060000)    /* block special*/
#	define S_ISREG(m)      ((m & 0170000) == 0100000)    /* regular file */
#	define S_ISFIFO(m)     ((m & 0170000) == 0010000)    /* fifo */
#endif
#ifndef S_ISLNK                         /* BSD POSIX 1003.1 extensions */
#	define S_ISLNK(m)      ((m & 0170000) == 0120000)    /* symbolic link */
#	define S_ISSOCK(m)     ((m & 0170000) == 0140000)    /* socket */
#endif

/* The type of a va_list. */
#ifndef _BSD_VA_LIST_                   /* 4.4BSD #define. */
#	define _BSD_VA_LIST_   char *
#endif

#endif /* _BASIC_H */
