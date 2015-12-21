/* translation of const 
   5
   "test"
 */
{
  currentValue->type = O_NUM;
  NumObj(currentValue) = 5;
  result = SUCCESS;
}

{
  currentValue->type = O_STRING;
  strcpy(StrObj(currentValue), "test");
  result = SUCCESS;
}
 
/* translation of binop
   a+b
 */
{
  nt_obj_t *a=newNumObj();
  nt_obj_t *b=newNumObj();
  //[a]
  *a = currentValue;
  //[b]
  *b = currentValue;
  NumObj(currentValue) = NumObj(a)+NumObj(b);
  result = SUCCESS;
}

transExp2C()
{
}
/* translation of values() to C code 
 * values(a(x),b(y))
 */
init:
{
  /* empty */
}

gettuple:
{
  currentList->empty();
  //[a(x)]
  currentList->appendElement(currentValue);
  //[b(y)]
  currentList->appendElement(currentValue);
  result = SUCCESS;
}

/* translation of materialized table to C code 

   t
*/
init:
{
  rc = t->cursor(t, NULL, &t_cursor, 0);
}

gettuple:

/* translation of SEL box to C code

   SELECT A(a) a, B(x) b
   FROM g1, g2;
   WHERE C(a,x);
*/
init:
{
  g1::init();
  g2::init();
}
gettuple:
{
  int node_index = 0;
  for (;;) {
    switch(node_index) {
    case 0:
      g1::gettuple();
      if (result==FAIL)
	goto next;
      node_index++;
      break;
    case 1:
      g2::gettuple();
      if (result==FAIL) {
	g2::init();
	node_index--;
      }
      else {
	if (C(a,x)) {
	  this->tuple.a = A(g1->tuple.a);
	  this->tuple.b = B(g2->tuple.b);
	    goto next;
	}
      }
      break;
    }
  }
 next:
}
