#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/*****************************************************************
 * Errno handling routines
 *****************************************************************/
/*
 * __my_get_errno --
 *	Return the value of errno.
 *
 * PUBLIC: int __my_get_errno __P((void));
 */
int
__my_get_errno()
{
    /* This routine must be able to return the same value repeatedly. */
    return (errno);
}

/*
 * __my_set_errno --
 *	Set the value of errno.
 *
 * PUBLIC: void __my_set_errno __P((int));
 */
void
__my_set_errno(evalue)
int evalue;
{
    errno = evalue;
}

/*****************************************************************
 * Memory allocation handling routines
 *****************************************************************/
/*
 * __my_strdup --
 *	The strdup(3) function for myself.
 *
 * PUBLIC: int __my_strdup __P((const char *, void *));
 */
int
__my_strdup(str, storep)
const char *str;
void *storep;
{
    size_t size;
    int ret;
    void *p;

    *(void **)storep = NULL;

    size = strlen(str) + 1;
    if((ret = __my_malloc(size, &p)) != 0)
	return ret;

    memcpy(p, str, size);

    *(void **)storep = p;
    return 0;
}

/*
 * __my_calloc --
 *	The calloc(3) function for myself.
 *
 * PUBLIC: int __my_calloc __P((size_t, size_t, void *));
 */
int
__my_calloc(num, size, storep)
size_t num, size;
void *storep;
{
    void *p;
    int ret;

    size *= num;
    if((ret = __my_malloc(size, &p)) != 0)
	return ret;

    memset(p, 0, size);

    *(void **)storep = p;
    return 0;
}

/*
 * __my_malloc --
 *	The malloc(3) function for myself.
 *
 * PUBLIC: int __my_malloc __P((size_t, void *));
 */
int
__my_malloc(size, storep)
size_t size;
void **storep;
{
    void *p;

    *(void **)storep = NULL;

    /* Never allocate 0 bytes -- some C libraries don't like it. */
    if (size == 0)
	++size;

    /* Some C libraries don't correctly set errno when malloc(3) fails. */
    __my_set_errno(0);
    p = malloc(size);
    if(p == NULL)
    {
	if(__my_get_errno() == 0)
	    __my_set_errno(ENOMEM);
	return __my_get_errno();
    }

    *(void **)storep = p;

    return 0;
}

/*
 * __my_realloc --
 *	The realloc(3) function for myself.
 *
 * PUBLIC: int __my_realloc __P((size_t, void *));
 */
int
__my_realloc(size, storep)
size_t size;
void *storep;
{
    void *p, *ptr;

    ptr = *(void **)storep;

    /* If we haven't yet allocated anything yet, simply call malloc. */
    if (ptr == NULL)
	return __my_malloc(size, storep);

    /* Never allocate 0 bytes -- some C libraries don't like it. */
    if (size == 0)
	++size;
    /*
     * Some C libraries don't correctly set errno when realloc(3) fails.
     *
     * Don't overwrite the original pointer, there are places in DB we
     * try to continue after realloc fails.
     */
    __my_set_errno(0);
    p = realloc(ptr, size);
    if(p == NULL)
    {
	if(__my_get_errno() == 0)
	    __my_set_errno(ENOMEM);
	return __my_get_errno();
    }

    *(void **)storep = p;

    return 0;
}
