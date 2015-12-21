#include "driver.h"
//#include "adllib.h"
#include "basic.h"
#include "const.h"
#include "ios/ios.h"
#include "util.h"
#include "querySchdl.h"
#include "strategy.h"
#include "monitor.h"

extern "C"{
#include <dbug.h>
}
//using namespace ESL;

namespace ESL {

	extern bool monitor_exp_mode;

	void _dg(char* msg) 
	{
		if (querySchdl::verbose) cout << msg << endl;
	}

	buffer_entry::buffer_entry(buffer_entry* entry) 
	{
		querySchdl::SMLOG(10, "Entering buffer_entry::buffer_entry");	
		this->b_type = entry->b_type;
		this->statement = entry->statement;
		this->buf = entry->buf;
		if (this->b_type == t_merge || this->b_type == t_merge_sink || this->b_type == t_merge_fork) {
			this->back.union_stmt = entry->back.union_stmt;
		} else if (this->b_type == t_fork_dummy) {
			this->back.processed = entry->back.processed;
		} else {
			this->back.back_buf = entry->back.back_buf;
		}

		this->inplace = entry->inplace;

		if(this->b_type == t_fork || this->b_type == t_source_fork || this->b_type == t_merge_fork) {
			this->forward.fork_count = entry->forward.fork_count;
		} else {
			this->forward.forward_buf = entry->forward.forward_buf;
		}

		this->action = entry->action;
	}

	int buffer_entry::pop() 
	{
		querySchdl::SMLOG(10, "Entering buffer_entry::pop");	
		//if it is not a fork dummy buffer, then pop the buffer
		//else, set the flag for the dummy
		if (b_type == t_fork_dummy) {
			back.processed = true;
			_dg("poped forkdummy");
			return SUCCESS;
		} else {
			_dg("poped");
			_dg(buf->name);
			if (buf->empty()) _dg("empty"); else _dg("not empty");	

			return buf->pop();
		}
	}

	Driver::Driver(int prty):priority(prty), id(0), dirty(false),stateTableChanged(false)
	{  
		querySchdl::SMLOG(10, "Entering Driver::Driver");	
		bufStateTable = new BufStateTblType();
		stmts = new list<stmt*>();
		bufs = new list<buffer*>();
		//last_run = NULL;
		last_run_name[0] = '\0';
		strategy = NULL;
		weightedStrategy = NULL;

		if (monitor_exp_mode) priority = 30000;
	}

	Driver::~Driver()
	{
		querySchdl::SMLOG(10, "Entering Driver::~Driver");	
		_dg("destructor ~Driver entered. ");
		//if (querySchdl::verbose) cout << "state table size is: " << bufStateTable->size() << endl;
		list<char*> dummies;
		int i = 0;

		for (BufStateTblType::iterator itr = bufStateTable->begin(); itr != bufStateTable->end(); ++itr, ++i) {
			if (itr->second->b_type == t_fork_dummy) {
				//dummy buffer names are allocated in this class
				//delete itr->first;
				//can not delete here, desrupts itr. store it, and delete later.
				dummies.push_back(itr->first);
			}

			delete itr->second;
			//if (querySchdl::verbose) cout << i << " ";
			//if (i>1000) break;
		}

		for (list<char*>::iterator itr2 = dummies.begin(); itr2 != dummies.end(); ++itr2) {
			delete (*itr2);
		}

		delete bufStateTable;

		for (list<stmt*>::iterator itr = stmts->begin(); itr != stmts->end(); itr++) {
			(*itr)->weightSet = false;
			(*itr)->weight = WeightProfile::WEIGHT_MIN;
			(*itr)->rankSet = false;
			(*itr)->rank.priority = OperatorPriorityStrategy::MAX_PRIORITY;
		}

		delete stmts;
		delete bufs;

		delete strategy;
		delete weightedStrategy;

		//_dg("destructor ~Driver exited");
	}

	void Driver::sendErrorForStmt(stmt* s) 
	{
		querySchdl::SMLOG(10, "Entering Driver::sendErrorForStmt");	
		int messageSize = 2*MAX_ID_LEN + 100;
		cDBT cdbt(messageSize);
		char msg[messageSize];

		//the trick is that if I am a sub-statement, I need to find out who the main stmt is
		//if my output buffer is a fork, or a merge, or a sink, or combination of the three, then I am definitely not a substatement
		//otherwise, have to check
		b_entry state = (*bufStateTable)[s->out->name];
		if (state->b_type == t_fork || state->b_type ==  t_merge || state->b_type == t_sink
				|| state->b_type == t_merge_sink || state->b_type == t_merge_fork) {
			sprintf(msg, "Error executing query %s", s->name);
		} else if (state->b_type == t_normal) {
			//look at my next stmt, if it is a union, use it, if it is a join, do a confirmation, since there could be a non-substatement upstream of a join
			stmt* s1 = state->statement;

			if (s1->type == stmt_t_union || s1->type == stmt_tl_union) {
				sprintf(msg, "Error executing query %s(at sub query %s)", s1->name, s->name);
			} else if (s1->type == stmt_join) {
				jStmt* s2 = static_cast<jStmt*> (s1);
				bool found = false;
				for (list<buffer*>::iterator itr = s2->window_bufs.begin(); itr != s2->window_bufs.end(); itr++) {
					if (strcmp(s->name, (*itr)->name) == 0) {
						found = true;
					}
				}

				if (found) {
					//this error statement is indeed a substatement of a join
					sprintf(msg, "Error executing query %s(at sub query %s)", s1->name, s->name);
				} else {
					//this is not a substatement
					sprintf(msg, "Error executing query %s", s->name);
				}
			} else {
				//normal case
				sprintf(msg, "Error executing query %s", s->name);
			}

		} else {
			//error
			if (querySchdl::verbose) cout << "wrong output buffer type for stmt name: " << s->name << endl;
			return;
		}  

		strcpy(msg, (char*)cdbt.data);

		//change to sdbt???

		//return failure message to stdout_error buffer
		bufferMngr *bm = bufferMngr::getInstance();
		bm->put("stdout_error", &cdbt);

	}


	void Driver::setId(int id) 
	{
		this->id = id;
		return;
	}

	int Driver::getId() 
	{
		return this->id;
	}

	void Driver::setPriority(int p) 
	{
		priority = p;
		return;
	}

	int Driver::getPriority() 
	{
		return this->priority;
	}

	void Driver::setStrategy(Strategy* strategy) {
		this->strategy = strategy; 
		//strategy->setDriver(this);
	}

	void Driver::setStrategy(ScheduleStrategy* strategy) {
		this->weightedStrategy = strategy; 
		//strategy->setDriver(this);
	}

	BufStateTblType* Driver::getBufGraph()
	{
		return this->bufStateTable;
	}

	list<stmt*>* Driver::getStmts()
	{
		return this->stmts;
	}

	list<buffer*>* Driver::getBufs()
	{
		return this->bufs;
	}

	stmt* Driver::getStmtByName(char* name) 
	{
		if (name == NULL) return NULL;

		for (list<stmt*>::iterator itr = stmts->begin(); itr != stmts->end(); itr++) {
			if (strcmp((*itr)->name, name) == 0) return *itr;
		}

		return NULL;
	}

	buffer* Driver::getBufByName(char* name) 
	{
		if (name == NULL) return NULL;

		for (list<buffer*>::iterator itr = bufs->begin(); itr != bufs->end(); itr++) {
			if (strcmp((*itr)->name, name) == 0) return *itr;
		}

		return NULL;
	}

	int Driver::addStmt(stmt* s) 
	{
		querySchdl::SMLOG(10, "Entering Driver::addStmt");	
		DBUG_ENTER("Driver::addStmt");

		//check to see if this statement is valid (since we allow deletion of streams used by statements, the statement might not be valid)
		if (!(s->valid)) {
			DBUG_RETURN(FAILURE);    
		}

		//if I have not got anything in this driver yet, just add it in
		b_entry entry;

		if (stmts->empty()) {
			if (s->type == stmt_normal /*|| s->type == stmt_join*/) {
				//add "in" as source
				entry = new buffer_entry();
				entry->b_type = t_source;
				entry->statement = s;
				entry->buf = s->in;
				entry->back.back_buf = 0;
				entry->inplace = s->in->name;
				entry->forward.forward_buf = s->out->name;
				entry->action = act_cjump;
				(*bufStateTable)[s->in->name] = entry;

				//add "out" as sink
				entry = new buffer_entry();
				entry->b_type = t_sink;
				entry->statement = 0;
				entry->buf = s->out;
				entry->back.back_buf = s->in->name;
				entry->inplace = 0;
				entry->forward.forward_buf = 0;
				entry->action = act_none;
				(*bufStateTable)[s->out->name] = entry;

				bufs->push_back(s->in);
				//entryPoint = s->in;
				bufs->push_back(s->out);      
			} else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
				uStmt* s1 = static_cast<uStmt*> (s);
				for (list<buffer*>::iterator itr = s1->union_bufs.begin(); itr != s1->union_bufs.end(); itr++) {
					//add all in bufs as sources
					entry = new buffer_entry();
					entry->b_type = t_source;
					entry->statement = s;
					entry->buf = *itr;
					entry->back.back_buf = 0;
					entry->inplace = (*itr)->name;
					entry->forward.forward_buf = s->out->name;
					entry->action = act_cjump;
					(*bufStateTable)[(*itr)->name] = entry;

					bufs->push_back(*itr);
					//if (entryPoint == 0) {
					//  entryPoint = *itr;
					//}	
				}
				//add "out" as merge_sink
				entry = new buffer_entry();
				entry->b_type = t_merge_sink;
				entry->statement = 0;
				entry->buf = s->out;
				entry->back.union_stmt = s;
				entry->inplace = 0;
				entry->forward.forward_buf = 0;
				entry->action = act_merge;
				(*bufStateTable)[s->out->name] = entry;

				bufs->push_back(s->out);      
			} else if (s->type == stmt_join) {
				jStmt* s1 = static_cast<jStmt*> (s);
				//first add "in", then all the window buffers
				entry = new buffer_entry();
				entry->b_type = t_source;
				entry->statement = s;
				entry->buf = s->in;
				entry->back.back_buf = 0;
				entry->inplace = s->in->name;
				entry->forward.forward_buf = s->out->name;
				entry->action = act_cjump;
				(*bufStateTable)[s->in->name] = entry;

				for (list<buffer*>::iterator itr = s1->window_bufs.begin(); itr != s1->window_bufs.end(); itr++) {
					//add all window bufs as sources
					entry = new buffer_entry();
					entry->b_type = t_source;
					entry->statement = s;
					entry->buf = *itr;
					entry->back.back_buf = 0;
					entry->inplace = (*itr)->name;
					entry->forward.forward_buf = s->out->name;
					entry->action = act_cjump;
					(*bufStateTable)[(*itr)->name] = entry;

					bufs->push_back(*itr);
					//if (entryPoint == 0) {
					//  entryPoint = *itr;
					//}	
				}
				//add "out" as merge_sink
				entry = new buffer_entry();
				entry->b_type = t_merge_sink;
				entry->statement = 0;
				entry->buf = s->out;
				entry->back.union_stmt = s;
				entry->inplace = 0;
				entry->forward.forward_buf = 0;
				entry->action = act_merge;
				(*bufStateTable)[s->out->name] = entry;

				bufs->push_back(s->out);      
			} else {
				if (querySchdl::verbose) cout << "unknown statement type for stmt name: " << s->name << endl;
				DBUG_RETURN(FAILURE);;
			}
			stmts->push_back(s);    
			setDirty();
			DBUG_RETURN(SUCCESS);;
		}

		buffer_ty in_type = t_none;  
		//buffer_ty out_type = t_none;
		b_entry dummy = 0;
		char* dummy_name = 0;
		char* old_forward = 0;
		list<buffer*> in_bufs;

