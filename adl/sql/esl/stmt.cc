#include "stmt.h"
#include "strategy.h"
#include "querySchdl.h"
#include "SMLog.h"

using namespace ESL;
using namespace std;

stmt::stmt():weight(WeightProfile::WEIGHT_MIN), monitor_src_stat(false),monitor_sink_stat(false), /*monitor_stmt_stat(false),*/ valid(true),rankSet(false),weightSet(false),deadline(0),quota(0),quota_left(0),deadline_quota(0)
{  
	this->rank.priority = OperatorPriorityStrategy::MAX_PRIORITY;
	this->stmtEntry = NULL;
}

stmt::~stmt() 
{  
}

//nStmt::nStmt() 
//{
//  type = stmt_normal;
//}

nStmt::nStmt(char *name, buffer* i, buffer* o, bool sub_stmt)
{
	SMLog::SMLOG(10, "Entering nStmt::nStmt");	
	type = stmt_normal;
	strcpy(this->name, name);
	in = i;
	out = o;
	strcpy(this->inBufName, i->name);  
	strcpy(this->outBufName, o->name);  
	this->sub_stmt = sub_stmt;
}

void nStmt::print() {
	fprintf(stderr, "%s: %s -> %s\n", name, inBufName, outBufName);
}

nStmt::~nStmt() 
{  
}

stmt_rc nStmt::exe(int freeVar) 
{
	SMLog::SMLOG(10, "Entering nStmt::exe freevar: %i", freeVar);	

	if (querySchdl::verbose) cout << "enter exe of stmt " << name << endl;

	if (func) 
	{
		SMLog::SMLOG(11, "\texecute stmt: %s", name);	
		if (querySchdl::verbose) 
			cout << "execute stmt: " << name << endl;    
		stmt_rc rc = (stmt_rc) ((*func)(bufferMngr::getInstance(), freeVar, NULL, getInMemTables()));
		SMLog::SMLOG(11, "\tfinish stmt, return code: %i", (int) rc);	
		if (querySchdl::verbose) 
			cout << "finish stmt, return code " << (int) rc << endl;    
		return rc;
	} 
	else 
	{
		cerr<<"Warning: stmt not ready"<<endl;
		return s_failure;
	}

	/*
	//for testing only, just move the tuple over
	struct timeval tv1;  
	dbt *d = new cDBT("ddd", MAX_STR_LEN, &tv1);
	//dbt* d = 0;
	if (in == 0) {
	cout << "in buffer is null in statememt " << name << endl;
	return s_failure;
	} else if (in->empty()) {
	cout << "buffer has no input in statement " << name << endl;
	return s_no_input;
	} else {
	cout << "in buffer is " << in->name << endl;
	in->get(d);
	if (d == 0) {
	cout << "buffer has no input in statement " << name << endl;
	return s_no_input;
	}    
	}

	if (out == 0) {
	cout << "out buffer is null in statememt " << name << endl;
	return s_failure;
	} else {
	cout << "out buffer is " << out->name << endl;
	out->put(d);
	}
	delete d;

	cout << "stmt executed: " << name << endl;

	return s_success;
	 */
}

uStmt::uStmt()
{
}

uStmt::~uStmt() 
{  
}

void uStmt::print() {
	fprintf(stderr, "union stmt\n");

	for (list<stmt*>::iterator itr = sub_stmts.begin(); itr != sub_stmts.end(); itr++) {
		(*itr)->print();
	}
	fprintf(stderr, "DONE union stmt %s\n", outBufName);
}

//utStmt::utStmt() 
//{
//  type = stmt_t_union;
//  in = 0;
//}

utStmt::utStmt(char *name, /*buffer* i,*/ buffer* o, bool sub_stmt)
{
	SMLog::SMLOG(10, "Entering utStmt::utStmt");	
	type = stmt_t_union;
	strcpy(this->name, name);
	in = 0;
	out = o;
	this->inBufName[0] = '\0';
	strcpy(this->outBufName, o->name);  
	this->sub_stmt = sub_stmt;  
}

utStmt::~utStmt() 
{  
}


