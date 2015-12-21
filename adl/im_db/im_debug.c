#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/time.h>
#include <unistd.h>

#include "im_db.h"

time_t current_time;
struct tm *tm;

/****************************************************************/
/* debugging routines */
/****************************************************************/
void
cursor_print(IM_RELC *dbc)
{
    switch(dbc->rel->type)
    {
    case IM_LINKEDLIST:
	printf("Cursor: of Relation %s\n", dbc->rel->name);
	break;
    default:
	__im_db_err(NULL, "Internal error!: tuple_physical_print() - invalid data type");
    }	
}

void
tuple_print(TLL *tll)
{
    register void *key = tll->key.data, *data = tll->data.data;

    printf("\t|| %d | %s || %d | %s ||\n",
	   *(size_t *)key, key+sizeof(size_t),
	   *(size_t *)data, data+sizeof(size_t));
}

void
im_rel_print(IM_REL *rel)
{
    register int i;
    TLL *cur = rel->head;

    printf("IM_REL_LL(%s)\n", rel->name);
    /* print the tuples */
    if(cur == NULL)
	return;
    while(1)
    {
	if(IS_OID_REL(rel->flags) && TUPLE_DELETED(cur->status))
	   printf("deleted!");
	tuple_print(cur);
	cur = cur->next;
	if(cur == rel->head)
	    /* meet the head again, end */
	    break;
    }
}

/*****************************************************************
 * Debug routine main driver
 *****************************************************************/
DBT dbt1, dbt2, dbt3;