		//I need to modify existing table, possibly add entries.
		if (s->type == stmt_normal) {
			in_bufs.push_back(s->in);
		} else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
			//I have lots of input buffers. test one by one
			uStmt* s1 = static_cast<uStmt*> (s);
			in_bufs = s1->union_bufs;
		} else if (s->type == stmt_join) {
			//need to test "in" buffer and all the window buffers    
			jStmt* s1 = static_cast<jStmt*> (s);
			in_bufs = s1->window_bufs;
			in_bufs.push_back(s->in);
		} else {
			cerr << "unknown statement type for stmt name: " << s->name << endl;
			DBUG_RETURN(FAILURE);
		}

		for (list<buffer*>::iterator itr = in_bufs.begin(); itr != in_bufs.end(); itr++) {

			if (bufStateTable->find((*itr)->name) == bufStateTable->end()) {
				//this is a new entry
				//add "in" as source
				entry = new buffer_entry();
				entry->b_type = t_source;
				entry->statement = s;
				entry->buf = (*itr);
				entry->back.back_buf = 0;
				entry->inplace = (*itr)->name;
				entry->forward.forward_buf = s->out->name;
				entry->action = act_cjump;
				(*bufStateTable)[(*itr)->name] = entry;

				//if (entryPoint == 0) {
				//entryPoint = (*itr);
				//}

				bufs->push_back((*itr));      
				in_type = t_source;
			} else {

				//I need to modify an existing entry
				b_entry entry = (*bufStateTable)[(*itr)->name];
				b_entry entry2;

				//make sure out buffer is fine to use, before make any modifications here
				if (s->type == stmt_normal /* || s->type == stmt_join*/) {

					if (bufStateTable->find(s->out->name) != bufStateTable->end()) {
						//if (out_type == t_sink) {
						//  if (querySchdl::verbose) cout
						//    << "error, produces into an existing buffer, but consumes from an original sink." << endl;
						//  DBUG_RETURN(FAILURE);;
						//}

						//I need to modify an existing entry, which surely should have been a source.	  
						entry2 = (*bufStateTable)[s->out->name];
						if (entry2->b_type != t_source && entry2->b_type != t_source_fork) {
							if (querySchdl::verbose) cout
								<< "error, produces into an existing buffer which is not a source." << endl;
							DBUG_RETURN(FAILURE);;
						}
					}
				} else {

					//now that I am a union or join statement, the output buffer is a merge buffer.
					if (bufStateTable->find(s->out->name) != bufStateTable->end()) {
						//if (out_type == t_sink) {
						//  if (querySchdl::verbose) cout
						//    << "error, produces into an existing buffer, but consumes from an original sink." << endl;
						//  DBUG_RETURN(FAILURE);;
						//}
						//I need to modify an existing entry, which surely should have been a source.	  
						entry2 = (*bufStateTable)[s->out->name];
						if (entry2->b_type != t_source && entry2->b_type != t_source_fork) {
							if (querySchdl::verbose) cout
								<< "error, produces into an existing buffer which is not a source." << endl;
							DBUG_RETURN(FAILURE);;
						}
					}    
				}


				// cout << "entry type is " << entry->b_type << ", buffer name is " << entry->buf->name << endl;

				switch (entry->b_type) {
					case t_source:
					case t_normal:
					case t_merge:
						//one more reading statement makes this a fork buffer
						if (s->type == stmt_join && (*itr) != s->in) {
							//currently window buffers are not supposed to be shared into a fork
							if (querySchdl::verbose) cout << "window buffer sharing is not allowed now, buffer name: " << (*itr)->name << endl;
							//return FAILURE;
							DBUG_RETURN(FAILURE);
						}

						entry = (*bufStateTable)[(*itr)->name];
						if (entry->b_type == t_source) {  
							entry->b_type = t_source_fork;
							in_type = t_source_fork;
							entry->action = act_cfork;	  
						} else if (entry->b_type == t_normal) {  
							entry->b_type = t_fork;
							in_type = t_fork;
							entry->action = act_cfork;	  
						} else if (entry->b_type == t_merge) {  
							entry->b_type = t_merge_fork;
							in_type = t_merge_fork;
							entry->action = act_merge_cfork;	  
						}

						old_forward = entry->forward.forward_buf;
						entry->forward.fork_count = 2;

						//the existing next buffer needs to have a new dummy for it
						//make the dummy buffer named as concat of "in" name and "forward" name
						dummy = new buffer_entry();
						dummy_name = new char[strlen((*itr)->name)+strlen(old_forward)+1];
						strcpy(dummy_name, (*itr)->name);
						strcat(dummy_name, old_forward);

						(*bufStateTable)[dummy_name] = dummy;
						dummy->b_type = t_fork_dummy;
						dummy->buf = 0; //dummy buffer, does not exist
						dummy->statement = entry->statement;
						dummy->back.processed = false;
						dummy->inplace = (*itr)->name;
						dummy->forward.forward_buf = old_forward;
						dummy->action = act_jump;

						//now also change the original next entry's "back" value, to be the dummy.
						//unless it is a merge buffer, then we do not need to change it.
						if ((*bufStateTable)[old_forward]->b_type != t_merge &&
								(*bufStateTable)[old_forward]->b_type != t_merge_fork &&
								(*bufStateTable)[old_forward]->b_type != t_merge_sink) {
							(*bufStateTable)[old_forward]->back.back_buf = dummy_name;
						}

						//if (querySchdl::verbose) {
						//cout << "inside addstmt, dummy name " << dummy_name << endl;
						//this->printStateTable();
						//}

						//also add a new dummy for out buffer here.
						dummy = new buffer_entry();
						dummy_name = new char[strlen((*itr)->name)+strlen(s->out->name)+1];
						strcpy(dummy_name, (*itr)->name);
						strcat(dummy_name, s->out->name);

						(*bufStateTable)[dummy_name] = dummy;
						dummy->b_type = t_fork_dummy;
						dummy->buf = 0; //dummy buffer, does not exist
						dummy->statement = s;
						dummy->back.processed = false;
						dummy->inplace = (*itr)->name;
						dummy->forward.forward_buf = s->out->name;
						dummy->action = act_jump;

						//if (querySchdl::verbose) {
						//cout << "inside addstmt, dummy name " << dummy_name << endl;
						//this->printStateTable();
						//}

						break;

					case t_fork:
					case t_source_fork:
					case t_merge_fork:
						//already a fork buffer, just increment the count, and add in a new dummy for out.
						entry = (*bufStateTable)[(*itr)->name];
						if (entry->b_type == t_source_fork) {
							in_type = t_source_fork;	  
						} else if (entry->b_type == t_fork) {
							in_type = t_fork;
						} else if (entry->b_type == t_merge_fork) {
							in_type = t_merge_fork;
						}
						(entry->forward.fork_count)++;

						dummy = new buffer_entry();
						dummy_name = new char[strlen((*itr)->name)+strlen(s->out->name)+1];
						strcpy(dummy_name, (*itr)->name);
						strcat(dummy_name, s->out->name);

						(*bufStateTable)[dummy_name] = dummy;
						dummy->b_type = t_fork_dummy;
						dummy->buf = 0; //dummy buffer, does not exist
						dummy->statement = s;
						dummy->back.processed = false;
						dummy->inplace = (*itr)->name;
						dummy->forward.forward_buf = s->out->name;
						dummy->action = act_jump;

						break;

					case t_sink:
					case t_merge_sink:

						//my output will have to replace the original to be a new sink
						//out_type = t_sink;

						entry = (*bufStateTable)[(*itr)->name];
						entry->statement = s;
						if (entry->b_type == t_sink) {  
							entry->b_type = t_normal;
							in_type = t_normal;
							entry->action = act_cjump;	  
						} else if (entry->b_type == t_merge_sink) {  
							entry->b_type = t_merge;
							in_type = t_merge;
							entry->action = act_merge_jump;
						}
						entry->inplace = (*itr)->name;
						entry->forward.forward_buf = s->out->name;

						break;

					default:
						if (querySchdl::verbose) cout << "error recognizing buffer state type: " << entry->b_type << endl;
						DBUG_RETURN(FAILURE);;
				}
			}
		}

		//now handle "out" buffer
		if (s->type == stmt_normal /* || s->type == stmt_join*/) {  
			if (bufStateTable->find(s->out->name) == bufStateTable->end()) {
				//this is a new entry
				if (in_type != t_fork && in_type != t_source_fork && in_type != t_merge_fork) {
					//add "out" as sink, nothing special here
					entry = new buffer_entry();
					entry->b_type = t_sink;
					entry->statement = 0;
					entry->buf = s->out;
					entry->back.back_buf = s->in->name;
					entry->inplace = 0;
					entry->forward.forward_buf = 0;
					entry->action = act_none;
					(*bufStateTable)[s->out->name] = entry;
				} else {
					//add "out" as sink, but set "back" to the created dummy.
					entry = new buffer_entry();
					entry->b_type = t_sink;
					entry->statement = 0;
					entry->buf = s->out;
					entry->back.back_buf = dummy_name;
					entry->inplace = 0;
					entry->forward.forward_buf = 0;
					entry->action = act_none;
					(*bufStateTable)[s->out->name] = entry;	
				}

				bufs->push_back(s->out);
			} else {
				//if (out_type == t_sink) {
				//if (querySchdl::verbose) cout << "error, produces into an existing buffer, but consumes from an original sink." << endl;
				//DBUG_RETURN(FAILURE);;
				//}

				//I need to modify an existing entry, which surely should have been a source.      
				entry = (*bufStateTable)[s->out->name];
				if (entry->b_type != t_source && entry->b_type != t_source_fork) {
					if (querySchdl::verbose) cout << "error, produces into an existing buffer which is not a source." << endl;
					DBUG_RETURN(FAILURE);;
				}

				if (entry->b_type == t_source) {  
					entry->b_type = t_normal;
				} else if (entry->b_type == t_source_fork) {  
					entry->b_type = t_fork;
				}

				if (in_type != t_fork && in_type != t_source_fork && in_type != t_merge_fork) {
					entry->back.back_buf = s->in->name;
				} else {
					entry->back.back_buf = dummy_name;	
				}
			}
		} else {
			//now that I am a union or join statement, the output buffer is a merge buffer.
			if (bufStateTable->find(s->out->name) == bufStateTable->end()) {
				//this is a new entry
				//add "out" as merge sink
				entry = new buffer_entry();
				entry->b_type = t_merge_sink;
				entry->statement = 0;
				entry->buf = s->out;
				entry->back.union_stmt = s;
				entry->inplace = 0;
				entry->forward.forward_buf = 0;
				entry->action = act_merge;
				(*bufStateTable)[s->out->name] = entry;

				bufs->push_back(s->out);      
			} else {
				//if (out_type == t_sink) {
				//if (querySchdl::verbose) cout << "error, produces into an existing buffer, but consumes from an original sink." << endl;
				//DBUG_RETURN(FAILURE);;
				//}

				//I need to modify an existing entry, which surely should have been a source.      
				entry = (*bufStateTable)[s->out->name];
				if (entry->b_type != t_source && entry->b_type != t_source_fork) {
					if (querySchdl::verbose) cout << "error, produces into an existing buffer which is not a source." << endl;
					DBUG_RETURN(FAILURE);;
				}

				if (entry->b_type == t_source) {  
					entry->b_type = t_merge;
					entry->action = act_merge_jump;
				} else if (entry->b_type == t_source_fork) {  
					entry->b_type = t_merge_fork;
					entry->action = act_merge_cfork;
				}
				entry->back.union_stmt = s;
			}    
		}

		stmts->push_back(s);  
		setDirty();
		DBUG_RETURN(SUCCESS);;
	}

	//drop stmt, also need to modify the bufToDrvMap in DrvMgr,
	//since only driver knows when a buffer is used only in one statement thus requires removal of its entry
	int Driver::dropStmt(stmt *s){
		querySchdl::SMLOG(10, "Entering Driver::dropStmt");	
		DBUG_ENTER("Driver::dropStmt");
		list<stmt*>::iterator itr3;
		/*
		   if ((itr3 = find(stmts->begin(), stmts->end(), s)) == stmts->end()) {
		   cerr << "stmt does not exist in driver, internal inconsistency" << endl;
		   DBUG_RETURN(FAILURE);
		   }
		 */
		bool found = false;
		for (itr3 = stmts->begin(); itr3 != stmts->end(); itr3++) {
			if (strcmp((*itr3)->name,s->name) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			cerr << "stmt does not exist in driver, internal inconsistency" << endl;
			DBUG_RETURN(FAILURE);    
		}

		b_entry entry = 0;  
		char* dummy_name = 0;
		list<buffer*> in_bufs;
		pair<BufMapType::iterator, BufMapType::iterator> p;
		BufStateTblType::iterator itr2, itr_tmp;

		if (s->type == stmt_normal) {
			in_bufs.push_back(s->in);
		} else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
			//I have lots of input buffers. test one by one
			uStmt* s1 = static_cast<uStmt*> (s);
			in_bufs = s1->union_bufs;
		} else if (s->type == stmt_join) {
			//need to test "in" buffer and all the window buffers    
			jStmt* s1 = static_cast<jStmt*> (s);
			in_bufs = s1->window_bufs;
			in_bufs.push_back(s->in);
		} else {
			cerr << "unknown statement type for stmt name: " << s->name << endl;
			DBUG_RETURN(FAILURE);
		}

		for (list<buffer*>::iterator itr = in_bufs.begin(); itr != in_bufs.end(); itr++) {
			entry = (*bufStateTable)[(*itr)->name];

			switch (entry->b_type) {

				case t_source:
					//single source, buffer entry should be removed too
					bufs->remove(entry->buf);
					bufStateTable->erase((*itr)->name);

					//also need to remove entry in DrvMgr for this driver.
					p = DrvMgr::getInstance()->bufToDrvMap->equal_range((*itr)->name);
					while (p.first != p.second) {
						if ((*(p.first)).second == this) {
							DrvMgr::getInstance()->bufToDrvMap->erase(p.first);
							break;
						}	
						++(p.first);
					}

					break;

				case t_normal:
					//this buffer will become sink for the previous statement
					entry->b_type = t_sink;
					entry->statement = 0;
					entry->inplace = 0;
					entry->forward.forward_buf = 0;
					entry->action = act_none;

					break;

				case t_merge:
					//this buffer will become merge sink for the previous statement
					entry->b_type = t_merge_sink;
					entry->statement = 0;
					entry->inplace = 0;
					entry->forward.forward_buf = 0;
					entry->action = act_merge;

					break;

				case t_fork:
				case t_source_fork:
				case t_merge_fork:
					//remove my corresponding fork dummy entry
					//and change the entry back to normal/source/merge if currently there are only two fork paths
					(entry->forward.fork_count)--;

					dummy_name = new char[strlen((*itr)->name)+strlen(s->out->name)+1];
					strcpy(dummy_name, (*itr)->name);
					strcat(dummy_name, s->out->name);

					bufStateTable->erase(dummy_name);

					//if (querySchdl::verbose) this->printStateTable();

					if (entry->forward.fork_count ==1) {
						//need to remove the last dummy, and change this back to a normal buffer
						for (itr2 = bufStateTable->begin(); itr2 != bufStateTable->end(); itr2++) {
							if ((itr2->second->b_type == t_fork_dummy) &&
									(strcmp(itr2->second->inplace, entry->buf->name) == 0)) {
								if (entry->b_type == t_fork) {
									entry->b_type = t_normal;
									entry->action = act_cjump;
								} else if (entry->b_type == t_source_fork) {
									entry->b_type = t_source;
									entry->action = act_cjump;
								} else if (entry->b_type == t_merge_fork) {
									entry->b_type = t_merge;
									entry->action = act_merge_jump;
								}

								entry->statement = itr2->second->statement;
								entry->forward.forward_buf = itr2->second->forward.forward_buf;
								((*bufStateTable)[itr2->second->forward.forward_buf])->back.back_buf =
									entry->buf->name;
								//bufStateTable->erase(itr2); //move erase to outside, otherwise iterator problem.
								itr_tmp = itr2;
							}
						}
						bufStateTable->erase(itr_tmp);
					}

					break;

				case t_sink:
				case t_merge_sink:	
				default:
					cerr << "wrong buffer state type for an input buffer: " << entry->b_type << endl;
					DBUG_RETURN(FAILURE);;
			}
		}

		//now handle "out" buffer
		entry = (*bufStateTable)[s->out->name];

		switch (entry->b_type) {

			case t_sink:
			case t_merge_sink:
				//single sink, buffer entry should be removed too
				bufs->remove(entry->buf);
				bufStateTable->erase(s->out->name);

				//also need to remove entry in DrvMgr for this driver.
				p = DrvMgr::getInstance()->bufToDrvMap->equal_range(s->out->name);
				while (p.first != p.second) {
					if ((*(p.first)).second == this) {
						DrvMgr::getInstance()->bufToDrvMap->erase(p.first);
						break;
					}	
					++(p.first);
				}

				break;

			case t_normal:
			case t_merge:    
				//this buffer will become source for the next statement
				entry->b_type = t_source;
				entry->back.back_buf = 0;
				entry->action = act_cjump;    

				break;

			case t_fork:
			case t_merge_fork:
				//this buffer will become source fork
				entry->b_type = t_source_fork;
				entry->back.back_buf = 0;
				entry->action = act_cfork; 

				break;

			case t_source_fork:      
			case t_source:
			default:
				cerr << "wrong buffer state type for an output buffer: " << entry->b_type << endl;
				DBUG_RETURN(FAILURE);
		}

		//finally, remove this statement from list, and reset its stat values
		(*itr3)->weightSet = false;
		(*itr3)->weight = WeightProfile::WEIGHT_MIN;
		(*itr3)->rankSet = false;
		(*itr3)->rank.priority = OperatorPriorityStrategy::MAX_PRIORITY;

		stmts->erase(itr3);
		setDirty();

		DBUG_RETURN(SUCCESS);
	};

	/* no longer used: driver now only keeps the original query graph, segmented graphs are kept by strategies.
	//the driver should be broken into two, after splitting.
	int Driver::setBreakPnt(list<buffer*>* bufs) 
	{
	DBUG_ENTER("Driver::setBreakPnt");

	//first reset all marks, to make sure.
	resetMarks();

	//do BFS on break points, to discover reachable buffers, to break the driver.
	list<char*> q;
	b_entry entry;

	for (list<buffer*>::iterator itr = bufs->begin(); itr!=bufs->end(); itr++) {

	entry = (*bufStateTable)[(*itr)->name];

	if (entry->b_type != t_source && entry->b_type != t_source_fork && entry->b_type != t_sink && entry->b_type != t_merge_sink && entry->b_type != t_fork_dummy) {

//disallow union statement breakup
if (entry->statement->type == stmt_t_union || entry->statement->type == stmt_tl_union) {
if (querySchdl::verbose) cout << "breakpoint set on union internal buffer, error." << endl;
DBUG_RETURN(FAILURE);
} else {
entry->mark = m_break;
q.push_back((*itr)->name);
}
}
}


if (q.empty()) {
if (querySchdl::verbose) {
cerr << "buffers can not be set break point on. " << endl;
}
DBUG_RETURN(FAILURE);
}

while (!q.empty()) {
char* buf = q.front();
q.pop_front();
entry = (*bufStateTable)[buf];
if (entry->b_type == t_sink || entry->b_type == t_merge_sink) {
continue;
} else if (entry->b_type != t_fork && entry->b_type != t_source_fork && entry->b_type != t_merge_fork) {
if ((*bufStateTable)[entry->forward.forward_buf]->mark == m_up) {
(*bufStateTable)[entry->forward.forward_buf]->mark = m_down;
q.push_back(entry->forward.forward_buf);
}
continue;
} else {
	//this is a fork
	int nFork = entry->forward.fork_count;
	//now find out the dummy states, the # should agree with nFork
	int cnt = 0;
	for (BufStateTblType::iterator itr2 = bufStateTable->begin(); itr2 != bufStateTable->end(); itr2++) {
	if (itr2->second->b_type != t_fork_dummy) continue;
	else if (strcmp(itr2->second->inplace, buf) == 0) {
	if (itr2->second->mark == m_up) {
	itr2->second->mark = m_down;
	q.push_back(itr2->first);
	}
	cnt++;
	} 
	}
	if (nFork != cnt) {
	if (querySchdl::verbose) cout
	<< "internal error, number of dummy states does not match the fork count for buffer: "
	<< buf << endl;
	DBUG_RETURN(FAILURE);
	}
	continue;      
}
}

//at this point, all buffers marked as m_down  belong to the set downstream of the breakpoints.
//make sure that no buffers marked as m_up are adjacent to any node that is marked m_down
for (BufStateTblType::iterator itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
	if (itr->second->mark == m_up) {
		if (itr->second->b_type == t_sink || itr->second->b_type == t_merge_sink) {
			continue;
		} else if (itr->second->b_type != t_fork && itr->second->b_type != t_source_fork
				&& itr->second->b_type != t_merge_fork) {
			if ((*bufStateTable)[itr->second->forward.forward_buf]->mark == m_down) {
				//problem, the breakpoints did not completely divide the driver into two parts
				if (querySchdl::verbose) cout << "breakpoints did not completely divide driver." << endl;
				DBUG_RETURN(FAILURE);
			}
			continue;
		} else {
			//this is a fork, check every dummy, as well as dummy's next.
			for (BufStateTblType::iterator itr3 = bufStateTable->begin();
					itr3 != bufStateTable->end(); itr3++) {
				if (itr3->second->b_type != t_fork_dummy) continue;
				else if (strcmp(itr3->second->inplace, itr->first) == 0) {
					if (itr3->second->mark == m_down) {
						//this should never happen
						if (querySchdl::verbose) cout << "breakpoints did not completely divide driver." << endl;
						DBUG_RETURN(FAILURE);
					} else if ((*bufStateTable)[itr3->second->forward.forward_buf]->mark == m_down) {
						//problem, the breakpoints did not completely divide the driver into two parts
						if (querySchdl::verbose) cout << "breakpoints did not completely divide driver." << endl;
						DBUG_RETURN(FAILURE);
					}
				}  
			}
			continue; 
		}
	}
}

//passed division test. now need to actually build the two resulting drivers.
//all the buffers marked as m_down goes together, marked with m_up goes together, each plus the breakpoints
//DrvMgr mapping information needs to be updated too.
if (querySchdl::verbose) {
	cout << "driver table before splitting: " << endl;
	this->printStateTable();
}

Driver* drv1 = new Driver();
drv1->setPriority(this->priority);
int d_id = ++DrvMgr::id_count;
drv1->setId(d_id);
DrvMgr* mgr = DrvMgr::getInstance();
mgr->drivers->push_back(drv1);
//this->last_run = NULL;
this->last_run_name[0] = '\0';

list<stmt*> remove;
for (list<stmt*>::iterator itr = stmts->begin(); itr!= stmts->end(); itr++) {
	//test the in buffer.
	//if up, the statement remains in this driver. If down or breakpoint, then it is in drv1
	if ((*bufStateTable)[(*itr)->in->name]->mark == m_up) {
		//this stmt remains. just check if the out is a breakpoint, if so set its entry to sink
		//(done automatically at drop?
		//if ((*bufStateTable)[(*itr)->out->name]->mark == m_break) {
		//setSink((*bufStateTable)[(*itr)->out->name]);
		//}
	} else if ((*bufStateTable)[(*itr)->in->name]->mark == m_down
			|| (*bufStateTable)[(*itr)->in->name]->mark == m_break) {
		//this stmt should be removed from myself, and add to drv1
		remove.push_back((*itr));
	}
}

if (!remove.empty()) {
	for (list<stmt*>::iterator itr = remove.begin(); itr!= remove.end(); itr++) {
		Driver *drv4=NULL;
		if (mgr->dropStmt((*itr), drv4) != SUCCESS) {
			if (querySchdl::verbose) cout << "error dropping statement " << (*itr)->name << endl;
			DBUG_RETURN(FAILURE);
		}

		//if (querySchdl::verbose) {
		//cout << "table after drop statement " << (*itr)->name << endl;
		//this->printStateTable();
		//}
	}

	for (list<stmt*>::iterator itr = remove.begin(); itr!= remove.end(); itr++) {          
		if (mgr->addStmt((*itr), drv1) != SUCCESS) {
			if (querySchdl::verbose) cout << "error adding statement " << (*itr)->name << endl;
			DBUG_RETURN(FAILURE);
		}

		//if (querySchdl::verbose) {
		//cout << "new table after add statement " << (*itr)->name << endl;
		//drv1->printStateTable();
		//}
	}
}

if (querySchdl::verbose) {
	cout << "old driver table after splitting: " << endl;
	this->printStateTable();
	cout << "newly created driver table: " << endl;
	drv1->printStateTable();
}

this->setDirty();
drv1->setDirty();

DBUG_RETURN(SUCCESS);
}
*/