stmt_rc utStmt::exe_set_in() 
{
	SMLog::SMLOG(10, "Entering utStmt::exe_set_in");	
	if (querySchdl::verbose) cout << "enter exe of timed union stmt " << name << endl;

	struct timeval tv1;  
	//dbt *d = new cDBT("ddd", MAX_STR_LEN, &tv1);
	cDBT d(MAX_STR_LEN, &tv1);
	cDBT current(MAX_STR_LEN, &tv1);
	bool empty = false;
	bool first = true;

	//testing to see which input buffer can be used to take next tuple.
	for (list<buffer*>::iterator itr = union_bufs.begin(); itr != union_bufs.end(); itr++) {
		if ((*itr)->empty()) {
			empty = true;
			in = *itr;
			break;
		} else {
			(*itr)->get(&d);
			if (querySchdl::verbose) {
				cout << "tuple in timed union: ";
				d.print();
			}

			if (first) {
				current.copy(&d);
				//current.print();
				in = *itr;
				if (querySchdl::verbose) {
					cout << "earliest tuple so far: ";
					current.print();
				}
				first = false;
			} else if(current.atime->tv_sec > d.atime->tv_sec
					|| (current.atime->tv_sec == d.atime->tv_sec
						&& current.atime->tv_usec > d.atime->tv_usec)){
				//take the smallest timestamp
				current.copy(&d);
				in = *itr;
				if (querySchdl::verbose) {
					cout << "earliest tuple so far: ";
					current.print();
				}
			}
		}
	}

	if (empty) {
		if (querySchdl::verbose) cout << "timed union, one buffer empty, will backtrack." << endl;
		return s_no_input;
	} else {
		if (out == 0) {
			cerr << "out buffer is null in statememt " << name << endl;
			return s_failure;
		} else {
			if (querySchdl::verbose) {
				cout << "out buffer is " << out->name << endl;
				cout << "at the end of union, earliest tuple is: ";
				current.print();
			}
			out->put(&current);
			if (querySchdl::verbose) cout << "stmt executed: " << name << endl;
			return s_success;
		}
	}
}

/*
   stmt_rc utStmt::exe_set_in() 
   {
   cout << "enter exe of timed union stmt " << name << endl;

   struct timeval tv1;  
   dbt *d = new cDBT("ddd", MAX_STR_LEN, &tv1);
   dbt* current = 0;
   bool empty = false;

//testing to see which input buffer can be used to take next tuple.
for (list<buffer*>::iterator itr = union_bufs.begin(); itr != union_bufs.end(); itr++) {
if ((*itr)->empty()) {
empty = true;
in = *itr;
break;
} else {
(*itr)->get(d);
cout << "tuple in timed union: ";
d->print();
if (current == 0) {
current = new cDBT(d);	
in = *itr;
cout << "earliest tuple so far: ";
current->print();	
} else if(current->atime->tv_sec > d->atime->tv_sec
|| (current->atime->tv_sec == d->atime->tv_sec
&& current->atime->tv_usec > d->atime->tv_usec)){
//take the smallest timestamp
current = new cDBT(d);
in = *itr;
cout << "earliest tuple so far: ";
current->print();
}
}
}

delete d;

if (empty) {
cout << "timed union, one buffer empty, will backtrack." << endl;
delete current;
return s_no_input;
} else {
if (out == 0) {
cout << "out buffer is null in statememt " << name << endl;
delete current;      
return s_failure;
} else {
cout << "out buffer is " << out->name << endl;
cout << "at the end of union, earliest tuple is: ";
current->print();
out->put(current);
delete current;
cout << "stmt executed: " << name << endl;
return s_success;
}
}
}
 */

//utlStmt::utlStmt() 
//{
//  type = stmt_tl_union;
//  in = 0;
//}

utlStmt::utlStmt(char *name, /*buffer* i,*/ buffer* o, bool sub_stmt)
{
	SMLog::SMLOG(10, "Entering utlStmt::utlStmt");	
	type = stmt_tl_union;
	strcpy(this->name, name);
	in = 0;
	out = o;
	this->inBufName[0] = '\0';
	strcpy(this->outBufName, o->name);  
	this->sub_stmt = sub_stmt;  
}

utlStmt::~utlStmt() 
{  
}