#ifdef TEST
void
im_rel_test(u_int32_t flags)
{
    IM_REL *rel;
    register i;
    int ret;

    printf("\n\nCreate A\n");
    im_rel_create(&rel, NULL, IM_LINKEDLIST, flags);
    printf("\nOpen A\n");
    rel->open(rel, "A", 0);

    printf("\nPut A\n");
    dbt1.data = strdup(" hello"), dbt1.size = 7;
    dbt2.data = strdup(" world"), dbt2.size = 7;
    for(i=0; i<5; i++)
    {
	*(char *)(dbt1.data) = 'a' + i;
	*(char *)(dbt2.data) = 'a' + i;
	rel->put(rel, &dbt1, &dbt2, DB_APPEND);
    }
    *(char *)(dbt1.data) = 'a';
    *(char *)(dbt2.data) = 'a';
    rel->put(rel, &dbt1, &dbt2, DB_APPEND);
    rel->print(rel);

    printf("\nGet A (flag = 0)\n");
    dbt1.data = strdup("ehello"), dbt1.size = 7;
    if((ret = rel->get(rel, &dbt1, &dbt3, 0)) == 0)
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt1.data = strdup("fhello"), dbt1.size = 7;
    if((ret = rel->get(rel, &dbt1, &dbt3, 0)) == 0)
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nGet A (flag = GET_BOTH)\n");
    dbt1.data = strdup("ehello"), dbt1.size = 7;
    dbt3.data = strdup("eworld"), dbt3.size = 7;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_GET_BOTH)) == 0)
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt1.data = strdup("ehello"), dbt1.size = 7;
    dbt3.data = strdup("fworld"), dbt3.size = 7;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_GET_BOTH)) == 0)
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nGet A (flag = SET_RECNO)\n");
    ret = 6;
    dbt1.data = &ret;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_SET_RECNO)) == 0)
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    else
	fprintf(stderr, "error return = %d\n", ret);
    ret = 7;
    dbt1.data = &ret;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_SET_RECNO)) == 0)
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nGet A (flag = GET_OIDOFKEY ... GET_OID)\n");
    dbt1.data = "bhello", dbt1.size = 7;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_GET_OIDOFKEY)) == 0)
    {
	printf("data (data=%p,size=%d)\n", *(void **)dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt1.data = dbt3.data, dbt1.size = dbt3.size;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_GET_OID)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nDel A\n");
    dbt1.data = strdup("ehello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);
    dbt1.data = strdup("ahello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);
    dbt1.data = strdup("ahello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);
    dbt1.data = strdup("bhello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);
    dbt1.data = strdup("chello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);
    dbt1.data = strdup("dhello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);
    dbt1.data = strdup("dhello"), dbt1.size = 7;
    rel->del(rel, &dbt1, 0);
    rel->print(rel);

    printf("\nClose A\n");
    rel->close(rel, 0);
}

im_cursor_test(u_int32_t flags)
{
    IM_REL *rel;
    IM_RELC *relc;
    register i;
    int ret;

    printf("\n\nCreate A\n");
    im_rel_create(&rel, NULL, IM_LINKEDLIST, flags);
    printf("\nOpen A\n");
    rel->open(rel, "A", 0);

    printf("\nPut A\n");
    dbt1.data = strdup(" hello"), dbt1.size = 7;
    dbt2.data = strdup(" world"), dbt2.size = 7;
    for(i=0; i<5; i++)
    {
	*(char *)(dbt1.data) = 'a' + i;
	*(char *)(dbt2.data) = 'a' + i;
	rel->put(rel, &dbt1, &dbt2, DB_APPEND);
    }
    *(char *)(dbt1.data) = 'a';
    *(char *)(dbt2.data) = 'a';
    rel->put(rel, &dbt1, &dbt2, DB_APPEND);
    rel->print(rel);

    printf("\nOpen a cursor.\n");
    rel->cursor(rel, &relc, 0);

    printf("\nGet via cursor (flag = FIRST ... NEXT)\n");
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_FIRST)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    for(i=0; i<6; i++)
    {
	if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_NEXT)) == 0)
	{
	    printf("\nkey  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	    printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
	}
	else
	    fprintf(stderr, "error return = %d\n", ret);
    }

    printf("\nGet via cursor (flag = LAST ... PREV)\n");
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_LAST)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    for(i=0; i<6; i++)
    {
	if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_PREV)) == 0)
	{
	    printf("\nkey  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	    printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
	}
	else
	    fprintf(stderr, "error return = %d\n", ret);
    }

    printf("\nGet via cursor (flag = CURRENT / 1 fail & 1 success)\n");
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_CURRENT)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    relc->c_get(relc, &dbt1, &dbt3, DB_FIRST);
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_CURRENT)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nGet via cursor (flag = SET)\n");
    dbt1.data = strdup("ehello"), dbt1.size = 7;
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_SET)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt1.data = strdup("fhello"), dbt1.size = 7;
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_SET)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nGet via cursor (flag = GET_BOTH)\n");
    dbt1.data = strdup("ehello"), dbt1.size = 7;
    dbt3.data = strdup("eworld"), dbt1.size = 7;
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_GET_BOTH)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt3.data = strdup("fhello"), dbt3.size = 7;
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_GET_BOTH)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nGet via cursor (flag = FIRST ... NEXT_DUP)\n");
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_FIRST)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    for(i=0; i<3; i++)
    {
	if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_NEXT_DUP)) == 0)
	{
	    printf("\nkey  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	    printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
	}
	else
	    fprintf(stderr, "error return = %d\n", ret);
    }

    printf("\nGet via cursor (flag = GET_OIDOFKEY ... GET_OID)\n");
    dbt1.data = "bhello", dbt1.size = 7;
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_SET)) != 0)
	fprintf(stderr, "error return = %d\n", ret);
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_GET_OIDOFKEY)) == 0)
    {
	printf("data (data=%p,size=%d)\n", *(void **)dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt1.data = dbt3.data, dbt1.size = dbt3.size;
    if((ret = rel->get(rel, &dbt1, &dbt3, DB_GET_OID)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nPut via cursor\n");
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_FIRST)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    dbt3.data = "aw", dbt3.size = 3;
    if((ret = relc->c_put(relc, &dbt1, &dbt3, DB_CURRENT)) != 0)
	fprintf(stderr, "error return = %d\n", ret);
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_FIRST)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);

    printf("\nDelete via cursor\n");
    if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_FIRST)) == 0)
    {
	printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
    }
    else
	fprintf(stderr, "error return = %d\n", ret);
    relc->c_del(relc, 0);
    rel->print(rel);
    for(i=0; i<6; i++)
    {
	if((ret = relc->c_get(relc, &dbt1, &dbt3, DB_LAST)) == 0)
	{
	    printf("key  (data=%s,size=%d)\n", dbt1.data, dbt1.size);
	    printf("data (data=%s,size=%d)\n", dbt3.data, dbt3.size);
	}
	else
	    fprintf(stderr, "error return = %d\n", ret);
	relc->c_del(relc, 0);
	rel->print(rel);
    }

    printf("\nClose cursor & relation\n");
    relc->c_close(relc);
    rel->close(rel, 0);
}

int
main(int argc, char *argv[])
{
    im_rel_test(IM_REL_INDEXED | IM_OID);
    /*im_cursor_test(IM_REL_INDEXED | IM_OID);*/
}
#endif