void Driver::setSink(b_entry entry) 
{
	querySchdl::SMLOG(10, "Entering Driver::setSink");	
	switch (entry->b_type) {

		case t_normal:
		case t_fork:    
			entry->b_type = t_sink;
			entry->statement = 0;
			entry->inplace = 0;
			entry->forward.forward_buf = 0;
			entry->action = act_none;

			break;

		case t_merge:
		case t_merge_fork:
			entry->b_type = t_merge_sink;
			entry->statement = 0;
			entry->inplace = 0;
			entry->forward.forward_buf = 0;
			entry->action = act_merge;

			break;

		case t_source_fork:      
		case t_source:
		case t_sink:
		case t_merge_sink:
			break;

		default:
			cerr << "wrong buffer state type for an output buffer: " << entry->b_type << endl;
	}
}

void Driver::resetMarks() 
{
	querySchdl::SMLOG(10, "Entering Driver::resetMarks");	
	for (BufStateTblType::iterator itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
		itr->second->mark = m_up;
	}
}

//do I remove the tuple if it is an external source buffer?
//as long as no multiple drivers to share the same source, then it is fine to remove.
//currently this is the case
run_rc Driver::run(int* nTuples)
{ 
	//This method is been called very frequently so I have to comment the log
	//querySchdl::SMLOG(10, "Entering Driver::run");	
	if (*nTuples == 0) {
		*nTuples = priority;
	} 

	int oldNTuples = *nTuples;  
	int newNTuples = *nTuples;

	if (strategy != NULL || weightedStrategy != NULL) {
		//this part uses strategies.
		//_dg("use strategy classes to run the driver...");    
		if((strategy == NULL || strategy->empty()) && (weightedStrategy == NULL || weightedStrategy->empty())) {
			//nothing to run
			usleep(100);
		}

		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv, &tz);

		//prefer weighted strategy to normal strategy, for now
		run_rc rc;
		if (weightedStrategy != NULL && !weightedStrategy->empty()) {
			rc = weightedStrategy->runNextUnit(newNTuples);
			if (this->stateTableChanged) {
				StrategyFactory::getInstance()->cleanupTrash();
				if (querySchdl::verbose) cout << "state table changed in driver 1" << endl;
			}

		} else {
			rc = strategy->runNextUnit(nTuples);
			if (querySchdl::verbose && this->stateTableChanged) cout << "state table changed in driver 2" << endl;      
		}

		struct timeval tv2;
		gettimeofday(&tv2, &tz);
		//double diff = timeval_subtract(tv2, tv);
		//compilation error, just copy the code for timeval_subtract
		double diff = tv2.tv_sec;
		diff = diff + ((double)tv2.tv_usec/1000000);
		diff = diff - tv.tv_sec;
		diff = diff - ((double)tv.tv_usec/1000000);

		DrvMgr* dm = DrvMgr::getInstance();
		Monitor* monitor = dm->getMonitor();

		//if (diff > 0.00008) {
		//cout << "scheduling time: " << diff << endl;
		//}

		//if (diff > 0.01) {
		//cout << "scheduling time over 0.01s: " << diff << endl;
		//}

		//if (diff > 0.01) {
		//  cout << "outlier scheduling time is " << diff << endl;
		//  diff = 0.0005;
		//}

		if (monitor->collectStats && monitor->total_scheduling_time.on) {
			monitor->total_scheduling_time.value += diff;
		}

		if (rc == run_failure) return run_failure;
		else if (rc == run_success || (newNTuples < oldNTuples || *nTuples < oldNTuples)) return run_success;
		else return run_no_input;

		//if (rc == run_no_input) {
		//this means none of the operator has anything on input (or enough input to process), better sleep a little
		//cout << "no input in all stmts, sleep for 1 sec." << endl;
		//sleep(1);
		//}
	} else { //this part is the old existing implementation which does not use strategies.

		//starting from the next buffer of last_run
		//loop through all source buffers, until they are all empty for a complete round  
		BufStateTblType::iterator itr, itr2;
		//if (last_run == NULL) {
		if (strlen(last_run_name) == 0) {
			itr = bufStateTable->begin();
			itr2 = bufStateTable->begin();
		} else {
			if (bufStateTable->find(last_run_name) == bufStateTable->end())
				itr = itr2 = bufStateTable->begin();
			else {
				itr = itr2 = ++(bufStateTable->find(last_run_name));
				if (itr == bufStateTable->end())
					itr = itr2 = bufStateTable->begin();
			}
		}

		bool start = true;

		while(itr != itr2 || start) {
			if (start) start = false;

			if (itr->second->b_type == t_source || itr->second->b_type == t_source_fork) {

				if (itr->second->buf->empty()) {
					itr++;
					if (itr == bufStateTable->end()) itr = bufStateTable->begin();
					continue;
				} else {
					//set this as the first buffer not empty, to ensure going a full round before exit
					itr2 = itr;
					strcpy(last_run_name, itr->second->buf->name);
					run_rc rc = runState(itr->second, nTuples);
					if (rc == run_success && (*nTuples) <= 0) {
						_dg("driver finished processing tuple quota.");

						return run_success;
					} else if (rc == run_failure) {
						return rc;
					} 
					itr++;
					if (itr == bufStateTable->end()) itr = bufStateTable->begin();
					continue;
				}
			} else {
				itr++;
				if (itr == bufStateTable->end()) itr = bufStateTable->begin();      
				continue;
			}
		}
		//looped through all source buffers to be empty once
		usleep(100);
		return run_success;

	}
	//if (querySchdl::verbose && this->stateTableChanged) cout << "1 ";  
	}

	//static int test = 0;

	run_rc Driver::runState(b_entry state, int* nTuples, bool unionNoTime) 
	{
		querySchdl::SMLOG(10, "Entering Driver::runState");	
		//if (++test > 200) return run_failure;

		dbt* t = 0;

		run_rc rc;
		stmt_rc s_rc;

		buffer* buf;
		int nFork = 0;
		int cnt = 0;
		BufStateTblType::iterator itr;
		list<char*>::iterator itr2;
		list<char*> dummies;



		switch (state->action) {
			case act_cjump:
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act cJump, tuple quota to process is " << *nTuples << endl;
				//currently, this can be a normal or source buffer
				if (state->b_type != t_normal && state->b_type != t_source) {
					if (querySchdl::verbose) cout << "internal state table error. cjump action with wrong buffer type: "
						<< buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				if (state->b_type == t_source) {
					//I am a source, first test if I already processed my quota
					if ((*nTuples)-1 < 0) {
						if (querySchdl::verbose) cout << "driver finished processing quota." << endl;
						return run_success;
					}
					//if there is nothing in the external buffer, I exit
					//if I am a backtrack from a union buffer that does not consider time,
					//then I need to return different code, and set the return nTuples to correct value
					if (state->buf->empty()) {
						_dg("source buffer empty, finish this branch.");
						return run_success;
						/*
						   if (!unionNoTime) {
						   if (querySchdl::verbose) cout << "driver input queue empty, exit." << endl;	  
						   return run_success;
						   } else {
						   if (querySchdl::verbose) cout << "driver input queue empty, timeless union, exit to be re-entered." << endl;	  	  
						   return run_tl_union;
						   }
						 */
					}
				} else {
					//after adding join, non-source also need to make this check
					if ((*nTuples)-1 < 0) {
						if (querySchdl::verbose) cout << "driver finished processing quota." << endl;
						return run_success;
					}
				}

				if ((int)(s_rc = state->statement->exe()) == s_failure) {
					//sendErrorForStmt(state->statement);
				}

				if ((int)s_rc > 0 /*== s_success*/ || (int) s_rc == s_no_output || (int) s_rc == s_failure) {
					if (querySchdl::verbose) cout << "return code in cJump is " << (int) s_rc << endl;

					//I need to pop the tupple now
					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
						//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
						buffer* b = state->statement->in;
						//if the other "in" is not a fork buffer, we directly pop it.
						//If it is a fork buffer, then we need to know which dummy to record a pop.
						if (((*bufStateTable)[b->name])->b_type != t_fork
								&& ((*bufStateTable)[b->name])->b_type != t_source_fork && ((*bufStateTable)[b->name])->b_type != t_merge_fork) {
							if (querySchdl::verbose) cout << "about to pop tuple from buffer " << b->name << endl;
							b->pop();
							if (((*bufStateTable)[b->name])->b_type == t_source) (*nTuples)--;

							_dg("poped");
							_dg(b->name);
							if (b->empty()) _dg("empty"); else _dg("not empty");
						} else {
							//The buffer I need to pop is a fork.
							//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
							for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
								if (itr->second->b_type != t_fork_dummy) continue;
								if (strcmp(itr->second->inplace, b->name) == 0) {
									if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
										(itr->second->pop());
									}
								}    
							}
						}
					} else if (state->b_type == t_source/* && state->buf->shared != WINDOWED*/) {    
						//I am a source, for now I always pop the buffer
						//later I maybe need to see if this is a shared external buffer??
						if (querySchdl::verbose) cout << "about to pop tuple from buffer " << state->buf->name << endl;
						state->buf->pop();
						(*nTuples)--;

						_dg("poped");
						_dg(state->buf->name);
						if (state->buf->empty()) _dg("empty"); else _dg("not empty");	
					} else /*if (state->buf->shared != WINDOWED)*/ {
						//when not a source, always pop. (since in this case, I can not be a fork)
						if (querySchdl::verbose) cout << "about to pop tuple from buffer " << state->buf->name << endl;	
						state->buf->pop();

						_dg("poped");
						_dg(state->buf->name);
						if (state->buf->empty()) _dg("empty"); else _dg("not empty");
					}

					if (querySchdl::verbose) cout << "before moving the state" << endl;

					//now move the state
					if ((int) s_rc > 0 /*== s_success*/) {
						//I know this can not be a fork, just use the correct field
						char* nextBuf = state->forward.forward_buf;
						if ((*bufStateTable)[nextBuf]->b_type == t_sink
								/*|| (*bufStateTable)[nextBuf]->b_type == t_merge_sink*/) {
							return runState((*bufStateTable)[state->inplace], nTuples, unionNoTime);
						} else {
							return runState((*bufStateTable)[nextBuf], nTuples, unionNoTime);
						}
					} else if ((int)s_rc == s_no_output) {
						return runState((*bufStateTable)[state->inplace], nTuples, unionNoTime);
					} else if ((int)s_rc == s_failure) {
						return run_failure;
					}
				} else if ((int)s_rc == s_no_input) {
					if (querySchdl::verbose) cout << "return code in cJump is " << (int) s_rc << endl;

					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
						//retrack to the buffer which is before the one set to "in" of the statement
						return runState((*bufStateTable)[(((*bufStateTable)[state->statement->in->name])->back.back_buf)], nTuples, false);
					} else if (state->statement->type == stmt_join) {
						//retrack to the buffer which is before the one set to "back_buf" of the statement
						jStmt* s = static_cast<jStmt*> (state->statement);
						if (s->back_buf != NULL && s->back_buf != 0) {
							//_dg("join go to backbuf");
							//_dg(s->back_buf->name);
							b_entry back_entry = (*bufStateTable)[s->back_buf->name];

							if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
								return run_success;	    
							} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
								if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*bufStateTable)[(*(s2->union_bufs.begin()))->name], nTuples, false);
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*bufStateTable)[(*(s2->union_bufs.begin()))->name], nTuples, true);	
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
									return runState((*bufStateTable)[back_entry->back.union_stmt->in->name], nTuples, true);
								} else {
									if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
									return run_failure;
								}
							} else {
								return runState((*bufStateTable)[(((*bufStateTable)[s->back_buf->name])->back.back_buf)], nTuples, false);
							}
						}
					}

					//else {

					if (state->b_type == t_source) {
						//I already tested for this above, really should not have happened
						return run_success;
						/*
						   if (!unionNoTime) {
						   return run_success;
						   } else {
						   return run_tl_union;
						   }
						 */
					} else {
						//I know this can not be a merge buffer
						return runState((*bufStateTable)[state->back.back_buf], nTuples, unionNoTime);
					}

					//}
				}

				break;

			case act_merge_jump:
				//currently, this can only be a merge buffer
				if (state->b_type != t_merge) {
					if (querySchdl::verbose) cout << "internal state table error. merge jump action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//after adding join, non-source also need to make this check
				if ((*nTuples)-1 < 0) {
					if (querySchdl::verbose) cout << "driver finished processing quota." << endl;
					return run_success;
				}

				//if my buffer has tuples in there, move forward. If not, back track to the correct buffer
				if (state->buf->empty()) {
					//check input buffers of the previous union statement
					uStmt* s;

					if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						/*
						   for (list<buffer*>::iterator itr = s->union_bufs.begin(); itr != s->union_bufs.end(); itr++) {
						   if ((*itr)->empty()) {
						//retrack on the back buffer of this empty buffer
						return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, false);
						}
						}
						//by this time seems all buffers have tuples, so run union stmt again.
						 */

						return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, false);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						/*
						   list<buffer*>::iterator itr = s->union_bufs.begin();
						//retrack on the back buffer of any buffer is fine, since all is empty
						return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, true);
						 */

						return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, true);	
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
						return runState((*bufStateTable)[state->back.union_stmt->in->name], nTuples, true);
					} else {
						if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
						return run_failure;
					}      
				} else {
					//process the tuple
					if ((int)(s_rc = state->statement->exe()) == s_failure) {
						//sendErrorForStmt(state->statement);
					}

					if ((int)s_rc > 0 /*== s_success*/ || (int) s_rc == s_no_output || (int) s_rc == s_failure) {
						//I need to pop the tupple now
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
							//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
							buffer* b = state->statement->in;
							//if the other "in" is not a fork buffer, we directly pop it.
							//If it is a fork buffer, then we need to know which dummy to record a pop.
							if (((*bufStateTable)[b->name])->b_type != t_fork
									&& ((*bufStateTable)[b->name])->b_type != t_source_fork && ((*bufStateTable)[b->name])->b_type != t_merge_fork) {
								b->pop();
								if (((*bufStateTable)[b->name])->b_type == t_source) (*nTuples)--;

								_dg("poped");
								_dg(b->name);
								if (b->empty()) _dg("empty"); else _dg("not empty");
							} else {
								//The buffer I need to pop is a fork.
								//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
								for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
									if (itr->second->b_type != t_fork_dummy) continue;
									if (strcmp(itr->second->inplace, b->name) == 0) {
										if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
											(itr->second->pop());
										}
									}    
								}
							}
						} else /*if (state->buf->shared != WINDOWED)*/ {
							state->buf->pop();

							_dg("poped");
							_dg(state->buf->name);
							if (state->buf->empty()) _dg("empty"); else _dg("not empty");
						}

						//move the state
						if ((int) s_rc > 0 /*== s_success*/) {
							//I know this can not be a fork, just use the correct field
							char* nextBuf = state->forward.forward_buf;
							if ((*bufStateTable)[nextBuf]->b_type == t_sink) {
								return runState((*bufStateTable)[state->inplace], nTuples, unionNoTime);
							} else {
								return runState((*bufStateTable)[nextBuf], nTuples, unionNoTime);
							}
						} else if ((int)s_rc == s_no_output) {
							return runState((*bufStateTable)[state->inplace], nTuples, unionNoTime);
						} else if ((int)s_rc == s_failure) {
							return run_failure;
						}
					} else if ((int)s_rc == s_no_input) {
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
							//retrack to the buffer which is before the one set to "in" of the statement
							return runState((*bufStateTable)[(((*bufStateTable)[state->statement->in->name])->back.back_buf)], nTuples, false);
						} else if (state->statement->type == stmt_join) {
							//retrack to the buffer which is before the one set to "back_buf" of the statement
							jStmt* s = static_cast<jStmt*> (state->statement);
							if (s->back_buf != NULL && s->back_buf != 0) {
								b_entry back_entry = (*bufStateTable)[s->back_buf->name];

								if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
									return run_success;	    
								} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
									if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*bufStateTable)[(*(s2->union_bufs.begin()))->name], nTuples, false);
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*bufStateTable)[(*(s2->union_bufs.begin()))->name], nTuples, true);	
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
										return runState((*bufStateTable)[back_entry->back.union_stmt->in->name], nTuples, true);
									} else {
										if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
										return run_failure;
									}
								} else {
									return runState((*bufStateTable)[(((*bufStateTable)[s->back_buf->name])->back.back_buf)], nTuples, false);
								}
							}
						}

						//else {

						//we have tested to make sure there is an input tuple, so this really should not have happened.
						//check input buffers of the previous union statement
						uStmt* s;

						if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);
							/*
							   for (list<buffer*>::iterator itr = s->union_bufs.begin(); itr != s->union_bufs.end(); itr++) {
							   if ((*itr)->empty()) {
							//retrack on the back buffer of this empty buffer
							return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, false);
							}
							}
							//by this time seems all buffers have tuples, so run union stmt again.
							 */

							return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, false);
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);
							/*
							   list<buffer*>::iterator itr = s->union_bufs.begin();
							//retrack on the back buffer of any buffer is fine, since all is empty
							return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, true);
							 */
							return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, true);
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
							return runState((*bufStateTable)[state->back.union_stmt->in->name], nTuples, true);	
						} else {
							if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
							return run_failure;
						}

						//}
					}
				}
				break;

			case act_cfork:
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act cFork, tuple quota to process is " << *nTuples << endl;
				//currently, this can be a fork or source fork buffer
				if (state->b_type != t_fork && state->b_type != t_source_fork) {
					if (querySchdl::verbose) cout << "internal state table error. cfork action with wrong buffer type: "
						<< buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				if (state->b_type == t_source_fork) {
					//I am a source, first test if I already processed my quota
					if ((*nTuples)-1 < 0) {
						return run_success;
					}
					//if there is nothing in the external buffer, I exit
					//if I am a backtrack from a union buffer that does not consider time,
					//then I need to return different code, and set the return nTuples to correct value
					if (state->buf->empty()) {
						_dg("source buffer empty, finish this branch.");
						return run_success;
						/*
						   if (!unionNoTime) {
						   return run_success;
						   } else {
						//rc = runState((*bufStateTable)[unionNoTimeBuf], nTuples);
						return run_tl_union;
						}
						 */
					}
				} else if (state->buf->empty()) {
					//I know I am not a merge buffer
					return runState((*bufStateTable)[state->back.back_buf], nTuples, unionNoTime);
				}

				nFork = state->forward.fork_count;
				//now find out the dummy states, the # should agree with nFork
				cnt = 0;
				for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
					if (itr->second->b_type != t_fork_dummy) continue;
					if (strcmp(itr->second->inplace, state->buf->name) == 0) {
						dummies.push_back(itr->first);
						cnt++;
					}    
				}
				if (nFork != cnt) {
					if (querySchdl::verbose) cout << "internal error, number of dummy states does not match the fork count for buffer: " << state->buf->name << endl;
					return run_failure;
				}

				itr2 = dummies.begin();
				while (1) {

					if (state->b_type == t_source_fork) {
						//I am a source, first test if I already processed my quota
						if ((*nTuples)-1 < 0) {
							return run_success;
						}
					}

					//check if this branch has been processed before.
					b_entry entry = (*bufStateTable)[*itr2];
					if (entry->back.processed == true) {
						nFork--;
						itr2++;
					} else {    
						rc = runState(entry, nTuples, unionNoTime);      

						if (nFork == cnt && rc == run_no_input) {
							//this really should not happen since we already tested before
							//even if it happens, should only happen when the first fork branch is called,
							//since we do not pop the buffer until the end.
							if (state->b_type == t_source_fork) {
								_dg("source buffer empty, finish this branch.");	  
								return run_success;
								/*
								   if (!unionNoTime) {
								   return run_success;
								   } else {
								   return run_tl_union;
								   }
								 */
							} else {
								//fork dummy can not be a merge buffer
								return runState((*bufStateTable)[state->back.back_buf], nTuples, unionNoTime);
							}
						} else if (rc == run_failure) {
							return run_failure;
						}

						//advance the branch iterator only if I am processed (I may not, if my downstream is a union)
						if (((*bufStateTable)[*itr2])->back.processed != true) {
							continue;
						} else {
							_dg("move fork pointer to process next one.");
							nFork--;
							itr2++;
						}
					}

					if (nFork > 0 && itr2 == dummies.end()) {
						if (querySchdl::verbose) cout << "internal error, dummy state list length and fork count mismatch for buffer: " << state->buf->name << endl;
						return run_failure;
					}

					if (nFork == 0 && itr2 == dummies.end()) {
						//the last fork branch is already called
						//make sure all are processed, otherwise, continue to loop (this is not likely to happen, so ignore efficiency here
						bool finished = true;
						for (list<char*>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
							if ((*bufStateTable)[*itr3]->back.processed == false) {
								finished = false;
								nFork = cnt;
								itr2 = dummies.begin();
							}
						}

						if (finished) {

							for (list<char*>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
								(*bufStateTable)[*itr3]->back.processed = false;
							}

							if (state->b_type == t_source_fork) {    
								//I am a source, for now I always pop the buffer
								//later I maybe need to see if this is a shared external buffer??
								state->buf->pop();
								(*nTuples)--;

								_dg("poped");
								_dg(state->buf->name);
								if (state->buf->empty()) _dg("empty"); else _dg("not empty");
							} else {
								//when not a source, always pop.
								state->buf->pop();

								_dg("poped");
								_dg(state->buf->name);
								if (state->buf->empty()) _dg("empty"); else _dg("not empty");
							}

							if (state->b_type == t_source_fork) {
								//I am a source, first test if I already processed my quota
								if ((*nTuples)-1 < 0) {
									return run_success;
								}
								//if there is nothing in the external buffer, I exit
								//if I am a backtrack from a union buffer that does not consider time,
								//then I need to return different code, and set the return nTuples to correct value
								if (state->buf->empty()) {
									_dg("source buffer empty, finish this branch.");
									return run_success;
									/*
									   if (!unionNoTime) {
									   return run_success;
									   } else {
									//rc = runState((*bufStateTable)[unionNoTimeBuf], nTuples);
									return run_tl_union;
									}
									 */
								}
							} else if (state->buf->empty()) {
								//I know I am not a merge buffer
								return runState((*bufStateTable)[state->back.back_buf], nTuples, unionNoTime);
							}

							//restart the while loop, and reset all processed flag on branches.
							nFork = cnt;
							//for (itr2 = dummies.begin(); itr2!= dummies.end(); itr2++) {
							//  (*bufStateTable)[*itr2]->back.processed = false;
							//}
							itr2 = dummies.begin();
							_dg("fork flags reset for next round");
						}
					}
				}
				break;

			case act_jump:
				if (querySchdl::verbose) cout << "state " << state->inplace << state->forward.forward_buf << ", act jump, tuple quota to process is " << *nTuples << endl;
				//currently, this can be a fork dummy buffer
				//this will not check for source, or pop the tuple, unlike cjump.
				if (state->b_type != t_fork_dummy) {
					if (querySchdl::verbose) cout << "internal state table error. jump action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//after adding join, non-source also need to make this check
				if ((*nTuples)-1 < 0) {
					if (querySchdl::verbose) cout << "driver finished processing quota." << endl;
					return run_success;
				}    

				//I am already processed in this round of fork
				if (state->back.processed) {
					return run_success;
				}

				if ((int)(s_rc = state->statement->exe()) == s_failure) {
					//sendErrorForStmt(state->statement);
				}

				if ((int)s_rc > 0 /*== s_success*/ || (int) s_rc == s_no_output || (int) s_rc == s_failure) {
					//pop myself, unless the following buffer is a union. If union, I then need to look up the correct one to pop
					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
						//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
						buffer* b = state->statement->in;
						//if the other "in" is not a fork buffer, we directly pop it.
						//If it is a fork buffer, then we need to know which dummy to record a pop.
						if (((*bufStateTable)[b->name])->b_type != t_fork
								&& ((*bufStateTable)[b->name])->b_type != t_source_fork && ((*bufStateTable)[b->name])->b_type != t_merge_fork) {
							b->pop();
							if (((*bufStateTable)[b->name])->b_type == t_source) (*nTuples)--;

							_dg("poped");
							_dg(b->name);
							if (b->empty()) _dg("empty"); else _dg("not empty");
						} else {
							//The buffer I need to pop is a fork.
							//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
							for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
								if (itr->second->b_type != t_fork_dummy) continue;
								if (strcmp(itr->second->inplace, b->name) == 0) {
									if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
										(itr->second->pop());
									}
								}    
							}
						}
					} else {
						//just pop myself as a dummy
						state->pop();
					}
					if (querySchdl::verbose) cout << "return code is: " << (int) s_rc << endl;
					//move the state
					if ((int) s_rc > 0 /*== s_success*/) {

						char* nextBuf = state->forward.forward_buf;
						if ((*bufStateTable)[nextBuf]->b_type == t_sink /*|| (*bufStateTable)[nextBuf]->b_type == t_merge_sink*/) {
							return run_success;
						} else {
							return runState((*bufStateTable)[nextBuf], nTuples, unionNoTime);
						}
					} else if ((int)s_rc == s_no_output) {
						return run_success;
					} else if ((int)s_rc == s_failure) {
						return run_failure;
					}
				} else if ((int)s_rc == s_no_input) {
					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
						//retrack to the buffer which is before the one set to "in" of the statement
						return runState((*bufStateTable)[(((*bufStateTable)[state->statement->in->name])->back.back_buf)], nTuples, false);
					} else if (state->statement->type == stmt_join) {
						//retrack to the buffer which is before the one set to "back_buf" of the statement
						jStmt* s = static_cast<jStmt*> (state->statement);
						if (s->back_buf != NULL && s->back_buf != 0) {
							b_entry back_entry = (*bufStateTable)[s->back_buf->name];

							if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
								return run_success;	    
							} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
								if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*bufStateTable)[(*(s2->union_bufs.begin()))->name], nTuples, false);
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*bufStateTable)[(*(s2->union_bufs.begin()))->name], nTuples, true);	
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
									return runState((*bufStateTable)[back_entry->back.union_stmt->in->name], nTuples, true);
								} else {
									if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
									return run_failure;
								}
							} else {
								return runState((*bufStateTable)[(((*bufStateTable)[s->back_buf->name])->back.back_buf)], nTuples, false);
							}
						}
					} //else {
					return run_no_input;
					//}
				} 
				break;

			case act_merge:
				//currently, this can only be a merge_sink buffer
				if (state->b_type != t_merge_sink) {
					if (querySchdl::verbose) cout << "internal state table error. merge action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//if my buffer does not have tuples in there, back track to the correct buffer
				if (state->buf->empty()) {
					//check input buffers of the previous union statement
					uStmt* s;

					if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						/*
						   for (list<buffer*>::iterator itr = s->union_bufs.begin(); itr != s->union_bufs.end(); itr++) {
						   if ((*itr)->empty()) {
						//retrack on the back buffer of this empty buffer
						return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, false);
						}
						}
						//by this time seems all buffers have tuples, so run union stmt again.
						 */

						return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, false);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						/*
						   list<buffer*>::iterator itr = s->union_bufs.begin();
						//retrack on the back buffer of any buffer is fine, since all is empty
						return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, true);
						 */

						return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, true);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
						return runState((*bufStateTable)[state->back.union_stmt->in->name], nTuples, true);	
					} else {
						if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
					}      
				} else {
					//do nothing, especially, do not pop
					return run_success;
				}
				break;

			case act_merge_cfork:
				//currently, this can only be a merge-fork buffer
				if (state->b_type != t_merge_fork) {
					if (querySchdl::verbose) cout << "internal state table error. merge cfork action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//after adding join, non-source also need to make this check
				if ((*nTuples)-1 < 0) {
					if (querySchdl::verbose) cout << "driver finished processing quota." << endl;
					return run_success;
				}    

				//if my buffer has tuples in there, move forward. If not, back track to the correct buffer
				if (state->buf->empty()) {
					//check input buffers of the previous union statement
					uStmt* s;

					if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						/*
						   for (list<buffer*>::iterator itr = s->union_bufs.begin(); itr != s->union_bufs.end(); itr++) {
						   if ((*itr)->empty()) {
						//retrack on the back buffer of this empty buffer
						return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, false);
						}
						}
						//by this time seems all buffers have tuples, so run union stmt again.
						 */

						return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, false);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						/*
						   list<buffer*>::iterator itr = s->union_bufs.begin();
						//retrack on the back buffer of any buffer is fine, since all is empty
						return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, true);
						 */

						return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, true);	
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
						return runState((*bufStateTable)[state->back.union_stmt->in->name], nTuples, true);	
					} else {
						if (querySchdl::verbose) cout << "internal error. merge cfork action with wrong statement type. "  << endl;
					}      
				} else {
					//we move forward      
					nFork = state->forward.fork_count;
					//now find out the dummy states, the # should agree with nFork
					cnt = 0;
					for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
						if (itr->second->b_type != t_fork_dummy) continue;
						if (strcmp(itr->second->inplace, state->buf->name) == 0) {
							dummies.push_back(itr->first);
							cnt++;
						}    
					} 
					if (nFork != cnt) {
						if (querySchdl::verbose) cout << "internal error, number of dummy states does not match the fork count for buffer: " << state->buf->name << endl;
						return run_failure;
					}

					itr2 = dummies.begin();
					while (1) {
						//check if this branch has been processed before.
						b_entry entry = (*bufStateTable)[*itr2];
						if (entry->back.processed == true) {
							nFork--;
							itr2++;
						} else {
							rc = runState(entry, nTuples, unionNoTime);
							if (nFork == cnt && rc == run_no_input) {
								if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
									//retrack to the buffer which is before the one set to "in" of the statement
									return runState((*bufStateTable)[(((*bufStateTable)[state->statement->in->name])->back.back_buf)], nTuples, false);
								} else {
									//this really should not happen since we already tested before
									//even if it happens, should only happen when the first fork branch is called,
									//since we do not pop the buffer until the end.

									//check input buffers of the previous union statement
									uStmt* s;	    
									if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
										s = static_cast<uStmt*> (state->back.union_stmt);
										/*
										   for (list<buffer*>::iterator itr = s->union_bufs.begin(); itr != s->union_bufs.end(); itr++) {
										   if ((*itr)->empty()) {
										//retrack on the back buffer of this empty buffer
										return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, false);
										}
										}
										//by this time seems all buffers have tuples, so run union stmt again.
										 */

										return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, false);
									} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
										s = static_cast<uStmt*> (state->back.union_stmt);
										/*
										   list<buffer*>::iterator itr = s->union_bufs.begin();
										//retrack on the back buffer of any buffer is fine, since all is empty
										return runState((*bufStateTable)[(((*bufStateTable)[(*itr)->name])->back.back_buf)], nTuples, true);
										 */

										return runState((*bufStateTable)[(*(s->union_bufs.begin()))->name], nTuples, true);
									} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
										return runState((*bufStateTable)[state->back.union_stmt->in->name], nTuples, true);	
									} else {
										if (querySchdl::verbose) cout << "internal error. merge cfork action with wrong statement type. "  << endl;
										return run_failure;
									}
								}
							} else if (rc == run_failure) {
								return run_failure;
							}

							//advance the branch iterator only if I am processed (I may not, if my downstream is a union)
							if (((*bufStateTable)[*itr2])->back.processed != true) {
								continue;
							} else {
								nFork--;
								itr2++;
							}
						}

						if (nFork > 0 && itr2 == dummies.end()) {
							if (querySchdl::verbose) cout << "internal error, dummy state list length and fork count mismatch for buffer: " << state->buf->name << endl;
							return run_failure;
						}
						if (nFork == 0 && itr2 == dummies.end()) {
							//make sure all are processed, otherwise, start over again (this is not likely to happen, so ignore efficiency here
							bool finished = true;
							for (list<char*>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
								if ((*bufStateTable)[*itr3]->back.processed == false) {
									finished = false;
									nFork = cnt;
									itr2 = dummies.begin();
								}
							}

							if (finished) {

								for (itr2 = dummies.begin(); itr2!= dummies.end(); itr2++) {
									(*bufStateTable)[*itr2]->back.processed = false;
								}

								state->buf->pop();

								_dg("poped");
								_dg(state->buf->name);
								if (state->buf->empty()) _dg("empty"); else _dg("not empty");

								//restart the while loop, and reset all processed flag on branches.
								nFork = cnt;
								itr2 = dummies.begin();
							}
						}
					}
				}
				break;

			case act_none:
				return run_success;

			default:
				if (querySchdl::verbose) cout << "action not recognizable for buffer: " << state->buf->name << endl;
				return run_failure;
		}

		//all correct returns should have happened in the switch statement. Error if I reach here.
		if (querySchdl::verbose) cout << "driver error, switch statement does not return for: " << state->buf->name << endl;
		return run_failure;  
	}

	buffer* Driver::getBuffer(char* bn) 
	{
		if (bufStateTable->find(bn) != bufStateTable->end()) {
			return (*bufStateTable)[bn]->buf;
		} else {
			return 0;
		}
	}

	buffer_ty Driver::getBufType(buffer* b) 
	{
		if (bufStateTable->find(b->name) != bufStateTable->end()) {
			return (*bufStateTable)[b->name]->b_type;
		} else {
			return t_none;
		}
	}

	void Driver::printStateTable(BufStateTblType* tbl, ostream* os)
	{
		if (os == NULL) os = &cout;

		if (tbl == NULL) {
			tbl = bufStateTable;
			(*os) << "display state for driver, # of tuples to process before checking command (used to be called priority) is " << priority << endl;
			(*os) << "number of statements is " << stmts->size() << endl;
			(*os) << "number of buffers is " << bufs->size() << endl;
			(*os) << "number of entry in state table is " << bufStateTable->size() << endl;
		} else {
			(*os) << "display segmentation state for strategy. "<< endl;
		}

		(*os) << "BufNm\t\t\t\tType\t\tStmt\t\t\tBack\t\tInplace\tFw/Fork\tAction" << endl;

		for (BufStateTblType::iterator itr = tbl->begin(); itr != tbl->end(); itr++) {
			//if (querySchdl::verb(*os)e) (*os) << 0; (*os).flush();
			int len = strlen((*itr).first);
			if (len >= 24) {
				(*os) << (*itr).first << "\t";
			} else if (len >= 16) {
				(*os) << (*itr).first << "\t\t";
			} else if (len >= 8) {
				(*os) << (*itr).first << "\t\t\t";
			} else {
				(*os) << (*itr).first << "\t\t\t\t";
			}

			if ((*itr).second->b_type == t_fork_dummy || (*itr).second->b_type == t_merge_fork || (*itr).second->b_type == t_source_fork) {
				(*os) << buffer_t_strs[(*itr).second->b_type]
					<< "\t";
			} else {
				(*os) << buffer_t_strs[(*itr).second->b_type]
					<< "\t\t";
			}    

			if ((*itr).second->b_type != t_sink && (*itr).second->b_type != t_merge_sink) {
				len = strlen((*itr).second->statement->name);
				if (len >= 16) {
					(*os) << (*itr).second->statement->name << "\t";
				} else if (len >= 8) {
					(*os) << (*itr).second->statement->name << "\t\t";
				} else {
					(*os) << (*itr).second->statement->name << "\t\t\t";
				}
			} else {
				(*os) << "-" << "\t\t\t";
			}

			if ((*itr).second->b_type == t_merge || (*itr).second->b_type == t_merge_sink
					|| (*itr).second->b_type == t_merge_fork) {
				(*os) << (*itr).second->back.union_stmt->name << "\t\t";
			} else if ((*itr).second->b_type != t_fork_dummy && (*itr).second->b_type != t_source && (*itr).second->b_type != t_source_fork) {
				if (strlen((*itr).second->back.back_buf) >= 8) {
					(*os) << (*itr).second->back.back_buf << "\t";
				} else {
					(*os) << (*itr).second->back.back_buf << "\t\t";
				}
			} else {
				(*os) << "-" << "\t\t";  
			}

			if ((*itr).second->b_type != t_sink && (*itr).second->b_type != t_merge_sink) {
				(*os) << (*itr).second->inplace << "\t";
			} else {
				(*os) << "-" << "\t";
			}

			if (itr->second->b_type == t_fork || itr->second->b_type == t_source_fork
					|| itr->second->b_type == t_merge_fork) {
				(*os) << itr->second->forward.fork_count << "\t";
			} else if ((*itr).second->b_type != t_sink && (*itr).second->b_type != t_merge_sink) {
				(*os) << itr->second->forward.forward_buf << "\t";
			} else {
				(*os) << "-" << "\t";
			}

			if ((*itr).second->b_type != t_sink && (*itr).second->b_type != t_merge_sink) {
				(*os) << d_action_strs[itr->second->action] << endl;
			} else {
				(*os) << "-" << endl;
			}

		}
		(*os) << endl;
	}


	int DrvMgr::id_count = 0;

	DrvMgr* DrvMgr::_instance = 0;

	DrvMgr* DrvMgr::getInstance()
	{
		if (_instance == 0) {
			_instance = new DrvMgr();
		}
		return _instance;
	}

	void DrvMgr::destroy()
	{
		if (_instance != 0) {
			delete _instance;
			_instance = 0;
		}
	}

	DrvMgr::DrvMgr()
	{
		querySchdl::SMLOG(10, "Entering DrvMgr::DrvMgr");	
		drivers = new list<Driver*>();
		bufToDrvMap = new BufMapType();
		stmtToDrvMap = new hash_map<char*, Driver*, hash<char*>, eqstr >();
		monitor = new Monitor();
	}


	DrvMgr::~DrvMgr()
	{
		querySchdl::SMLOG(10, "Entering DrvMgr::~DrvMgr");	

		for (list<Driver*>::iterator itr = drivers->begin(); itr != drivers->end(); itr++) {
			delete *itr;
		}
		delete drivers;
		delete bufToDrvMap;
		delete stmtToDrvMap;
		delete monitor;
	}


	int DrvMgr::addStmt(stmt *s){
		querySchdl::SMLOG(10, "Entering DrvMgr::addStmt");	
		Driver *dummy=NULL;
		return addStmt(s, dummy);
	};
	//first search for which driver to add it in, once found, add it to that driver.
	//if not found in any driver, then create a new driver for it. Add stmt and new buffers to the driver maps.
	//to search for a driver for a stmt
	//if the 'in' buffer (or the in-buffers, in the case of union) is found somewhere
	//  if it is a unique finding, attach it to that driver.
	//  if there are two found and one is source buffer, one is sink buffer, then it was an break point, attach to the source buffer.
	//  if there are more than two found, error.
	//if 'in' buffer is found no where, then if 'out' buffer is found somewhere
	//  if it is a unique finding and a source, use that driver.
	//  if there are two or more found, error
	//else create a new driver for it
	int DrvMgr::addStmt(stmt* s, Driver* &drv) 
	{
		querySchdl::SMLOG(10, "Entering DrvMgr::addStmt 2");	
		DBUG_ENTER("DrvMgr::addStmt");
		if (stmtToDrvMap->find(s->name) != stmtToDrvMap->end()) {
			if (querySchdl::verbose) cout << "stmt already added in drvmgr" << endl;
			DBUG_RETURN(SUCCESS);
		}

		bool newDrv = false;
		bool union_buf = (s->type == stmt_t_union || s->type == stmt_tl_union) ? true : false;
		list<buffer*> found_bufs;

		if (union_buf) {
			uStmt* s2 = static_cast<uStmt*> (s);
			for (list<buffer*>::iterator itr = s2->union_bufs.begin(); itr != s2->union_bufs.end(); itr++) {
				if (bufToDrvMap->find((*itr)->name) == bufToDrvMap->end()) {
					//this is a new buffer
					continue;
				} else {
					//at least one is found
					found_bufs.push_back(*itr);
				}
			}
		} else {
			if (bufToDrvMap->find(s->in->name) != bufToDrvMap->end()) {
				found_bufs.push_back(s->in);
			}
		}  

		if (!found_bufs.empty()) {

			//input buffer is found somewhere, check which driver to use
			//also in the case of union buffers, check all found ones are consistent
			for (list<buffer*>::iterator itr = found_bufs.begin(); itr != found_bufs.end(); itr++) {

				pair<BufMapType::const_iterator, BufMapType::const_iterator> p
					= bufToDrvMap->equal_range((*itr)->name);
				Driver* drv2 = (*(p.first)).second;
				Driver* drv3 = 0;
				if (++(p.first) != p.second) {
					drv3 = (*(p.first)).second;
					if (++(p.first) != p.second) {
						if (querySchdl::verbose) cout << 0 << endl;
						if (querySchdl::verbose) cout << "stmt with name " << s->name
							<< " consumes from more than two existing drivers. error for now." << endl;
						DBUG_RETURN( FAILURE); //To be determined later
					} 	
				}

				bool inDrv = false;
				if (drv!= 0 && drv->getBuffer((*itr)->name) != 0) inDrv = true;

				if (drv3 != 0) {

					//choose between drv2 and drv3
					if ((drv2->getBufType(*itr) == t_sink || drv2->getBufType(*itr) == t_merge_sink)
							&& (drv3->getBufType(*itr) == t_source || drv3->getBufType(*itr) == t_source_fork)) {
						if (drv != 0 && inDrv && drv != drv3) {
							//error, the sources map to different drivers
							if (querySchdl::verbose) cout << 1 << endl;
							if (querySchdl::verbose) cout << "stmt with name " << s->name
								<< " consumes from more than two existing drivers. error for now." << endl;
							DBUG_RETURN( FAILURE); //To be determined later
						}
						if (drv == 0 || !inDrv) drv = drv3;
					} else if ((drv3->getBufType(*itr) == t_sink || drv3->getBufType(*itr) == t_merge_sink)
							&& (drv2->getBufType(*itr) == t_source || drv2->getBufType(*itr) == t_source_fork)) {
						if (drv != 0 && inDrv && drv != drv2) {
							//error, the sources map to different drivers
							if (querySchdl::verbose) cout << 2 << endl;
							if (querySchdl::verbose) cout << "stmt with name " << s->name
								<< " consumes from more than two existing drivers. error for now." << endl;
							DBUG_RETURN(FAILURE);; //To be determined later
						}
						if (drv == 0 || !inDrv) drv = drv2;
					} else {
						//wrong combination of buffer types. return error.
						if (querySchdl::verbose) cout << "stmt with name " << s->name
							<< " consumes from two drivers, but buffer types are not one sink one source, error." << endl;
						DBUG_RETURN(FAILURE);;
					}
				} else {
					if (drv != 0 && inDrv && drv != drv2) {
						if ((drv2->getBufType(*itr) == t_sink || drv2->getBufType(*itr) == t_merge_sink)
								&& (drv->getBufType(*itr) == t_source || drv->getBufType(*itr) == t_source_fork) ) {
							//no-op
						} else if ((drv->getBufType(*itr) == t_sink || drv->getBufType(*itr) == t_merge_sink)
								&& (drv2->getBufType(*itr) == t_source || drv2->getBufType(*itr) == t_source_fork) ) {
							drv = drv2;
						}  else {
							//error, the sources map to different drivers
							if (querySchdl::verbose) cout << 3 << "buffer is " << (*itr)->name << endl;
							if (querySchdl::verbose) cout << "drv is " << drv->id << ", drv2 is " << drv2->id << endl;
							if (querySchdl::verbose) {
								drv->printStateTable();
								drv2->printStateTable();
							} 
							if (querySchdl::verbose) cout << "stmt with name " << s->name
								<< " consumes from more than two existing drivers. error for now." << endl;
							DBUG_RETURN(FAILURE);; //To be determined later
						}
					}
					if (drv == 0 || !inDrv) drv = drv2;
				}

				inDrv = false;
				if (drv!= 0 && drv->getBuffer(s->out->name) != 0) inDrv = true;

				//now check the out buffer for consistency,
				//it should not be found anywhere else, and if found in this driver it has to be a source.
				if (bufToDrvMap->find(s->out->name) != bufToDrvMap->end()) {      
					pair<BufMapType::const_iterator, BufMapType::const_iterator> p
						= bufToDrvMap->equal_range(s->out->name);
					Driver* drv4 = (*(p.first)).second;

					if (++(p.first) != p.second) {
						if (querySchdl::verbose) cout << "1 stmt with name " << s->name
							<< " produces into an existing non-source buffer, or into more than one drivers." << endl;
						DBUG_RETURN(FAILURE);;
					} else {
						if (drv4 != drv && inDrv) {
							if (querySchdl::verbose) cout << "2 stmt with name " << s->name
								<< " produces into more than one existing driver." << endl;
							DBUG_RETURN(FAILURE);;
						} else {
							//make sure it is a source in this driver, since I am producing into it.
							if (inDrv && drv->getBufType(s->out) != t_source && drv->getBufType(s->out) != t_source_fork) {
								if (querySchdl::verbose) cout << "3 stmt with name " << s->name
									<< " produces into existing non-source buffer, error."
										<< endl;
							}
						}
					}
				}
			}
		} else {
			//the input buffers are not found anywhere. see if it produce to existing output buffer
			if (bufToDrvMap->find(s->out->name) == bufToDrvMap->end()) {
				//it is a stmt that needs a new driver
				if (drv==0) {
					//for now, let's say that we will use existing driver, if there is one
					if (drivers->empty()) {
						drv = new Driver();
						int d_id = ++id_count;
						drv->setId(d_id);
						newDrv = true;
					} else {
						drv = drivers->back();
					}
				}
			} else {

				bool inDrv = false;
				if (drv!= 0 && drv->getBuffer(s->out->name) != 0) inDrv = true;

				//the in buffer is a new buffer, but the statement produce into an existing buffer
				pair<BufMapType::const_iterator, BufMapType::const_iterator> p
					= bufToDrvMap->equal_range(s->out->name);
				Driver* drv2 = (*(p.first)).second;
				if (++(p.first) != p.second) {
					if (querySchdl::verbose) cout << "4 stmt with name " << s->name
						<< " produces into more than two existing drivers, or into non-source buffers." << endl;
					DBUG_RETURN(FAILURE);;
				} else {
					//make sure it is a source
					if (drv2->getBufType(s->out) != t_source && drv2->getBufType(s->out) != t_source_fork) {
						if (querySchdl::verbose) cout << "5 stmt with name " << s->name << "produces into non-source buffer." << endl;
						DBUG_RETURN(FAILURE);;
					}

					if (inDrv && drv != 0 && drv2 != drv) {
						//in this case, it has to be a sink in this drv, so it is a break point. Otherwise error.
						//actually, error anyway
						//if (drv->getBufType(s->out) != t_sink && drv->getBufType(s->out) != t_merge_sink) {
						if (querySchdl::verbose) cout << "6 stmt with name " << s->name << "produces into more than one driver, "
							<< "but it is not the case that one is source and one is sink." << endl;
						DBUG_RETURN(FAILURE);;
						//}
					}
					if (drv == 0) drv = drv2;
				} 
			}
		}

		int rc = SUCCESS;

		if ((rc = drv->addStmt(s)) == SUCCESS) {

			if (newDrv) drivers->push_back(drv);

			if (union_buf) {
				uStmt* s2 = static_cast<uStmt*> (s);
				for (list<buffer*>::iterator itr = s2->union_bufs.begin(); itr != s2->union_bufs.end(); itr++) {
					if (bufToDrvMap->find((*itr)->name) == bufToDrvMap->end()) {
						bufToDrvMap->insert(BufMapType::value_type((*itr)->name, drv));	  
					}
				}
			}
			else {
				if (bufToDrvMap->find(s->in->name) == bufToDrvMap->end()) {
					bufToDrvMap->insert(BufMapType::value_type(s->in->name, drv));
				}
			}

			if (bufToDrvMap->find(s->out->name) == bufToDrvMap->end()) {
				bufToDrvMap->insert(BufMapType::value_type(s->out->name, drv));
			}

			(*stmtToDrvMap)[s->name] = drv;

			DBUG_RETURN( SUCCESS);
		} else {
			//it is a failure of some type
			if (newDrv) delete drv;
			drv = 0;
			DBUG_RETURN( rc);
		}
	}

	list<Driver*>* DrvMgr::getDrivers() 
	{
		return this->drivers;
	}

	int DrvMgr::dropStmt(stmt *s, Driver* &drv){
		querySchdl::SMLOG(10, "Entering DrvMgr::dropStmt");	
		DBUG_ENTER("DrvMgr::dropStmt");

		if (querySchdl::verbose) cout << "buffer in dmgr before dropping stmt: " << s->name << endl;
		if (querySchdl::verbose) {
			for (BufMapType::iterator itr2 = bufToDrvMap->begin(); itr2 != bufToDrvMap->end(); itr2++) {
				cout << "buffer " << itr2->first << endl;
			}
		}

		StmtMapType::iterator itr;
		if ((itr = stmtToDrvMap->find(s->name)) == stmtToDrvMap->end()) {
			if (querySchdl::verbose) cout << "stmt does not exist in drvmgr" << endl;
			DBUG_RETURN(SUCCESS);
		} else {

			int rc = itr->second->dropStmt(s);

			if (rc == SUCCESS) {
				drv = itr->second;
				stmtToDrvMap->erase(s->name);
				if (drv->stmts->empty()) {
					//this driver can be dropped now
					drivers->remove(drv);
					delete drv;
					drv = NULL;
				}
			}

			if (querySchdl::verbose) cout << "buffer left in dmgr after dropping stmt: " << s->name << endl;
			if (querySchdl::verbose) {
				for (BufMapType::iterator itr2 = bufToDrvMap->begin(); itr2 != bufToDrvMap->end(); itr2++) {
					cout << "buffer " << itr2->first << endl;
				}
			}

			DBUG_RETURN(rc);
		}
	};

	//to be done later.
	int DrvMgr::setBreakPnt(list<buffer*>* bufs) {
		DBUG_ENTER("DrvMgr::setBreakPnt");
		DBUG_RETURN(SUCCESS);
	}

	/*  no longer used: driver now only keeps the original query graph, segmented graphs are kept by strategies. So set breakpoint will need to call strategies
	    int DrvMgr::setBreakPnt(list<buffer*>* bufs)
	    {
	    DBUG_ENTER("DrvMgr::setBreakPnt");
	//make sure all map to one driver, then use that driver
	Driver* drv = NULL;
	pair<BufMapType::const_iterator, BufMapType::const_iterator> p;
	for (list<buffer*>::iterator itr = bufs->begin(); itr != bufs->end(); itr++) {
	p = bufToDrvMap->equal_range((*itr)->name);
	if (drv == NULL) {
	drv = (*(p.first)).second;
	} else if ((drv != (*(p.first)).second) || (++(p.first) != p.second)) {
	cerr << "breakpoint maps to more than one driver. error." << endl;
	DBUG_RETURN(FAILURE);
	}
	}
	if (drv == NULL) {
	cerr << "breakpoint does not map to any driver. error." << endl;
	DBUG_RETURN(FAILURE);
	} else {
	DBUG_RETURN(drv->setBreakPnt(bufs));
	}
	}
	 */

	run_rc DrvMgr::run(Driver* drv, int* nTuples)
	{
		return drv->run(nTuples);
	}

	run_rc DrvMgr::run(Driver* drv)
	{
		int nTuples = 0;
		return drv->run(&nTuples);
	}

	stmt* DrvMgr::getStmtByName(char* name) 
	{
		if (name == NULL) return NULL;

		if (stmtToDrvMap->find(name) == stmtToDrvMap->end()) return NULL;
		else return (*stmtToDrvMap)[name]->getStmtByName(name);
	}

	buffer* DrvMgr::getBufByName(char* name) 
	{
		if (name == NULL) return NULL;

		pair<BufMapType::const_iterator, BufMapType::const_iterator> p =
			bufToDrvMap->equal_range(name);
		if (p.first == p.second) return NULL;
		else return (*(p.first)).second->getBufByName(name);
	}

	Driver* DrvMgr::getDrvById(int id) 
	{
		for (list<Driver*>::iterator itr = drivers->begin(); itr != drivers->end(); itr++) {
			if ((*itr)->getId() == id) return (*itr);
		}
		return NULL;
	}

	bool DrvMgr::bufferInUse(char* bname) 
	{
		if (bufToDrvMap->find(bname) == bufToDrvMap->end()) return false;
		else return true;  
	}

	bool DrvMgr::stmtInUse(const char* name) 
	{
		char bufname[MAX_ID_LEN];
		strcpy(bufname, name);

		if (stmtToDrvMap->find(bufname) == stmtToDrvMap->end()) return false;
		else return true;  
	}

	int driver_test ()
	{
		/*
		   if (querySchdl::verbose) cout << "***************test driver******************" << endl;

		   sDBT::createSM();
		   bufferMngr* bm= (bufferMngr*)mm_malloc(sDBT::dbt_mm, sizeof(bufferMngr));
		   if (!bm){
		   perror("Malloc buffer manager in shared memory");
		   return 1;
		   }
		   bufferMngr bmTemp;
		   memcpy(bm, (void*)&bmTemp, sizeof(bufferMngr));

		//to sleep 1 usec, so that the tuples will for sure have ordering.
		timespec pause;
		pause.tv_sec = 0;
		pause.tv_nsec = 1000;

		struct timeval tv1;
		gettimeofday(&tv1, NULL);
		nanosleep(&pause,NULL);
		struct timeval tv2;
		gettimeofday(&tv2, NULL);
		nanosleep(&pause,NULL);
		struct timeval tv3;
		gettimeofday(&tv3, NULL);
		nanosleep(&pause,NULL);  
		struct timeval tv4;
		gettimeofday(&tv4, NULL);
		nanosleep(&pause,NULL);  
		struct timeval tv5;
		gettimeofday(&tv5, NULL);

		dbt *d1 = new cDBT("abc", MAX_STR_LEN, &tv1);
		dbt *d2 = new cDBT("xyz", MAX_STR_LEN, &tv2);
		dbt *d3 = new cDBT("kkk", MAX_STR_LEN, &tv3);
		dbt *d4 = new cDBT("vvv", MAX_STR_LEN, &tv4);
		dbt *d5 = new cDBT("www", MAX_STR_LEN, &tv5);  
		d1->print();
		d2->print();
		d3->print();
		d4->print();
		d5->print();

		bm->create("buf1",
		SHARED);
		bm->create("buf2",
		SHARED);
		bm->create("buf3",
		SHARED);
		bm->create("buf4",
		SHARED);
		bm->create("buf5",
		SHARED);
		bm->create("buf6",
		SHARED);
		bm->create("buf7",
		SHARED);
		bm->create("buf8",
		SHARED);
		bm->create("buf9",
		SHARED);
		bm->create("buf10",
		SHARED);
		bm->create("buf11",
		SHARED);
		bm->create("buf12",
		SHARED);

		stmt* s1;
		stmt* s2;
		stmt* s3;
		stmt* s4;
		stmt* s5;
		stmt* s6;
		stmt* s7;
		stmt* s8;
		stmt* s9;
		stmt* s10;
		stmt* s11;
		stmt* s12;

		//first test a normal statement
		if (querySchdl::verbose) cout << "test normal statement" << endl;

		s1 = new nStmt();
		buffer *b = bm->lookup("buf1");

		b->put(d1);
		b->put(d2);

		s1->in = b;
		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name, "1");

		DrvMgr* dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  
		Driver* drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(2);
		drv->printStateTable();

		int nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing normal statement" << endl << endl;
		delete s1;

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}

		DrvMgr::destroy();

		//test a path
		if (querySchdl::verbose) cout << endl << "test path" << endl;  
		s1 = new nStmt();
		s2 = new nStmt();
		s3 = new nStmt();

		b = bm->lookup("buf1");

		b->put(d1);
		b->put(d2);

		s1->in = b;
		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name,"1");

		s2->in = b;
		b = bm->lookup("buf3");  
		s2->out = b;
		strcpy(s2->name, "2");

		s3->in = b;
		b = bm->lookup("buf4");  
		s3->out = b;
		strcpy(s3->name, "3");

		dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  

		drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(2);
		if (querySchdl::verbose) cout << "add stmt1" << endl;
		drv->printStateTable();

		if (dm->addStmt(s2, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt2" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt2" << endl;
		drv->printStateTable();

		if (dm->addStmt(s3, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt3" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt3" << endl;
		drv->printStateTable();

		_dg("end add");

		nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing path" << endl << endl;  

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}

		delete s1;
		delete s2;
		delete s3;
		DrvMgr::destroy();

		//test a fork
		if (querySchdl::verbose) cout << endl << "test fork" << endl;  
		s1 = new nStmt();
		s2 = new nStmt();
		s3 = new nStmt();
		s4 = new nStmt();

		b = bm->lookup("buf1");

		b->put(d1);
		b->put(d2);

		s1->in = b;
		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name, "1");

		s2->in = b;
		s4->in = b;
		b = bm->lookup("buf5");
		s4->out = b;
		strcpy(s4->name, "4");
		b = bm->lookup("buf3");  
		s2->out = b;
		strcpy(s2->name, "2");

		s3->in = b;
		b = bm->lookup("buf4");  
		s3->out = b;
		strcpy(s3->name, "3");

		dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  

		drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(2);
		if (querySchdl::verbose) cout << "add stmt1" << endl;
		drv->printStateTable();

		if (dm->addStmt(s2, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt2" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt2" << endl;
		drv->printStateTable();

		if (dm->addStmt(s3, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt3" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt3" << endl;
		drv->printStateTable();

		if (dm->addStmt(s4, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt4" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt4" << endl;
		drv->printStateTable();

		_dg("end add");

		nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing fork" << endl << endl;  

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}

		delete s1;
		delete s2;
		delete s3;
		delete s4;
		//_dg("deleted statements");

		DrvMgr::destroy();
		//_dg("drvmgr destroyed");

		//test a timed union
		if (querySchdl::verbose) cout << endl << "test timed union" << endl;  
		s1 = new nStmt();
		s2 = new nStmt();
		s3 = new nStmt();
		s4 = new utStmt();
		s5 = new nStmt();

		b = bm->lookup("buf1");

		b->put(d1);
		b->put(d3);

		s1->in = b;
		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name, "1");

		s2->in = b;
		b = bm->lookup("buf3");
		s2->out = b;
		strcpy(s2->name, "2");
		(static_cast<uStmt*>(s4))->union_bufs.push_back(b);

		b = bm->lookup("buf4");  
		s3->in = b;
		b->put(d2);

		b = bm->lookup("buf5");  
		s3->out = b;
		strcpy(s3->name, "3");
		(static_cast<uStmt*>(s4))->union_bufs.push_back(b);

		b = bm->lookup("buf6");  
		s4->out = b;
		strcpy(s4->name, "4");
		s5->in = b;

		b = bm->lookup("buf7");  
		s5->out = b;
		strcpy(s5->name, "5");

		dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  

		drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(3);
		if (querySchdl::verbose) cout << "add stmt1" << endl;
		drv->printStateTable();

		if (dm->addStmt(s2, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt2" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt2" << endl;
		drv->printStateTable();

		if (dm->addStmt(s3, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt3" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt3" << endl;
		drv->printStateTable();

		if (dm->addStmt(s4, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt4" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt4" << endl;
		drv->printStateTable();

		if (dm->addStmt(s5, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt5" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt5" << endl;
		drv->printStateTable();

		_dg("end add");

		nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing union" << endl << endl;

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}


		delete s1;
		delete s2;
		delete s3;
		delete s4;
		delete s5;  
		_dg("deleted statements");

		DrvMgr::destroy();
		_dg("drvmgr destroyed");

		//test a timeless union
		if (querySchdl::verbose) cout << endl << "test timeless union" << endl;  
		s1 = new nStmt();
		s2 = new nStmt();
		s3 = new nStmt();
		s4 = new utlStmt();
		s5 = new nStmt();

		b = bm->lookup("buf1");

		b->put(d1);
		b->put(d3);

		s1->in = b;
		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name, "1");

		s2->in = b;
		b = bm->lookup("buf3");
		s2->out = b;
		strcpy(s2->name, "2");
		(static_cast<uStmt*>(s4))->union_bufs.push_back(b);

		b = bm->lookup("buf4");  
		s3->in = b;
		b->put(d2);

		b = bm->lookup("buf5");  
		s3->out = b;
		strcpy(s3->name, "3");
		(static_cast<uStmt*>(s4))->union_bufs.push_back(b);

		b = bm->lookup("buf6");  
		s4->out = b;
		strcpy(s4->name, "4");
		s5->in = b;

		b = bm->lookup("buf7");  
		s5->out = b;
		strcpy(s5->name, "5");

		dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  

		drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(3);
		if (querySchdl::verbose) cout << "add stmt1" << endl;
		drv->printStateTable();

		if (dm->addStmt(s2, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt2" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt2" << endl;
		drv->printStateTable();

		if (dm->addStmt(s3, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt3" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt3" << endl;
		drv->printStateTable();

		if (dm->addStmt(s4, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt4" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt4" << endl;
		drv->printStateTable();

		if (dm->addStmt(s5, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt5" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt5" << endl;
		drv->printStateTable();

		_dg("end add");

		nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing union" << endl << endl;

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}


		delete s1;
		delete s2;
		delete s3;
		delete s4;
		delete s5;  
		_dg("deleted statements");

		DrvMgr::destroy();
		_dg("drvmgr destroyed");

		//test a join
		if (querySchdl::verbose) cout << endl << "test join" << endl;  
		s1 = new nStmt();
		s2 = new nStmt();
		s3 = new nStmt();
		s4 = new jStmt();
		s5 = new nStmt();

		b = bm->lookup("buf1");

		b->put(d1);
		b->put(d3);

		s1->in = b;
		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name, "1");

		s2->in = b;
		b = bm->lookup("buf3");
		s2->out = b;
		strcpy(s2->name, "2");
		s4->in = b;

		b = bm->lookup("buf4");  
		s3->in = b;
		b->put(d2);

		b = bm->lookup("buf5");  
		s3->out = b;
		strcpy(s3->name, "3");
		//b->shared = WINDOWED;
		(static_cast<jStmt*>(s4))->window_bufs.push_back(b);

		b = bm->lookup("buf6");  
		s4->out = b;
		strcpy(s4->name, "4");
		s5->in = b;

		b = bm->lookup("buf7");  
		s5->out = b;
		strcpy(s5->name, "5");

		dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  

		drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(3);
		if (querySchdl::verbose) cout << "add stmt1" << endl;
		drv->printStateTable();

		if (dm->addStmt(s2, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt2" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt2" << endl;
		drv->printStateTable();

		if (dm->addStmt(s3, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt3" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt3" << endl;
		drv->printStateTable();

		if (dm->addStmt(s4, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt4" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt4" << endl;
		drv->printStateTable();

		if (dm->addStmt(s5, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt5" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt5" << endl;
		drv->printStateTable();

		_dg("end add");

		nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing join" << endl << endl;

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}


		delete s1;
		delete s2;
		delete s3;
		delete s4;
		delete s5;  
		_dg("deleted statements");

		DrvMgr::destroy();
		_dg("drvmgr destroyed");  

		if (querySchdl::verbose) cout << endl << "build buffer table for buffer topology of the following, with both union and fork: " << endl;
		if (querySchdl::verbose) cout << "1 -> 2 -> 4 -> 5 -> 6" << endl;
		if (querySchdl::verbose) cout << "    3 -> 11 -^  |-> 7 -> 8" << endl;
		if (querySchdl::verbose) cout << "    9 -> 12 -^  |-> 10" << endl << endl;

		s1 = new nStmt();
		s2 = new nStmt();
		s3 = new nStmt();
		s4 = new nStmt();
		s5 = new utStmt();
		s6 = new nStmt();
		s7 = new nStmt();
		s8 = new nStmt();
		s9 = new nStmt();

		b = bm->lookup("buf1");
		b->put(d1);
		b->put(d3);
		s1->in = b;

		b = bm->lookup("buf2");  
		s1->out = b;
		strcpy(s1->name, "1");
		s2->in = b;

		b = bm->lookup("buf4");
		s2->out = b;
		(static_cast<uStmt*>(s5))->union_bufs.push_back(b);  
		strcpy(s2->name, "2");

		b = bm->lookup("buf3");  
		b->put(d2);
		b->put(d4);
		s3->in = b;

		b = bm->lookup("buf11");
		s3->out = b;
		(static_cast<uStmt*>(s5))->union_bufs.push_back(b);    
		strcpy(s3->name, "3");

		b = bm->lookup("buf9");  
		b->put(d5);
		s4->in = b;

		b = bm->lookup("buf12");
		s4->out = b;
		(static_cast<uStmt*>(s5))->union_bufs.push_back(b);    
		strcpy(s4->name, "4");  

		b = bm->lookup("buf5");  
		s5->out = b;
		strcpy(s5->name, "5");
		s6->in = b;
		s7->in = b;
		s8->in = b;

		b = bm->lookup("buf6");  
		s6->out = b;
		strcpy(s6->name, "6");

		b = bm->lookup("buf7");  
		s7->out = b;
		strcpy(s7->name, "7");
		s9->in = b;

		b = bm->lookup("buf8");  
		s9->out = b;
		strcpy(s9->name, "9");

		b = bm->lookup("buf10");  
		s8->out = b;
		strcpy(s8->name, "8");

		dm = 0;
		dm = DrvMgr::getInstance();
		if (dm == 0) if (querySchdl::verbose) cout << "ERROR creating driver manager" << endl;  

		drv = 0;

		dm->addStmt(s1, drv);
		if (drv == 0) if (querySchdl::verbose) cout << "ERROR creating driver" << endl;
		drv->setPriority(5);
		if (querySchdl::verbose) cout << "add stmt1" << endl;
		drv->printStateTable();

		if (dm->addStmt(s2, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt2" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt2" << endl;
		drv->printStateTable();

		if (dm->addStmt(s3, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt3" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt3" << endl;
		drv->printStateTable();

		if (dm->addStmt(s4, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt4" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt4" << endl;
		drv->printStateTable();

		if (dm->addStmt(s5, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt5" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt5" << endl;
		drv->printStateTable();

		if (dm->addStmt(s6, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt6" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt6" << endl;
		drv->printStateTable();

		if (dm->addStmt(s7, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt7" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt7" << endl;
		drv->printStateTable();

		if (dm->addStmt(s8, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt8" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt8" << endl;
		drv->printStateTable();

		if (dm->addStmt(s9, drv) == FAILURE) {
			if (querySchdl::verbose) cout << "error adding stmt9" << endl;
		}
		if (querySchdl::verbose) cout << "add stmt9" << endl;
		drv->printStateTable();

		_dg("end add");

		nTuples = 0;
		if (dm->run(drv, &nTuples) == run_failure) {
			if (querySchdl::verbose) cout << "ERROR running driver" << endl;
		}

		if (querySchdl::verbose) cout << "end testing all" << endl << endl;

		for (int i = 1; i<=12; i++) {
			char name[6];
			sprintf(name, "buf%d", i);
			buffer *b = bm->lookup(name);
			while (b!=0 && !(b->empty())) {
				if (querySchdl::verbose) cout << "clean up buffer " << name << " for one tuple" << endl;
				b->pop();
			}
		}


		delete s1;
		delete s2;
		delete s3;
		delete s4;
		delete s5;  
		delete s6;
		delete s7;
		delete s8;
		delete s9;
		if (querySchdl::verbose) cout << endl;
		_dg("deleted statements");

		DrvMgr::destroy();
		_dg("drvmgr destroyed");


		delete d1;
		delete d2;
		delete d3;
		delete d4;
		delete d5;

		sDBT::destroySM();

		if (querySchdl::verbose) cout << "------------end testing driver---------------" << endl;

		cout.flush();
		*/  
			return 0;
	}

} //name space ESL