stmt_rc utlStmt::exe_set_in() 
{
	SMLog::SMLOG(10, "Entering utlStmt::exe_set_in");	
	if (querySchdl::verbose) cout << "enter exe of timeless union stmt " << name << endl;

	struct timeval tv1;
	cDBT d(MAX_STR_LEN, &tv1);

	list<buffer*>::iterator itr = union_bufs.begin();
	list<buffer*>::iterator itr2 = union_bufs.end();
	bool iter2assign = false;
	//&(itr2) = 0;
	bool in_found = false;

	//if all buffers are empty, then retract, otherwise, process a tuple
	//to ensure fairness, start actual checking from the buffer next in the list
	//of the previously used buffer (which was set as "in" for popping)
	while (itr != itr2) {
		if (in == 0) {
			if (itr2 == union_bufs.end()) {itr2 = union_bufs.begin();}
		} else if (!in_found) {
			//loop until I see the buffer that was last popped or retracked on
			if ((*itr) == in) {
				itr2 = itr;
				in_found = true;
			} else {
				itr++;
				continue;
			}
		}

		//now I am seeing the first buffer to check
		//printf("checking buf %s %d\n", (*itr)->name, (*itr)->empty());
		if ((*itr)->empty()) {
			itr++;
			if (itr == union_bufs.end()) {
				itr = union_bufs.begin();
			}
			continue;
		} else {
			//at least one buffer is not empty, get the tuple from it
			(*itr)->get(&d);
			if (querySchdl::verbose) {
				cout << "tuple to use in timeless union: ";
				d.print();
			}

			if (out == 0) {
				if (querySchdl::verbose) cout << "out buffer is null in statememt " << name << endl;
				return s_failure;
			} else {
				if (querySchdl::verbose) cout << "out buffer is " << out->name << endl;
				out->put(&d);
				if (querySchdl::verbose) cout << "stmt executed: " << name << endl;
				in = (*itr);  //singal to pop this buffer to the calling function
				return s_success;
			}
		}
	}

	if (querySchdl::verbose) cout << "timeless union, all buffer empty, will backtrack." << endl;
	in = (*itr2);
	return s_no_input;
}

cStmt::cStmt(char* name)
{
	SMLog::SMLOG(10, "Entering cStmt::cStmt");	
	type = stmt_coll;
	strcpy(this->name, name);
	this->sub_stmt = 0;
}

cStmt::cStmt(char* name, stmt* sub_stmt)
{
	SMLog::SMLOG(10, "Entering cStmt::cStmt 2");	
	type = stmt_coll;
	strcpy(this->name, name);
	addSubStmt(sub_stmt);
	this->sub_stmt = 0;
}

void cStmt::addSubStmt(stmt* s) {
	SMLog::SMLOG(10, "Entering cStmt::addSubStmt");	
	list<stmt*> toRemove;
	sub_stmts.push_back(s); 
	if(s->type == stmt_t_union || s->type == stmt_tl_union) {
		uStmt* s1 = (uStmt*)s;
		for(list<stmt*>::iterator itr = sub_stmts.begin(); itr!=sub_stmts.end();itr++) {
			bool found = false;
			for(list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2!=s1->sub_stmts.end() && found == false;itr2++) {
				if(strcmp((*itr2)->name, (*itr)->name) == 0) {
					found = true;
					toRemove.push_back((*itr));
				}
			}
		}
		for(list<stmt*>::iterator itr=toRemove.begin();itr!=toRemove.end();itr++) {
			sub_stmts.remove(*itr);
		}
	}
}

cStmt::~cStmt()
{
}

void cStmt::print() {
	fprintf(stderr, "coll stmt %s %d\n", name, sub_stmts.size());

	for (list<stmt*>::iterator itr = sub_stmts.begin(); itr != sub_stmts.end(); itr++) {
		(*itr)->print();
	}
	fprintf(stderr, "DONE coll stmt\n");
}


//jStmt::jStmt() 
//{
//  type = stmt_join;
//}

void jStmt::print() {
	fprintf(stderr, "%s: %s -> %s\n", name, inBufName, outBufName);
}

