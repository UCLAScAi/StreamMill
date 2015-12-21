/* 
   values (1,"123",4+5);
*/
cv = newObj(1);
appendList(cl, cv);
cv = newObj("123");
appendList(cl, cv);
cv1 = newObj(4);
cv2 = newObj(5);
NumObj(cv) = NumObj(cv1)+NumObj(cv2);
appendList(cl, cv);

/* 
   t           v      
   -------     -------
   a  b  c     e  f  g

   table t(a int, b int, c int);
   table v(e int, f int, g int);

   select a+b as 1, b+c as 2 
   from t as t1, (select a+b as a, b+c as b from t as x) as t2
   where t1.a=t2.a and 
         t1.b > (select f from v,t where t.a=v.e) and
	 exists (select f from v where t.a<v.e) and
	 not exists (select f from v where t.a>v.e) 

            node0
       /      |        \
   node1 t1 node2 t2   node3 _
              |        /  \
            node4 x node5  node6
                     (v)    (t)
*/

  // t,{} => venv    
  // _0_t1, _0_t2 => venv

/*********** global (program) begin ***********/
DBT key, data;
int rc = 0;
/*********** global (program) end ***********/

/*********** global (block) begin ***********/
/* declare all base tables */
decDB("t", NULL); // [t, {}] => venv
decDB("v", NULL); // [v, {}] => venv 
/*********** global (block) end ***********/

int main() {

  /*********** global (SQL statement) begin ***********/
  /* declare all cursors on the base table */
  decDBC("t", "t1_1");
  decDBC("t", "x_4");
  decDBC("v", "v_5");
  decDBC("t", "t_6");

  /* declare all first entry flag */
  decInt("first_entry_0", 1);
  /* declare all index */
  decInt("_0_index", 0);
  decInt("_1_index", 0);

  while (rc==0) {
    /*********** global (SQL statement) end  ***********/
    
    /* return type */
    decQun("_null_0", { {"field_1", int}, {"field_2", int}});
    /************* transSqlOpr*************************************/

    /* declare all quns under current node */
    decQun("t1_0_1", {/*t*/});  // [t1,{}]=>env
    decQun("t2_0_2", { {"field_a", int}, {"field_b", int}}); // [t2,{}]=>env
    decQun("_0_3", { {"field_1", int}}); // --

    /* compute sub query */
  next:
    while (_0_index>=0 && _0_index < 2) {
      switch (_0_index) {
      case 0:   // from t as t1
	{
	  /*********** transSqlOpr Begin *********/
	  getCursor(first_entry_1, "t1_1"); //Berkeley code
	  if (rc == 0) 
	    assign(key, data, "t1_0_1");
	  else if (rc == DB_NOTFOUND)
	    first_entry_1 = 1;
	  else 
	    goto exit;
	  /*********** transSqlOpr End ***********/
	}
	break;
      case 1:   // from (select a+b as a, b+c as b from t as x) as t2
	{
	  // _1_t1, t => venv
	  /*********** transSqlOpr Begin *********/
	  decQun("x_2_4");

	  /*********** transSqlOpr Begin *********/
	next_4:
	  getCursor(first_entry_4, "x_4");	// Berkeley code
	  if (rc == 0) 
	    assign(key, data, "x_2_4");
	  else if (rc == DB_NOTFOUND) {
	    first_entry_4 = 1;
	  } else 
	    goto exit;
	  /*********** transSqlOpr End ***********/
	
	  if (rc==0) {
	    t2_0_2.a = x_2_4.a+x_2_4.b;
	    t2_0_2.b = x_2_4.b+x_2_4.c;
	  }
	  /*********** transSqlOpr End ***********/

	  // _1_t1, t <= venv
	}
	break;
      }

      if (rc==0)
	_0_index++;
      else if (rc==DB_NOTFOUND)
	_0_index--;
    }
  
    if (rc == 0) {
      _0_index--;		// reset index 

      /* compute predicate */
      if (!(t1_0_1.a == t2_0_2.a))
	goto next;

      retry=0;
    retry:
      {
	decQun("v_3_5");
	decQun("t_3_6");

      
      }
      if (rc==0) {
	if (retry# > 0 ) 
	  goto exit;
	else {
	  retry#++;
	  goto retry;
	}
      } else if (retry#==0)
	goto exit;
      else if (!(t1_0_1.b > _0_3.f1))
	goto next;
    
      /* EXISTS */
      {
      }
      if (rc==DB_NOTFOUND)
	goto next;

      /* NOT EXISTS */
      {
      }
      if (rc==0)
	goto next;

      /* compute head expr */
      for (i=0; i<head_expr->length; i++) {
    
      }
      /* insert/delete/update */
      {
      }
    } 
    else { /* rc!=0 */
      /* close all cursors */
      closeCursor(_0_t1);
      closeCursor(_1_t1);
    }
  } /* while (rc==0) */
  
 last:
  
  //    <= venv ;
  //    <= venv ;
  delDB("t", NULL); 
  delDB("v", NULL);
 exit:
  return rc;
}
   
/* getTuple ("_0_t1") */
memset(&key, 0, sizeof(key));
memset(&data, 0, sizeof(data));
rc = _0_t1->c_get(_0_t1, key, data, DB_NEXT);
if (rc != DB_NOTFOUND) {
  _0_t1->err(_0_t1, rc, "DBcursor->get");
  goto exit;
}
appendList(_0_t1_list);

/* getTuple ("_0_t2") */
rc = _1_t1->c_get();
appendList(_1_t1_list);
/* head expr */
appendList(_0_t2_list);



