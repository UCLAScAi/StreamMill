/*

create FUNCTION tdaggregatetesta
  (x INTEGER)
 RETURNS INTEGER
 CLASS AGGREGATE (10000)
 LANGUAGE C
 NO SQL
 PARAMETER STYLE SQL
 DETERMINISTIC
 CALLED ON NULL INPUT
 EXTERNAL name 'CS!tdaggregatetesta!/usr/local/Graphical_Debugger/workspace/StreamMill/Samples/StreamMillSQLUDFs/TD/AggregateUDF/NormalAggregates/Tests/TDTestA.c';

replace FUNCTION tdaggregatetesta
  (x INTEGER)
 RETURNS INTEGER
 CLASS AGGREGATE (10000)
 LANGUAGE C
 NO SQL
 PARAMETER STYLE SQL
 DETERMINISTIC
 CALLED ON NULL INPUT
 EXTERNAL name 'CS!tdaggregatetesta!/usr/local/Graphical_Debugger/workspace/StreamMill/Samples/StreamMillSQLUDFs/TD/AggregateUDF/NormalAggregates/Tests/TDTestA.c';



 */

#define SQL_TEXT Latin_Text
#include <sqltypes_td.h>
#include <string.h>
#define inc(next, size) ( (next+1) % size )
typedef struct element {
	int v;
} Element;
typedef struct agr_storage {
	int current_value;    // Current moving sum total
	int next;             // Location to store next detail row
	int tptr;             // Trailing buffer pointer
	int count;            // Number of detail rows processed
	int window_size;      // Moving window size
	Element *data;
	int heap;             // Heap area to store window rows
} AGR_Storage;
void tdaggregatetesta( FNC_Phase phase,
		FNC_Context_t *fctx,
		INTEGER *x,
		INTEGER *result,
		int *x_i,
		int *result_i,
		char sqlstate[6],
		SQL_TEXT fncname[129],
		SQL_TEXT sfncname[129],
		SQL_TEXT error_message[257] )
{
	/* pointers to intermediate storage areas */
	AGR_Storage *s1 = fctx->interim1;
	AGR_Storage *s2 = fctx->interim2;
	int i, t_x;
	switch (phase)
	{
		case AGR_INIT:
			if (fctx->window_size < 0) // non moving window case
				s1 = FNC_DefMem(sizeof(AGR_Storage));
			else
				s1 = FNC_DefMem(sizeof(AGR_Storage) + (fctx->window_size *
							sizeof(Element)));
			if (s1 == NULL)
			{
				strcpy(sqlstate, "U0001");
				return;
			}
			if (fctx->window_size >= 0) // moving window case
			{
				s1->count = 0;
				s1->next = 0;
				s1->tptr = 0;
				s1->data = (Element*)&s1->heap;
				for (i=0; i < fctx->window_size; i++) //initialize heap
					s1->data[i].v = 0;
			}
			s1->current_value = 0;
			s1->window_size = fctx->window_size;
			/***************************************************/
			/* Fall through to detail phase*/
			/***************************************************/
		case AGR_DETAIL:
			if (*x_i == -1) //NULL set value to zero
				t_x = 0;
			else
				t_x = *x;
			if (s1->window_size <= 0) // non moving case
				s1->current_value += t_x;
			else // Moving
			{
				s1->count++;
				if (s1->count > s1->window_size) // Remove value row from window
					{
						s1->current_value -= s1->data[s1->tptr].v;
						s1->tptr = inc(s1->tptr,s1->window_size);
					}
				s1->current_value += t_x;         // Add row to window
				s1->data[s1->next].v = t_x;
				s1->next = inc(s1->next,s1->window_size);
			}
			break;
		case AGR_MOVINGTRAIL:
			s1->current_value -= s1->data[s1->tptr].v;
			s1->tptr = inc(s1->tptr,s1->window_size);
			break;
		case AGR_FINAL:
			*result = s1->current_value;
			break;
		case AGR_COMBINE:      // Also add this to catch any undefined cases
			s1->current_value += s2->current_value;
			break;
		default:
			/* If it gets here there must be an error */
			sprintf(error_message,"phase is %d",phase);
			strcpy(sqlstate, "U0005");
			return;
	}
}