jStmt::jStmt(char *name, buffer* i, buffer* o, bool sub_stmt)
{
	SMLog::SMLOG(10, "Entering jStmt::jStmt");	
	type = stmt_join;
	strcpy(this->name, name);
	in = i;
	out = o;
	strcpy(this->inBufName, i->name);  
	strcpy(this->outBufName, o->name);  
	this->sub_stmt = sub_stmt;
	back_buf = NULL;
}

jStmt::~jStmt() 
{  
}

/* inlined
   stmt_rc jStmt::exe(int freeVar) 
   {
   back_buf = NULL;
   return exe_set_backbuf(freeVar);
   }
 */

stmt_rc jStmt::exe_set_backbuf(int freeVar) 
{
	SMLog::SMLOG(10, "Entering jStmt::exe_set_backbuf");	
	if (querySchdl::verbose) cout << "enter exe of join stmt " << name << endl;

	//only to free common state
	if (freeVar == 1) {
		if (func) {
			if (querySchdl::verbose) cout << "execute stmt: " << name << endl;    
			return (stmt_rc) ((*func)(bufferMngr::getInstance(), freeVar, back_buf, getInMemTables()));
		} else {
			cerr<<"Warning: stmt not ready"<<endl;
			return s_failure;
		}
	}

	//here freeVar = 0, normal execution
	//if in buffer is empty, set back_buf to in, and return no-input.
	//Otherwise, let the join function to decide.
	if (in->empty()) {
		back_buf = in;
		return s_no_input;
	} else {
		if (func) {
			if (querySchdl::verbose) cout << "execute stmt: " << name << endl;    
			return (stmt_rc) ((*func)(bufferMngr::getInstance(), freeVar, back_buf, getInMemTables()));
		} else {
			cerr<<"Warning: stmt not ready"<<endl;
			return s_failure;
		}

		//return join(this);
	}
}


//this is for initial dummy testing only, no longer needed
//return s_no_input if window is not complete, and set the back_buf of stmt to the window that needs backtracking.
//return s_no_output if window is complete but join operation produces nothing
//return s_success if window is complete and join operation produces some tuple
//return s_failure if any error happens
stmt_rc join(jStmt* stmt) 
{
	//for now make this work like a timed union, for testing.
	struct timeval tv1;  
	dbt *d = new cDBT("ddd", MAX_STR_LEN, &tv1);
	dbt* current = 0;
	buffer* cb = 0;
	bool empty = false;
	list<buffer*> all_bufs;
	all_bufs.push_back(stmt->in);
	for (list<buffer*>::iterator itr = stmt->window_bufs.begin(); itr != stmt->window_bufs.end(); itr++) {
		all_bufs.push_back(*itr);
	}

	//testing to see which input buffer can be used to take next tuple.
	for (list<buffer*>::iterator itr = all_bufs.begin(); itr != all_bufs.end(); itr++) {
		if ((*itr)->empty()) {
			empty = true;
			stmt->back_buf = *itr;
			break;
		} else {
			(*itr)->get(d);
			cout << "tuple in join: ";
			d->print();
			if (current == 0) {
				current = new cDBT(d);	
				cb = *itr;
				cout << "earliest tuple so far: ";
				current->print();	
			} else if(current->atime->tv_sec > d->atime->tv_sec
					|| (current->atime->tv_sec == d->atime->tv_sec
						&& current->atime->tv_usec > d->atime->tv_usec)){
				//take the smallest timestamp
				current = new cDBT(d);
				cb = *itr;
				cout << "earliest tuple so far: ";
				current->print();
			}
		}
	}

	delete d;

	if (empty) {
		cout << "join, one buffer empty, will backtrack." << endl;
		delete current;
		return s_no_input;
	} else {
		if (stmt->out == 0) {
			cout << "out buffer is null in statememt " << stmt->name << endl;
			delete current;      
			return s_failure;
		} else {
			cout << "out buffer is " << stmt->out->name << endl;
			cout << "at the end of join, earliest tuple is: ";
			current->print();
			stmt->out->put(current);
			delete current;

			//simulate the window management here. Otherwise, the calling function will do the pop
			if (cb != stmt->in) {
				//the tuple I took was from one of the window buffers
				cb->pop();
				cout << "window buffer poped:" << cb->name << endl;	
			}

			cout << "stmt executed: " << stmt->name << endl;
			return s_success;
		}
	}
}














