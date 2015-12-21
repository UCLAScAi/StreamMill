#include "strategy.h"
#include "adllib.h"
//#include "basic.h"
#include "const.h"
#include "util.h"
#include "querySchdl.h"
#include "monitor.h"
#include <algorithm>
#include <math.h>

extern "C"{
#include <dbug.h>
}

namespace ESL {

	extern bool exp_batch_activate_mode;

	StmtEntry::StmtEntry(stmt* s) {
		s->stmtEntry = this;

		this->stmt_name = new string(s->name);
		this->pStmt = s;
		if (s->rankSet) {
			this->priority = this->oriPriority = s->rank.priority;
		} else {
			this->priority = this->oriPriority = WeightProfile::DEFAULT_VALUE;
		}
		this->expectedQMax = 0;
		this->outputProcTime = 0;
		this->urgencyTime.tv_sec = 1999999999;
		this->urgencyTime.tv_usec = 0; 
	}

	WeightGroup::~WeightGroup() {
		for (vector<StmtEntry*>::iterator itr = vSEntries.begin(); itr != vSEntries.end(); itr++) {
			delete *itr;
		}
	}

	int WeightProfile::WEIGHT_MAX = 10;  //changable

	WeightProfile::WeightProfile(Driver* drv, int roundTotal, quota_mode mode) : roundTotal(roundTotal),mode(mode),stmtWithDeadline(false)  {
		querySchdl::SMLOG(10, "Entering WeightProfile::WeightProfile");	
		this->drv = drv;
		totalSourceStmt = 0;
		totalSrcOriPriority = 0;
		totalSrcPriority = 0;

		//update();
	}

	WeightProfile::~WeightProfile() {

		querySchdl::SMLOG(10, "Entering WeightProfile::~WeightProfile");	
		for (BufStateTblType::iterator itr = queryGraph.begin(); itr != queryGraph.end();) {
			char* s = itr->first; 
			delete itr->second;
			itr++;
			delete s;
		}

		for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); ++itr) {
			delete (*itr);
		}

	}

	int WeightProfile::calc_weight(stmt* s, BufStateTblType* drvMap) {
		querySchdl::SMLOG(10, "Entering WeightProfile::calc_weight");	
		if (s->monitor_sink_stat) {
			s->weightSet = true;
			//return s->weight;
		} else {
			//check downstream
			b_entry entry = (*drvMap)[s->out->name];
			if (entry->b_type != t_fork && entry->b_type != t_merge_fork) {
				//there is only one stmt downstream
				stmt* s2 = entry->statement;
				if (s2->weightSet) {
					s->weight = s2->weight;
					s->weightSet = true;
					//return s->weight;
				} else {
					s->weight = calc_weight(s2, drvMap);
					s->weightSet = true;
					//return s->weight;
				}
			} else {
				//this is a fork, check all downstream, and take max
				int maxWeight = WeightProfile::WEIGHT_MIN;
				int nFork = entry->forward.fork_count;
				//now find out the dummy states, the # should agree with nFork
				int cnt = 0;
				for (BufStateTblType::iterator itr2 = drvMap->begin(); itr2 != drvMap->end(); itr2++) {
					int weight2 = WeightProfile::DEFAULT_VALUE;
					if (itr2->second->b_type != t_fork_dummy) continue;
					else if (strcmp(itr2->second->inplace, s->out->name) == 0) {
						if (itr2->second->statement->weightSet) {
							weight2 = itr2->second->statement->weight;
						} else {
							weight2 = calc_weight(itr2->second->statement, drvMap);
						}
						if (maxWeight < weight2) {
							maxWeight = weight2;
						} 
						cnt++;
					} 
				}
				if (nFork != cnt) {
					cout
						<< "internal error, number of dummy states does not match the fork count for buffer: "
						<< s->out->name << endl;
				}
				s->weight = maxWeight;
				s->weightSet = true;
				//return s->weight;
			}
		}

		if (querySchdl::verbose) {
			cout << "weight for source statement " << s->name << " is " << s->weight << endl;
		}

		return s->weight;
	}

	//the base class method builds the query graph, calculate all weights, and put source stmts into their corresponding weight groups
	//it will be the children class responsibility to find out all priorities and sort the stmts on priority
	void WeightProfile::update() {
		querySchdl::SMLOG(10, "Entering WeightProfile::update");	
		//first build query graph from driver. Here is an exact copy, but other weight profile might choose to segment it more.
		if (queryGraph.size() > 0) {

			for (BufStateTblType::iterator itr = queryGraph.begin(); itr != queryGraph.end();) {
				if (itr->second->b_type != t_sink && itr->second->b_type != t_merge_sink && itr->second->statement != NULL) {
					itr->second->statement->weightSet = false;
					itr->second->statement->rankSet = false;
				}

				char* s = itr->first; 
				delete itr->second;
				itr++;
				delete s;
			}
			queryGraph.clear();    
		}

		//then build the weight groups
		if (vSGroups.size() > 0) {  
			for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); itr++) {
				delete (*itr);
			}
			vSGroups.clear();
		}

		//by default, the query graph is the same as in driver and does not have additional segmentation. Overwritten in children if needed.
		//build both query graph and weightgroups together
		BufStateTblType* drvMap = drv->getBufGraph();
		char* name;
		b_entry entry;

		//build query graph
		//Calculate all weights, start recursively from source, each stmt takes the max of its immidiate downstream stmt.
		//only sink stmts are assigned weights by end user.
		totalSourceStmt = 0;
		for (BufStateTblType::iterator itr = drvMap->begin(); itr != drvMap->end(); itr++) {
			entry = new buffer_entry(itr->second);
			//always copy over the key, it should not be shared since this is updated later than driver. 
			//if shared, by the time the destructor is called the shared key may already have been deleted.
			name = new char[strlen(itr->first)+1];
			strcpy(name, itr->first);
			//(*queryGraph)[name] = entry; 
			queryGraph.insert(BufStateTblType::value_type(name, entry));

			//calculate weights for all source statements
			if(entry->b_type == t_source || entry->b_type == t_source_fork) {
				list<stmt*> pStmts;

				if (entry->b_type == t_source) {
					if (querySchdl::verbose) cout << "to assign weightgroup, analyze " << entry->statement->name << endl;
					entry->statement->weightSet = false;
					pStmts.push_back(entry->statement);
				} else if (entry->b_type == t_source_fork) {
					//find all stmts in this fork
					for (BufStateTblType::iterator itr3 = drvMap->begin(); itr3 != drvMap->end(); itr3++) {
						if (itr3->second->b_type != t_fork_dummy) continue;
						if (strcmp(itr3->second->inplace, entry->buf->name) == 0) {
							itr3->second->statement->weightSet = false;	      
							pStmts.push_back(itr3->second->statement);
						}    
					}
				}

				for (list<stmt*>::iterator itr4 = pStmts.begin(); itr4 != pStmts.end(); ++itr4) {
					if ((*itr4)->weightSet) continue;
					calc_weight((*itr4), drvMap);
					//find whether a weight group with this weight already exists, if so, insert the stmt. If not, create a weight group, insert the stmt.
					vector<WeightGroup*>::iterator pre = lower_bound(vSGroups.begin(), vSGroups.end(), (*itr4)->weight, gtwg());
					WeightGroup* wg = NULL;
					if (pre != vSGroups.end() && (*pre)->weight == (*itr4)->weight) {
						//found, stmt will be added to this group
						wg = *pre;
					} else {
						//not found, create a new group, put into the vector
						wg = new WeightGroup((*itr4)->weight);
						vSGroups.insert(pre, wg);
					}
					wg->addStmt(*itr4); //at this point I do not care about sorting by priority, that is taken care of by children classes
					++totalSourceStmt;
				}
			} 
		}
	}

	void WeightProfile::update_priority(StmtEntry* se) {
		querySchdl::SMLOG(10, "Entering WeightProfile::update_priority");	
		StmtEntry* se2 = update_priority(se->pStmt);
		se->oriPriority = se2->oriPriority;
		se->priority = se2->priority;
		se->expectedQMax = se2->expectedQMax;
		se->outputProcTime = se2->outputProcTime;    
		return;
	}

	void WeightProfile::printStateTable() {
		querySchdl::SMLOG(10, "Entering WeightProfile::printStateTable");	
		cout << "current profile has " << (stmtWithDeadline?"":"no") << " statement with deadline, there are " << vSGroups.size() << " WeightGroups, for each group, the order of stmts is the following (stmt, priority): " << endl;

		for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); ++itr) {
			cout << "Group with weight " << (*itr)->weight << " (name, priority, priority_before_adjust_for_time, expected_tolerable_max_queue_length, quota, quota_left,rankSet flags, outputProcTime, deadline):" <<endl;

			for (vector<StmtEntry*>::const_iterator itr2 = (*itr)->getEntries()->begin(); itr2 != (*itr)->getEntries()->end(); ++itr2) {
				cout.setf(ios_base::fixed);
				cout << "(" << *((*itr2)->stmt_name) << "," << (*itr2)->priority << "," << (*itr2)->oriPriority << "," << (*itr2)->expectedQMax << ","
					<< (*itr2)->pStmt->quota << "," << (*itr2)->pStmt->quota_left << ",rankSet " << ((*itr2)->pStmt->rankSet?"true,":"false,") << (*itr2)->outputProcTime << "," << (*itr2)->pStmt->deadline << ")";
			}
		}

		cout << endl << endl;

		//cout << "quota, quota_left and rankSet flags in stmts:" << endl;

		//for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
		//  cout << "(" << (*itr)->name << "," << (*itr)->quota << "," << (*itr)->quota_left << ",rankSet " << ((*itr)->rankSet?"true)":"false)");
		//}

		//cout << endl << endl;  

		drv->printStateTable(&(this->queryGraph));

		return;
	}


	int WeightGroupSchedule::AVG_PER_JOB_ROUND = 500; //changable

	void WeightGroupSchedule::printStateTable() { }

	void WeightGroupSchedule::update() {
		//this->stateTableChanged = true;
		this->drv->stateTableChanged = true;
	}

	run_rc WeightGroupSchedule::runState(b_entry state, int &nTuples, bool* output) {
		//if (querySchdl::verbose && state->b_type != t_sink && state->b_type != t_merge_sink) {
		//cout << "about to run stmt " << state->statement->name << ", Tuple remained to process for current round is " << nTuples << endl;
		//}

		if (querySchdl::verbose && state->b_type != t_sink && state->b_type != t_merge_sink && ((state->b_type != t_fork_dummy && !state->buf->empty()) || state->b_type == t_fork_dummy)) {
			if (state->b_type != t_fork && state->b_type != t_source_fork && state->b_type != t_merge_fork) {
				cout << "about to run non-empty stmt " << state->statement->name << ", Tuple remained to process for current round is " << nTuples << ", state type is " << buffer_t_strs[state->b_type] << endl;
			} else {
				cout << "about to run fork buffer " << state->buf->name << ", Tuple remained to process for current round is " << nTuples << ", state type is " << buffer_t_strs[state->b_type] << endl;
			}
		}

		quota_mode mode = this->strategy->getWeightProfile()->getMode();

		DrvMgr* dm = DrvMgr::getInstance();
		Monitor* monitor = dm->getMonitor();
		WeightProfile* wp = this->strategy->getWeightProfile();
		BufStateTblType* segmentMap = wp->getQueryGraph();

		this->recursion_count++;

		if (this->recursion_count > 10 && (state->b_type == t_source ||
					state->b_type == t_source_fork)) {
			//if(this->recursion_count > 100) {
			printf("we have a recursion count overflow\n");fflush(stdout);
			this->recursion_count = 0;
			return run_success;
		}


		//monitor->updateLastInTurn(state->statement);

		//first see whether this statement needs updating its quota, for now ignore quota changes based on feedback. This only takes care of changes due to initial stat availability
		stmt* s = state->statement;
		if (s!= NULL && state->b_type != t_fork && state->b_type != t_merge_fork) {
			stmt_entry* stat = monitor->getStmtStat(s);
			//if (s->rank.quota == this->default_quota && !s->rankSet && !this->fixed_quota && isUpdateTrigger(s)) {
			if (!s->rankSet && wp->isUpdateTrigger(s)) {
				if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
					if (querySchdl::verbose) {
						cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update quota for statement " << s->name << endl;
					}

					//this is not included in regular scheduling overhead.
					struct timeval tv_start;
					struct timezone tz;
					gettimeofday(&tv_start, &tz);

					strategy->update(); //this set flag stateTableChanged.

					struct timeval tv_end;
					gettimeofday(&tv_end, &tz);
					double diff_proc = timeval_subtract(tv_end, tv_start);
					if (monitor->total_processing_time.on) {
						monitor->total_processing_time.value += diff_proc;
					}      

					this->last_stmt = NULL; //indicate to use the first stmt

					return run_success; //need to return since underlying list<> and segment table changed
				}
			}

			//if selectivity or process rate changed significantly (more than some threshold), then update all priorities to reflect latest stat data
			if (stat!= NULL && s->rankSet && wp->change_significant(stat)) {
				//this is not included in regular scheduling overhead.
				struct timeval tv_start;
				struct timezone tz;
				gettimeofday(&tv_start, &tz);

				strategy->update(); //this set flag stateTableChanged.

				struct timeval tv_end;
				gettimeofday(&tv_end, &tz);
				double diff_proc = timeval_subtract(tv_end, tv_start);
				if (monitor->total_processing_time.on) {
					monitor->total_processing_time.value += diff_proc;
				}      

				if (querySchdl::verbose) {
					cout << "stat changes are above threshold, update all priorities." << endl;
					strategy->printStateTable();
				} 
				this->last_stmt = NULL;

				return run_success; //need to return since underlying list<> and segment table changed
			}
		}
		if (querySchdl::verbose && !s->in->empty()) cout << "weightgroup size is " << wg->getEntries()->size() << ", current stmt is " << s->name << endl;

		//if ((state->b_type != t_fork_dummy && !state->buf->empty()) || state->b_type == t_fork_dummy) cout << 0 << endl;

		if (state->b_type == t_source || state->b_type == t_source_fork) {
			if (state->buf->empty()) {
				//cout << "empty in buffer: " << state->buf->name << ", type " << state->buf->type << " " << endl;

				if (monitor->total_empty_buffer_hit.on) {
					monitor->total_empty_buffer_hit.value++;
				}

				//if (querySchdl::verbose) cout << "source buffer empty, finish this segment." << endl;
				monitor->updateLastInTurn(state->statement);

				return run_no_input;
			} else {
				//if (querySchdl::verbose) cout << "quota left for statement " << s->name << " is " << s->quota_left << endl;		
			}

			//Also need to check my own quota_left. This is needed since I may be a result of branch backtracking
			//if multiple backtracks go to the same source buffer, then my quota may have been exhausted. Can not solely rely on checking in RunNextUnit()
			//if ((mode == QUOTA_COUNT_MODE && s->quota_left <= 0) || (mode == QUOTA_TIME_MODE && s->time_quota_left <= 0)) {
			if (s->quota_left <= 0) {
				//if it is a fork buffer, can not check here, has to deligate to fork-dummy operation to check here, otherwise may cause infinite loop
				if (state->b_type != t_fork && state->b_type != t_source_fork && state->b_type != t_merge_fork) {
					if (querySchdl::verbose) cout << "quota consumed for statement " << s->name << endl;	
					return run_success;
				}
			}

			if (nTuples <= 0) {
				if (querySchdl::verbose) cout << "finished one driver round." << endl;
				return run_success;
			}

		}

		//dbt* t = NULL;
		run_rc rc;
		stmt_rc s_rc;

		buffer* buf;
		int nFork = 0;
		int cnt = 0;
		BufStateTblType::iterator itr;
		list<b_entry>::iterator itr2;
		list<b_entry> dummies;

		switch (state->action) {
			case act_cjump:
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act cJump, tuple quota to process is " << nTuples << endl;
				//currently, this can be a normal or source buffer
				if (state->b_type != t_normal && state->b_type != t_source) {
					if (querySchdl::verbose) cout << "internal state table error. cjump action with wrong buffer type: "
						<< buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				if (state->buf->empty()) {
					if (monitor->total_empty_buffer_hit.on) {
						monitor->total_empty_buffer_hit.value++;
					}

					//directly move the state since I am empty and not an unsynchronized union
					//I know this can not be a merge buffer
					//the source case is already checked before the case statement
					monitor->updateLastInTurn(state->statement);
					//if strategy is changed, this flag is set, return immediately
					if (this->drv->stateTableChanged) {
						if (querySchdl::verbose) cout << "strategy Changed" << endl;
						return run_success; //leave immediately after underlying table change.
					}

					return runState((*segmentMap)[state->back.back_buf], nTuples, output);
				}

				if ((int)(s_rc = (monitor->exeStmt(state->statement, drv))) > 0 || (int) s_rc == s_no_output || (int)s_rc == s_failure) {
					if (querySchdl::verbose) {
						cout << "return code in cJump is " << (int) s_rc << endl;
						printf("return code in cJump is %d",(int)s_rc);
						fflush(stdout);
					}

					if (output != NULL && s_rc > 0) (*output) = true;

					//I need to pop the tupple now
					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
						//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
						buffer* b = state->statement->in;
						//if the other "in" is not a fork buffer, we directly pop it.
						//If it is a fork buffer, then we need to know which dummy to record a pop.
						b_entry entry2 = (*segmentMap)[b->name];
						if (entry2->b_type != t_fork && entry2->b_type != t_source_fork && entry2->b_type != t_merge_fork) {
							b->pop();
							if (querySchdl::verbose) cout << "poped " << b->name << (b->empty()?" empty":" not empty") << endl;

							if (entry2->b_type == t_source) {
								nTuples--;
								//also need to update its corresponding quota_left
								//if (mode == QUOTA_COUNT_MODE) {
								entry2->statement->quota_left--;
								if (entry2->statement->quota_left < 0) { //allow off 1 to let downstream operators to finish
									return run_success;
								}
								//} else if (mode == QUOTA_TIME_MODE) {
								//entry2->statement->time_quota_left = ??
								//if (entry2->statement->time_quota_left < 0) { //allow off 1 to let downstream operators to finish
								//  return run_success;
								//}
								//}

								if (nTuples < 0) { //allow off 1 to let downstream operators to finish
									if (querySchdl::verbose) cout << "finished one driver round." << endl;
									return run_success;
								}
							}	  	  
						} else {
							//The buffer I need to pop is a fork.
							//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
							for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
								if (itr->second->b_type != t_fork_dummy) continue;
								if (strcmp(itr->second->inplace, b->name) == 0) {
									if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
										(itr->second->pop());
									}
								}    
							}
						}
					} else {
						state->buf->pop();
						if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

						if (state->b_type == t_source) {
							nTuples--;
							//also need to update its corresponding quota_left
							//if (mode == QUOTA_COUNT_MODE) {
							s->quota_left--;
							if (s->quota_left < 0) { //allow off 1 to let downstream operators to finish
								return run_success;
							}
							//} else if (mode == QUOTA_TIME_MODE) {
							//s->time_quota_left = ??
							//if (s->time_quota_left < 0) { //allow off 1 to let downstream operators to finish
							//  return run_success;
							//}
							//}

							if (nTuples < 0) { //allow off 1 to let downstream operators to finish
								if (querySchdl::verbose) cout << "finished one driver round." << endl;
								return run_success;
							}
						}	  	
					}  

					//now move the state
					if ((int) s_rc > 0) {
						//I know this can not be a fork, just use the correct field
						char* nextBuf = state->forward.forward_buf;
						if ((*segmentMap)[nextBuf]->b_type == t_sink
								|| (*segmentMap)[nextBuf]->b_type == t_merge_sink) {  //condition on this line used to be commented out to do merge and find a buffer to execute (among the merge buffers)
									rc = runState((*segmentMap)[state->inplace], nTuples);
									if (rc == run_failure) return rc;
								} else {
									rc = runState((*segmentMap)[nextBuf], nTuples);
									if (rc == run_failure) return rc;
								}

						return run_success;
					} else if ((int)s_rc == s_no_output) {
						rc = runState((*segmentMap)[state->inplace], nTuples, output);

						if (rc == run_failure) return rc;
						return run_success;
					} else if ((int)s_rc == s_failure) {
						return run_failure;
					}
				} else if ((int)s_rc == s_no_input) {
					if (querySchdl::verbose) cout << "return code in cJump is " << (int) s_rc << endl;

					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
						//retrack to the buffer which is before the one set to "in" of the statement
						return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
					} else if (state->statement->type == stmt_join) {
						//retrack to the buffer which is before the one set to "back_buf" of the statement
						jStmt* s = static_cast<jStmt*> (state->statement);
						if (s->back_buf != NULL && s->back_buf != 0) {
							b_entry back_entry = (*segmentMap)[s->back_buf->name];

							if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
								return run_no_input;	    
							} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
								if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
									return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
								} else {
									if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
									return run_failure;
								}
							} else {
								return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
							}
						}
					} 
					//else {	
					if (state->b_type == t_source) {
						return run_no_input; //this should not have happened
					} else {
						//I know this can not be a merge buffer
						return runState((*segmentMap)[state->back.back_buf], nTuples, output);
					}
					//}
				}
				break;

			case act_merge:
				//currently, this can only be a merge_sink buffer
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act merge, tuple quota to process is " << nTuples << endl;

				if (state->b_type != t_merge_sink) {
					if (querySchdl::verbose) cout << "internal state table error. merge action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				/*
				//check input buffers of the previous union statement
				uStmt* s;

				if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
				s = static_cast<uStmt*> (state->back.union_stmt);

				return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, false);
				} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
				s = static_cast<uStmt*> (state->back.union_stmt);

				return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, true);
				} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
				return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, true);	
				} else {
				if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
				}      
				 */

				return run_success; //just do nothing in this state, not needed.
				break;

			case act_merge_jump:
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act merge_jump, tuple quota to process is " << nTuples << endl;

				//currently, this can only be a merge buffer
				if (state->b_type != t_merge) {
					if (querySchdl::verbose) cout << "internal state table error. merge jump action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//if my buffer has tuples in there, execute. If not, back track to the correct buffer
				if (state->buf->empty()) {
					if (monitor->total_empty_buffer_hit.on) {
						monitor->total_empty_buffer_hit.value++;
					}

					//check input buffers of the previous union statement
					monitor->updateLastInTurn(state->statement);
					//if strategy is changed, this flag is set, return immediately
					if (this->drv->stateTableChanged) {
						if (querySchdl::verbose) cout << "strategy Changed" << endl;
						return run_success; //leave immediately after underlying table change.
					}

					uStmt* s;

					if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);	
						return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);	
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
						return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);
					} else {
						if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
						return run_failure;
					}      
				} else {
					//process the tuple
					if ((int)(s_rc = monitor->exeStmt(state->statement, drv)) > 0 || (int)s_rc == s_no_output || (int)s_rc == s_failure) {
						if (output != NULL && s_rc > 0) (*output) = true;

						//I need to pop the tupple now
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
							//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
							buffer* b = state->statement->in;
							//if the other "in" is not a fork buffer, we directly pop it.
							//If it is a fork buffer, then we need to know which dummy to record a pop.
							b_entry entry2 = (*segmentMap)[b->name];
							if (entry2->b_type != t_fork && entry2->b_type != t_source_fork && entry2->b_type != t_merge_fork) {
								b->pop();
								if (querySchdl::verbose) cout << "poped " << b->name << (b->empty()?" empty":" not empty") << endl;

								if (entry2->b_type == t_source) {
									nTuples--;
									//also need to update its corresponding quota_left
									//if (mode == QUOTA_COUNT_MODE) {
									entry2->statement->quota_left--;
									if (entry2->statement->quota_left < 0) { //allow off 1 to let downstream operators to finish
										return run_success;
									}
									//} else if (mode == QUOTA_TIME_MODE) {
									//entry2->statement->time_quota_left = ??
									//if (entry2->statement->time_quota_left < 0) { //allow off 1 to let downstream operators to finish
									//  return run_success;
									//}
									//}

									if (nTuples < 0) { //allow off 1 to let downstream operators to finish
										if (querySchdl::verbose) cout << "finished one driver round." << endl;
										return run_success;
									}
								}	  	  
							} else {
								//The buffer I need to pop is a fork.
								//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
								for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
									if (itr->second->b_type != t_fork_dummy) continue;
									if (strcmp(itr->second->inplace, b->name) == 0) {
										if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
											(itr->second->pop());
										}
									}    
								}
							}
						} else {
							state->buf->pop();
							if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

							if (state->b_type == t_source) {
								nTuples--;
								//also need to update its corresponding quota_left
								//if (mode == QUOTA_COUNT_MODE) {
								s->quota_left--;
								if (s->quota_left < 0) { //allow off 1 to let downstream operators to finish
									return run_success;
								}
								//} else if (mode == QUOTA_TIME_MODE) {
								//s->time_quota_left = ??
								//if (s->time_quota_left < 0) { //allow off 1 to let downstream operators to finish
								//  return run_success;
								//}
								//}

								if (nTuples < 0) { //allow off 1 to let downstream operators to finish
									if (querySchdl::verbose) cout << "finished one driver round." << endl;
									return run_success;
								}
							}	  	
						}  	

						//move the state
						if ((int) s_rc > 0) {
							//I know this can not be a fork, just use the correct field
							char* nextBuf = state->forward.forward_buf;
							if ((*segmentMap)[nextBuf]->b_type == t_sink) { //can not be merge_sink either
								return runState((*segmentMap)[state->inplace], nTuples);

							} else {
								return runState((*segmentMap)[nextBuf], nTuples);
							}
						} else if ((int)s_rc == s_no_output) {
							return runState((*segmentMap)[state->inplace], nTuples, output);
						} else if ((int)s_rc == s_failure) {
							return run_failure;
						} 
					} else if ((int)s_rc == s_no_input) {
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
							//retrack to the buffer which is before the one set to "in" of the statement
							return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
						} else if (state->statement->type == stmt_join) {
							//retrack to the buffer which is before the one set to "back_buf" of the statement
							jStmt* s = static_cast<jStmt*> (state->statement);
							if (s->back_buf != NULL && s->back_buf != 0) {
								b_entry back_entry = (*segmentMap)[s->back_buf->name];

								if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
									return run_success;	    
								} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
									if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
										return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
									} else {
										if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
										return run_failure;
									}
								} else {
									return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
								}
							}
						}
						//else {
						//check input buffers of the previous union statement
						uStmt* s;

						if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);
							return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);
							return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
							return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);	
						} else {
							if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
							return run_failure;
						}	  
						//}
					}
				}
				break;

			case act_cfork:
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act cFork, tuple quota to process is " << nTuples << endl;
				//currently, this can be a fork or source fork buffer
				if (state->b_type != t_fork && state->b_type != t_source_fork) {
					if (querySchdl::verbose) cout << "internal state table error. cfork action with wrong buffer type: "
						<< buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				if (state->buf->empty()) {
					monitor->updateLastInTurn(state->statement);
					//if strategy is changed, this flag is set, return immediately
					if (this->drv->stateTableChanged) {
						if (querySchdl::verbose) cout << "strategy Changed" << endl;
						return run_success; //leave immediately after underlying table change.
					}

					//I know I am not a merge buffer
					//the source case is already handled.
					return runState((*segmentMap)[state->back.back_buf], nTuples, output);
				}

				nFork = state->forward.fork_count;
				//now find out the dummy states, the # should agree with nFork
				cnt = 0;
				for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
					if (itr->second->b_type != t_fork_dummy) continue;
					if (strcmp(itr->second->inplace, state->buf->name) == 0) {
						dummies.push_back(itr->second);
						cnt++;
					}    
				}
				if (nFork != cnt) {
					cerr << "internal error, number of dummy states does not match the fork count for buffer: " << state->buf->name << endl;
					return run_failure;
				}

				itr2 = dummies.begin();

				while (1) {
					if (state->b_type == t_source_fork) {
						//I am a source, first test if the round already finished
						if (nTuples <= 0) {
							return run_success;
						}
					}

					//check if this branch has been processed before.
					b_entry entry = *itr2;
					if (querySchdl::verbose) cout << "try fork branch statement " << entry->statement->name<< endl;	  	
					if (entry->back.processed == true) {

						nFork--;
						itr2++;
					} else {

						rc = runState(entry, nTuples, output);      

						if (nFork == cnt && rc == run_no_input) {
							if (state->b_type == t_source_fork) {
								//if (querySchdl::verbose) cout << "source buffer empty, finish this branch." << endl;	  
								return run_success;
							} else {
								//this is not be a merge buffer
								return runState((*segmentMap)[state->back.back_buf], nTuples, output);
							}
						} else if (rc == run_failure) {
							return run_failure;
						}

						//advance the branch iterator only if I am processed (I may not, if my downstream is a join)
						if ((*itr2)->back.processed != true) {
							continue;
						} else {
							if (querySchdl::verbose) cout << "move fork pointer to process next one." << endl;
							nFork--;
							itr2++;
						}
					}

					if (nFork > 0 && itr2 == dummies.end()) {
						cerr << "internal error, dummy state list length and fork count mismatch for buffer: " << state->buf->name << endl;
						return run_failure;
					}

					if (nFork == 0 && itr2 == dummies.end()) {
						//the last fork branch is already called
						//make sure all are processed, otherwise, continue to loop
						bool finished = true;
						nFork = cnt;
						for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end() && finished; itr3++) {
							if ((*itr3)->back.processed == false) {
								finished = false;
								itr2 = itr3;
							} else {
								nFork--;
							}
						}

						if (finished) {	  
							//reset every flag, since we have finished one round for all branches.
							for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
								(*itr3)->back.processed = false;
							}

							state->buf->pop();
							if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

							if (state->b_type == t_source_fork) {
								nTuples--;
								//also need to update its corresponding quota_left
								//if (mode == QUOTA_COUNT_MODE) {
								s->quota_left--;
								if (s->quota_left < 0) { //allow off 1 to let downstream operators to finish
									return run_success;
								}
								//} else if (mode == QUOTA_TIME_MODE) {
								//s->time_quota_left = ??
								//if (s->time_quota_left < 0) { //allow off 1 to let downstream operators to finish
								//  return run_success;
								//}
								//}	      

								if (nTuples < 0) { //allow off 1 to let downstream operators to finish
									if (querySchdl::verbose) cout << "finished one driver round." << endl;
									return run_success;
								}
							}	  	

							//now move the state, first for the buffer empty case
							if (state->b_type == t_source_fork) {
								if (state->buf->empty()) {
									if (monitor->total_empty_buffer_hit.on) {
										monitor->total_empty_buffer_hit.value++;
									}

									monitor->updateLastInTurn(state->statement);

									//if (querySchdl::verbose) cout << "source buffer empty, finish this branch." << endl;
									return run_success;
								}
							} else if (state->buf->empty()) {
								if (monitor->total_empty_buffer_hit.on) {
									monitor->total_empty_buffer_hit.value++;
								}

								monitor->updateLastInTurn(state->statement);
								//if strategy is changed, this flag is set, return immediately
								if (this->drv->stateTableChanged) {
									if (querySchdl::verbose) cout << "strategy Changed" << endl;
									return run_success; //leave immediately after underlying table change.
								}

								//I know I am not a merge buffer
								return runState((*segmentMap)[state->back.back_buf], nTuples, output);
							}

							//non-empty case
							//restart the while loop
							nFork = cnt;
							itr2 = dummies.begin();
							if (querySchdl::verbose) cout << "fork flags reset for next round" << endl;
						} // end if (finished)
					} //end if (reaching the end of branches)
				} //end while

				break;

			case act_merge_cfork:
				if (querySchdl::verbose) cout << "state " << state->buf->name << ", act merge_cFork, tuple quota to process is " << nTuples << endl;
				//currently, this can only be a merge-fork buffer
				if (state->b_type != t_merge_fork) {
					if (querySchdl::verbose) cout << "internal state table error. merge cfork action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//if my buffer has tuples in there, execute. If not, back track to the correct buffer
				if (state->buf->empty()) {
					if (monitor->total_empty_buffer_hit.on) {
						monitor->total_empty_buffer_hit.value++;
					}

					monitor->updateLastInTurn(state->statement);
					//if strategy is changed, this flag is set, return immediately
					if (this->drv->stateTableChanged) {
						if (querySchdl::verbose) cout << "strategy Changed" << endl;
						return run_success; //leave immediately after underlying table change.
					}

					//check input buffers of the previous union statement
					uStmt* s;

					if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);	
						return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
						s = static_cast<uStmt*> (state->back.union_stmt);
						return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);	
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
						return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);
					} else {
						if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
						return run_failure;
					}      
				} else {
					//we move forward      
					nFork = state->forward.fork_count;
					//now find out the dummy states, the # should agree with nFork
					cnt = 0;
					for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
						if (itr->second->b_type != t_fork_dummy) continue;
						if (strcmp(itr->second->inplace, state->buf->name) == 0) {
							dummies.push_back(itr->second);
							cnt++;
						}    
					}
					if (nFork != cnt) {
						cerr << "internal error, number of dummy states does not match the fork count for buffer: " << state->buf->name << endl;
						return run_failure;
					}

					itr2 = dummies.begin();
					while (1) {
						//check if this branch has been processed before.
						b_entry entry = (*itr2);
						if (entry->back.processed == true) {
							nFork--;
							itr2++;
						} else {
							rc = runState(entry, nTuples, output);
							if (nFork == cnt && rc == run_no_input) {
								if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
									//retrack to the buffer which is before the one set to "in" of the statement
									return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
								} else if (state->statement->type == stmt_join) {
									//retrack to the buffer which is before the one set to "back_buf" of the statement
									jStmt* s = static_cast<jStmt*> (state->statement);
									if (s->back_buf != NULL && s->back_buf != 0) {
										b_entry back_entry = (*segmentMap)[s->back_buf->name];

										if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
											return run_success;	    
										} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
											if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
												uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
												return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
											} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
												uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
												return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
											} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
												return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
											} else {
												if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
												return run_failure;
											}
										} else {
											return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
										}
									}
								} else {
									//need to go to the upstream of myself.
									//check input buffers of the previous union statement
									uStmt* s;	    
									if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
										s = static_cast<uStmt*> (state->back.union_stmt);
										return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
									} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
										s = static_cast<uStmt*> (state->back.union_stmt);
										return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
									} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
										return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);	
									} else {
										if (querySchdl::verbose) cout << "internal error. merge cfork action with wrong statement type. "  << endl;
										return run_failure;
									}
								}
							} else if (rc == run_failure) {
								return run_failure;
							}

							//advance the branch iterator only if I am processed (I may not, if my downstream is a union)
							if ((*itr2)->back.processed != true) {
								continue;
							} else {
								if (querySchdl::verbose) cout << "move fork pointer to process next one." << endl;
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
							//make sure all are processed, otherwise, continue to loop
							bool finished = true;
							nFork = cnt;
							for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end() && finished; itr3++) {
								if ((*itr3)->back.processed == false) {
									finished = false;
									itr2 = itr3;
								} else {
									nFork--;
								}
							}

							if (finished) {
								//reset every flag, since we have finished one round for all branches.
								for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
									(*itr3)->back.processed = false;
								}

								state->buf->pop();
								if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

								//restart the while loop, and reset all processed flag on branches.
								nFork = cnt;
								itr2 = dummies.begin();
								if (querySchdl::verbose) cout << "fork flags reset for next round" << endl;
							} // end if (finished)
						} //end if (reaching the end of branches)
					} //end while
				} //end if (empty buffer)
				break;

			case act_jump:
				if (querySchdl::verbose) cout << "state " << state->inplace << state->forward.forward_buf << ", act jump, tuple quota to process is " << nTuples << endl;
				//currently, this can be a fork dummy buffer
				//this will not check for source, or pop the tuple, unlike cjump.
				if (state->b_type != t_fork_dummy) {
					if (querySchdl::verbose) cout << "internal state table error. jump action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
					return run_failure;
				}

				//I am already processed in this round of fork
				if (state->back.processed) {
					return run_success;
				}

				if ((int)(s_rc = monitor->exeStmt(state->statement, drv)) > 0 || (int)s_rc == s_no_output || (int)s_rc == s_failure) {

					//pop myself, unless the following buffer is a union. If union, I then need to look up the correct one to pop
					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
						//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
						buffer* b = state->statement->in;
						//if the other "in" is not a fork buffer, we directly pop it.
						//If it is a fork buffer, then we need to know which dummy to record a pop.
						b_entry entry2 = (*segmentMap)[b->name];
						if (entry2->b_type != t_fork && entry2->b_type != t_source_fork && entry2->b_type != t_merge_fork) {
							b->pop();
							if (querySchdl::verbose) cout << "poped " << b->name << (b->empty()?" empty":" not empty") << endl;

							if (entry2->b_type == t_source) {
								nTuples--;
								//also need to update its corresponding quota_left
								//if (mode == QUOTA_COUNT_MODE) {
								entry2->statement->quota_left--;
								if (entry2->statement->quota_left < 0) { //allow off 1 to let downstream operators to finish
									return run_success;
								}
								//} else if (mode == QUOTA_TIME_MODE) {
								//entry2->statement->time_quota_left = ??
								//if (entry2->statement->time_quota_left < 0) { //allow off 1 to let downstream operators to finish
								//  return run_success;
								//}
								//}

								if (nTuples < 0) { //allow off 1 to let downstream operators to finish
									if (querySchdl::verbose) cout << "finished one driver round." << endl;
									return run_success;
								}
							}	  	  
						} else {
							//The buffer I need to pop is a fork.
							//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
							for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
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
					if ((int) s_rc > 0) {
						//I know this can not be a fork, just use the correct field
						char* nextBuf = state->forward.forward_buf;
						if ((*segmentMap)[nextBuf]->b_type == t_sink
								|| (*segmentMap)[nextBuf]->b_type == t_merge_sink) {  //condition on this line used to be commented out to do merge and find a buffer to execute (among the merge buffers)
									return run_success;
								} else {
									rc = runState((*segmentMap)[nextBuf], nTuples);
									if (rc == run_failure) return rc;
								}
						return run_success;
					} else if ((int)s_rc == s_no_output) {
						return run_success;
					} else if ((int)s_rc == s_failure) {
						return run_failure;
					} 
				} else if ((int)s_rc == s_no_input) {
					if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
						//retrack to the buffer which is before the one set to "in" of the statement
						return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
					} else if (state->statement->type == stmt_join) {
						//retrack to the buffer which is before the one set to "back_buf" of the statement
						jStmt* s = static_cast<jStmt*> (state->statement);
						if (s->back_buf != NULL && s->back_buf != 0) {
							b_entry back_entry = (*segmentMap)[s->back_buf->name];

							if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
								return run_success;	    
							} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
								if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
									uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
									return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
								} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
									return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
								} else {
									if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
									return run_failure;
								}
							} else {
								return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
							}
						}
					} //else {
					return run_no_input;
					//}
				}
				break;    

			case act_none:
				if (querySchdl::verbose) cout << "null action for buffer: " << state->buf->name << endl;
				return run_success;

			default:
				if (querySchdl::verbose) cout << "action not recognizable for buffer: " << state->buf->name << endl;
				return run_failure;
		}

		//all correct returns should have happened in the switch statement. Error if I reach here.
		if (querySchdl::verbose) cout << "strategy error, switch statement does not return for: " << state->buf->name << endl;
		return run_failure;

		}

		ProfileSchedule::~ProfileSchedule() {
			for (vector<WeightGroupSchedule*>::iterator itr = vSGroupSchedules.begin(); itr != vSGroupSchedules.end(); ++itr) {
				delete *itr;
			}
		}  

		void ProfileSchedule::update() {
			if (vSGroupSchedules.empty()) {
				for (vector<WeightGroup*>::const_iterator itr = this->wp->getWeightGroups()->begin(); itr != this->wp->getWeightGroups()->end(); ++itr) {
					vSGroupSchedules.push_back(this->strategy->createWeightGroupSchedule(*itr));
				}
			} else {
				vector<WeightGroupSchedule*>::iterator itr2 = vSGroupSchedules.begin();
				bool endfound = false;
				for (vector<WeightGroup*>::const_iterator itr = this->wp->getWeightGroups()->begin(); itr != this->wp->getWeightGroups()->end(); ++itr) {
					if (!endfound && itr2 != vSGroupSchedules.end()) {
						(*itr2)->setWeightGroup(*itr);
						(*itr2)->update();
						++itr2;
					} else {
						vSGroupSchedules.push_back(this->strategy->createWeightGroupSchedule(*itr));
						endfound = true;
					}
				}    
			}
		}  

		void ProfileSchedule::printStateTable() {
			for (vector<WeightGroupSchedule*>::iterator itr = vSGroupSchedules.begin(); itr != vSGroupSchedules.end(); ++itr) {
				(*itr)->printStateTable();
			}
		}

		ScheduleStrategy::ScheduleStrategy(Driver* drv) {
			this->drv = drv;
			drv->setStrategy(this);

			this->schedule = NULL;
			this->profile = NULL;

			//update();
		}

		void ScheduleStrategy::setDriver(Driver* drv) {
			this->drv = drv;
			drv->setStrategy(this);

			update();
		}

		ScheduleStrategy::~ScheduleStrategy() {
			delete schedule;
			delete profile;
		}

		void ScheduleStrategy::update() {
			if (profile == NULL) {
				profile = createProfile();
			} else {
				profile->update();
			}
			if (schedule == NULL) {
				schedule = createSchedule();
			} else {
				schedule->setWeightProfile(profile);
				schedule->update();
			}
		}

		//to get it running for now, simplicity, just run the highest group. add more later.
		run_rc PriorityProfileSchedule::runNextUnit(int &nTuples) {
			//if (querySchdl::verbose) cout << "enter priorityProfileSchedule, there are " << vSGroupSchedules.size() << " groups" << endl;

			return (*(vSGroupSchedules.begin()))->runNextUnit(nTuples);
		}

		run_rc WRRGroupSchedule::runNextUnit(int &nTuples) {
			if (querySchdl::verbose) {
				//cout << "enter run weighted round robin " << endl;
				if (last_stmt != NULL) {
					//cout << "last statement to re-enter is " << last_stmt << endl;
				} else {
					//cout << "last_stmt is NULL" << endl;
				}
			}

			DrvMgr* dm = DrvMgr::getInstance();
			Monitor* monitor = dm->getMonitor();
			WeightProfile* wp = this->strategy->getWeightProfile();
			BufStateTblType* segmentMap = wp->getQueryGraph();
			quota_mode mode = this->strategy->getWeightProfile()->getMode();

			bool output = false;
			run_rc rc = run_success;
			stmt* s;

			//in RR mode, re-shuffle the vector after finish, to minimize arti`ficial effects of natural ordering
			//suspect to cost too much time, and also do-once not working out (end up shuffling when there is one stmt)
			//if (this->last_stmt == NULL && !shuffled && wg->getEntries()->front()->pStmt->quota == WeightOnlyProfile::DEFAULT_QUOTA
			//	&& wg->getEntries()->back()->pStmt->quota == WeightOnlyProfile::DEFAULT_QUOTA) {
			//  if (querySchdl::verbose) cout << "shuffle stmt entries."  << endl;
			//  random_shuffle(wg->getEntries()->begin(), wg->getEntries()->end());
			//  shuffled = true;
			//}

			this->drv->stateTableChanged = false;
			bool found = false;
			bool first = true;

			for (vector<StmtEntry*>::const_iterator itr = wg->getEntries()->begin(); itr != wg->getEntries()->end() && nTuples > 0; itr++) {
				if (this->last_stmt != NULL && !found) {
					if((*itr)->stmt_name->compare(this->last_stmt) == 0) {
						if (querySchdl::verbose) cout << "jump back into last time where it was left in the RR sequence" << endl;

						found = true;
					} else {
						continue;
					}
				}

				if (!(dm->stmtInUse(const_cast<char*>((*itr)->stmt_name->c_str())))) {
					//precaution, in case invalid statements has not been removed from driver.
					//this actually should not happen.
					if (querySchdl::verbose) cout << "find invalid entries on segment quota strategy stmt list."  << endl;
					continue;
				}

				s = (*itr)->pStmt;

				if (s == NULL || !(s->valid)) {
					if (querySchdl::verbose) {
						cout << "get invalid or NULL statement in segment strategy." << endl;
					}
					continue; //this really should not happen either
				}

				if (querySchdl::verbose && !s->in->empty()) cout << "runnextunit for loop, stmt " << s->name << ", quota_left is " << s->quota_left << endl;

				//update quota based on current queue size and deadline requirement, default to do nothing.
				if (found && first) {
					first = false;
				} else {
					updateQuotaByDeadline(*itr);	
				}

				rc = run_success;

				//while (nTuples > 0 && ((mode == QUOTA_COUNT_MODE && s->quota_left > 0) || (mode == QUOTA_TIME_MODE && s->time_quota_left > 0)) && rc == run_success) {
				while (nTuples > 0 && s->quota_left > 0 && rc == run_success) {	
					//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
					if (querySchdl::verbose && !s->in->empty()) cout << "runnextunit while loop, stmt " << s->name << ", quota_left is " << s->quota_left << endl;

					rc = runState((*segmentMap)[s->in->name], nTuples, &output);
					//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
					if (this->drv->stateTableChanged) {
						//this->drv->stateTableChanged = false;	  
						if (querySchdl::verbose) cout << "stateTableChanged 1" << endl;
						//break; //leave immediately after underlying table change.
						return run_success; //leave immediately after underlying table change.
					}
				}

				if (this->drv->stateTableChanged) {
					//this->drv->stateTableChanged = false;
					//break; //leave immediately after underlying table change.
					if (querySchdl::verbose) cout << "stateTableChanged 2" << endl;	
					return run_success; //leave immediately after underlying table change.	
				}

				if (nTuples > 0) {
					//if (mode == QUOTA_COUNT_MODE)
					s->quota_left = 0;
					//if (querySchdl::verbose) cout << "reset quota_left for stmt " << s->name << endl;	
					//else if (mode == QUOTA_TIME_MODE) s->time_quota_left = 0;
				}

				if (nTuples > 0 && wg->getEntries()->back()->stmt_name->compare(s->name) == 0) {
					//just finished the last one in the full round, it is time to reset quota_left for everyone.
					for (vector<StmtEntry*>::const_iterator itr2 = wg->getEntries()->begin(); itr2 != wg->getEntries()->end(); itr2++) {
						//if (mode == QUOTA_TIME_MODE) {
						//(*itr2)->pStmt->time_quota_left = (*itr2)->pStmt->time_quota;
						//} else {	  
						(*itr2)->pStmt->quota_left = (*itr2)->pStmt->quota;
						//if (querySchdl::verbose) cout << "Set quota_left for stmt " << s->name << " to " << (*itr2)->pStmt->quota_left << endl;	  
						//}
					}
					this->last_stmt = NULL;

				} else if (nTuples <= 0) {
					this->last_stmt = s->name;
				}

				//if (nTuples <= 0) {
				//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
				//}
				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << nTuples << endl;
					//cout << "new output tuple, print stats and strategy quota state during segment walking." << endl; 
					//monitor->printStateTables();
					//QuotaStrategy::printStateTable();
				}

			} //end for (segments)

			if (querySchdl::verbose && output) {
				//there are output tuples, print debug info for monitor stats
				//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
				//monitor->printStateTables();
				//strategy::printStateTable();
			}

			//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
			return rc;

			}

			//!!!!!!!!!!!!!!!----union, join not handled yet-------------
			void WRRDLGroupSchedule::updateQuotaByDeadline(StmtEntry* se) {

				stmt* s = se->pStmt;

				//for not do not care about it if no deadline is set
				//later can consider use a default deadline
				//when expectedQMax is 0, this means we do not have stat yet, can not predict processing time thus can not adjust for deadline this way
				if (s->deadline == 0 || se->expectedQMax == 0) return; 

				if (s->type == stmt_t_union || s->type == stmt_tl_union || s->type == stmt_join) {
					return; //do nothing
				}

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();

				//check queue size and double quota if size becomes above expected max queue size (based on processor share)
				double utilization = monitor->last_win_total_p_time;
				if (utilization < 0.5) utilization = 0.5;

				int cur_size = s->in->bufSize();
				if (cur_size == 0) {
					if(s->deadline_quota > 0) {
						s->deadline_quota = 0;
						if (querySchdl::verbose) cout << "deadline quota reset for stmt " << s->name << endl;
					}

					return;
				}

				//the original expectedQMax is currently calcualted by assuming utilization 1, correct it using utilization number, and deadline
				int max_size = (int)(ceil(((double)s->deadline)*utilization*se->expectedQMax));

				if (max_size > cur_size) {
					if(s->deadline_quota > 0) {
						s->deadline_quota = 0;
						if (querySchdl::verbose) cout << "deadline quota reset for stmt " << s->name << endl;
					}
				} else {
					if(s->deadline_quota == 0) s->deadline_quota = s->quota*2;
					else s->deadline_quota *= 2;

					s->quota_left = s->deadline_quota;
					if (querySchdl::verbose) cout << "deadline quota for stmt " << s->name << " increased to " << s->deadline_quota << endl;  
				}
			}

			bool exp_all_stat_set = false;

			run_rc PriorityGroupSchedule::runNextUnit(int &nTuples) {
				if (querySchdl::verbose) {
					//cout << "enter run priority-based groupschedule mechanism " << endl;
				}

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();
				bufferMngr* bm = bufferMngr::getInstance();

				WeightProfile* wp = this->strategy->getWeightProfile();
				BufStateTblType* segmentMap = wp->getQueryGraph();
				//quota_mode mode = this->strategy->getWeightProfile()->getMode();

				bm->new_arrival = 0;    
				bool first_tuple = true; 

				bool output = false;
				bool leave = false;

				run_rc rc = run_success;
				stmt* s;

				this->drv->stateTableChanged = false;

				//do some rounds of stat collection first, under this mode
				if (exp_batch_activate_mode && !exp_all_stat_set) {
					bool local_stat_set = true;

					for (vector<StmtEntry*>::const_iterator itr = wg->getEntries()->begin(); itr != wg->getEntries()->end() && nTuples > 0; itr++) {
						s = (*itr)->pStmt;
						if (!s->rankSet) local_stat_set = false;
					}
					if (local_stat_set) exp_all_stat_set = true;
				}

				if (exp_batch_activate_mode && !exp_all_stat_set) {    
					for (vector<StmtEntry*>::const_iterator itr = wg->getEntries()->begin(); itr != wg->getEntries()->end() && nTuples > 0; itr++) {
						s = (*itr)->pStmt;	
						if (s->rankSet) continue;
						else {
							//set to enough to gather initial stat in RR fashion
							s->quota = NUM_INPUT_BEFORE_SET_STAT*2;
							s->quota_left = NUM_INPUT_BEFORE_SET_STAT*2;

							while (nTuples > 0 && s->quota_left > 0 && rc == run_success) {	
								//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
								if (querySchdl::verbose && !s->in->empty()) cout << "runnextunit while loop, stmt " << s->name << ", quota_left is " << s->quota_left << endl;

								rc = runState((*segmentMap)[s->in->name], nTuples, &output);
								//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
								if (this->drv->stateTableChanged) {
									//this->drv->stateTableChanged = false;	  
									if (querySchdl::verbose) cout << "stateTableChanged 1" << endl;
									//break; //leave immediately after underlying table change.
									return run_success; //leave immediately after underlying table change.
								}
							}

							if (this->drv->stateTableChanged) {
								//this->drv->stateTableChanged = false;
								//break; //leave immediately after underlying table change.
								if (querySchdl::verbose) cout << "stateTableChanged 2" << endl;	
								return run_success; //leave immediately after underlying table change.	
							}
						}
					}
					return rc;
				}

				this->drv->stateTableChanged = false;

				//if (testocssegstart) cout << 1 << endl;

				int beginCount = nTuples;
				for (vector<StmtEntry*>::const_iterator itr = wg->getEntries()->begin(); itr != wg->getEntries()->end() && nTuples > 0 && (!(bm->new_arrival) || first_tuple) && !leave; itr++) {

					if (!(dm->stmtInUse(const_cast<char*>((*itr)->stmt_name->c_str())))) {
						//precaution, in case invalid statements has not been removed from driver.
						//this actually should not happen.
						if (querySchdl::verbose) cout << "reach invalid entries in priority-based groupschedule."  << endl;
						continue;
					}

					s = (*itr)->pStmt;

					if (s == NULL || !(s->valid)) {
						if (querySchdl::verbose) {
							cout << "get invalid statement in priority-based groupschedule." << endl;
						}
						continue; //this really should not have happened
					}

					//reset this to high value to prevent quota from having any effect. not very efficient, but ok for now.
					s->quota = WeightOnlyProfile::MAX_QUOTA;
					s->quota_left = WeightOnlyProfile::MAX_QUOTA;

					if (s->in->empty()) {
						if (monitor->total_empty_buffer_hit.on) {
							monitor->total_empty_buffer_hit.value++;
						}

						monitor->updateLastInTurn(s);

						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							//this->drv->stateTableChanged = false;
							if (querySchdl::verbose) cout << "strategy Changed +1" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						continue;
					} else {
						if (querySchdl::verbose) cout << "arrive for loop for non-empty stmt " << s->name << endl;
					}

					rc = run_success;
					//if (testocssegstart) cout << 2 << endl;

					while (nTuples > 0 && rc == run_success && (!(bm->new_arrival) || first_tuple) && !leave) {
						//if (testocssegstart) cout << "first tuple is " << (first_tuple?"true":"false") << ", new arrival is " << (bm->new_arrival?"true":"false") << ", nTuples is " << nTuples << ", stmt is " << s->name << endl;

						if (querySchdl::verbose) cout << "while loop new step for non-empty stmt " << s->name << endl;

						//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.	
						int count1 = 1;	
						rc = runState((*segmentMap)[s->in->name], count1, &output);
						if (count1 < 1) {
							nTuples--;
						} else {
							break;
						}

						if (nTuples < beginCount) {
							first_tuple = false;
							//testocssegstart = true;
						}
						//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
						if (this->drv->stateTableChanged) {
							leave = true;
							//break; //leave immediately after underlying table change.
							//this->drv->stateTableChanged = false;
							return run_success;
						}
					}

					if (this->drv->stateTableChanged) {
						//if (testocssegstart) cout << 3 << endl;
						//this->drv->stateTableChanged = false;
						leave = true;
						//break; //leave immediately after underlying table change.
						return run_success;
					}

					//if (nTuples <= 0) {
					//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
					//}

					if (querySchdl::verbose && output) {
						//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << nTuples << endl;
					}

				} //end for (segments)

				if (bm->new_arrival && querySchdl::verbose) {
					cout << "new arrival, return back to front of queue" << endl;
				}

				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
					//monitor->printStateTables();
					//printStateTable();
				}

				//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
				return rc;
			}

			/* //no longer needed, do it at checking time, since if a stmt does not have deadline set, there is no need to add it
			   void PriorityDLGroupSchedule::update() {
			   WeightGroupSchedule::update();

			//Construct the content of the deadline/delay information vector
			copy(wg->getEntries()->begin(), wg->getEntries()->end(), this->hSEntriesByTime.begin());

			//sort(this->vSEntriesByTime.begin(), this->vSEntriesByTime.end(), ltTimeSE());   //no longer needed, use a heap now
			}
			 */

			run_rc PriorityDLGroupSchedule::runNextUnit(int &nTuples) {
				if (!this->strategy->getWeightProfile()->stmtHasDeadline() || !deadlineMode || (exp_batch_activate_mode && !exp_all_stat_set)) {
					return PriorityGroupSchedule::runNextUnit(nTuples);
				}

				//deadline mode
				if (querySchdl::verbose) {
					cout << "enter run priority-based groupschedule mechanism under deadline mode" << endl;
				}

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();
				bufferMngr* bm = bufferMngr::getInstance();

				WeightProfile* wp = this->strategy->getWeightProfile();
				BufStateTblType* segmentMap = wp->getQueryGraph();

				bool output = false;
				bool leave = false;

				run_rc rc = run_success;
				stmt* s;

				this->drv->stateTableChanged = false;

				//if (testocssegstart) cout << 1 << endl;

				while (deadlineMode && nTuples > 0 && !leave) {
					StmtEntry* se = hSEntriesByTime.front();      
					s = se->pStmt;

					if (!(dm->stmtInUse(const_cast<char*>(se->stmt_name->c_str())))) {
						//precaution, in case invalid statements has not been removed from driver.
						//this actually should not happen.
						if (querySchdl::verbose) cout << "reach invalid entries in priority-based groupschedule under deadline mode."  << endl;
						pop_heap(hSEntriesByTime.begin(), hSEntriesByTime.end(), gtTimeSE());
						continue;
					}

					if (s == NULL || !(s->valid)) {
						if (querySchdl::verbose) {
							cout << "get invalid statement in priority-based groupschedule." << endl;
						}
						pop_heap(hSEntriesByTime.begin(), hSEntriesByTime.end(), gtTimeSE());	
						continue; //this really should not have happened
					}

					//reset this to high value to prevent quota from having any effect. not very efficient, but ok for now.
					s->quota = WeightOnlyProfile::MAX_QUOTA;
					s->quota_left = WeightOnlyProfile::MAX_QUOTA;      

					if (s->in->empty()) {
						if (monitor->total_empty_buffer_hit.on) {
							monitor->total_empty_buffer_hit.value++;
						}

						monitor->updateLastInTurn(s);
						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							//this->drv->stateTableChanged = false;
							if (querySchdl::verbose) cout << "strategy Changed +2" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						//the first stmt has emtpy buffer, this really should not have happened
						deadlineMode = false;
						break;
					}

					rc = run_success;
					//if (testocssegstart) cout << 2 << endl;

					//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
					int count1 = 1; 
					rc = runState((*segmentMap)[s->in->name], count1, &output);
					if (count1 < 1) {
						nTuples--;
					} else {
						break;
					}

					//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
					if (this->drv->stateTableChanged) {
						//if (testocssegstart) cout << 3 << endl;
						//this->drv->stateTableChanged = false;
						leave = true;
						//break; //leave immediately after underlying table change.
						return run_success;
					}

					//if (nTuples <= 0) {
					//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
					//}

					if (querySchdl::verbose && output) {
						//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << nTuples << endl;
					}

				} //end while (segments)    

				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
					//monitor->printStateTables();
					//printStateTable();
				}

				//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
				return rc;    
			}

			//also track delay information. For now only do so in source stmts, which also ensures that no need to check for union or join cases
			run_rc PriorityDLGroupSchedule::runState(b_entry state, int &nTuples, bool* output) {

				if (!this->strategy->getWeightProfile()->stmtHasDeadline() || (state->b_type != t_source && state->b_type != t_source_fork) || (exp_batch_activate_mode && !exp_all_stat_set))
					return PriorityGroupSchedule::runState(state, nTuples, output);

				//need to check for deadline mode, and set/reset it
				if (querySchdl::verbose) {
					cout << "enter runstate with deadline mode checking" << endl;
				}

				DrvMgr* dm = DrvMgr::getInstance();    
				Monitor* monitor = dm->getMonitor();

				//if there is a new window or if there has been 1000 tuples processed in the window, update deadline information
				//in case there are arrivals at previously empty buffers
				if (monitor->newWindow || (monitor->win_total_input.value % DEADLINE_CHECK_FREQUENCY_IN_COUNT) == 0) {
					//first update all information on deadlines in hSEntriesByTime, then make it into a heap
					//specifically, update urgencyTime in each entry
					//urgencyTime = tq+deadline-outputProcTime, where tq is the timestamp of the front tuple on the input queue
					//At source there can not be union and join stmts, no need to check
					if (!hSEntriesByTime.empty()) hSEntriesByTime.clear();

					struct timeval tq;
					if (querySchdl::verbose) cout << "Check deadline info, (stmt_name, urgencyTime), currently deadline mode is " << (deadlineMode?"true":"false") << endl;        
					for (vector<StmtEntry*>::iterator itr = wg->getEntries()->begin(); itr != wg->getEntries()->end(); ++itr) {
						stmt* s = (*itr)->pStmt;


						if (s->deadline == 0 || (*itr)->outputProcTime == 0) {
							continue;  //stat still not available, or stmt has no deadline set
						} else {
							hSEntriesByTime.push_back(*itr);

							if (!s->in->empty()) {
								s->in->get(&tq);

								(*itr)->urgencyTime = timeval_add(tq, s->deadline);
								(*itr)->urgencyTime = timeval_subtract((*itr)->urgencyTime, (*itr)->outputProcTime);

							} else {
								//buffer empty now, just reset urgencyTime to high value
								(*itr)->urgencyTime.tv_sec = 1999999999;
								(*itr)->urgencyTime.tv_usec = 0;

							}
							if (querySchdl::verbose) cout << "(" << (*((*itr)->stmt_name)) << ","
								<< ((*itr)->urgencyTime.tv_sec + (double)(*itr)->urgencyTime.tv_usec/1000000) << ")";
						}
					}

					if (querySchdl::verbose) cout << endl;        
					//now make it a heap, based on urgencyTime
					//use greater than as comp, since we want the smallest element, not the largest at front of heap
					if (!hSEntriesByTime.empty()) {
						make_heap(hSEntriesByTime.begin(), hSEntriesByTime.end(), gtTimeSE());
					}
				}


				//if normal mode, check to see whether there is tuples close to deadline (expected exit time larger than current time), if so switch over to deadline mode
				//if under deadline mode, check to see if there is still tuple satisfy the above condition. if not, switch back to normal mode.
				if (!deadlineMode && !hSEntriesByTime.empty()) {
					//now check for the minimum and see if we need to switch to deadlineMode
					struct timezone tz;
					struct timeval tv;      
					gettimeofday(&tv, &tz);

					if (timeval_cmp(hSEntriesByTime.front()->urgencyTime, tv) <= 0) {
						deadlineMode = true;
						return run_success;
					}
				} else if (deadlineMode) {
					//deadline mode, see if I should exit deadline mode
					struct timezone tz;
					struct timeval tv;
					gettimeofday(&tv, &tz);

					if (timeval_cmp(hSEntriesByTime.front()->urgencyTime, tv) > 0) {
						deadlineMode = false;
						return run_success;
					}
				}

				run_rc rc = PriorityGroupSchedule::runState(state, nTuples, output);

				//have to be source stmt to reach here, no need to check again
				if(deadlineMode && rc == run_success) {
					struct timeval tq;
					StmtEntry* se = hSEntriesByTime.front();
					stmt* s = se->pStmt;

					pop_heap(hSEntriesByTime.begin(), hSEntriesByTime.end(), gtTimeSE());

					if (!s->in->empty()) {
						s->in->get(&tq);

						se->urgencyTime = timeval_add(tq, s->deadline);
						se->urgencyTime = timeval_subtract(se->urgencyTime, se->outputProcTime);
					} else {
						//buffer empty now, just reset urgencyTime to high value
						se->urgencyTime.tv_sec = 1999999999;
						se->urgencyTime.tv_usec = 0; 
					}

					hSEntriesByTime.push_back(se);

					push_heap(hSEntriesByTime.begin(), hSEntriesByTime.end(), gtTimeSE());
				}

				return rc;
			}

			//int WeightOnlyProfile::DEFAULT_QUOTA = WeightGroupSchedule::AVG_PER_JOB_ROUND;
			int WeightOnlyProfile::DEFAULT_QUOTA = 150;  
			int WeightOnlyProfile::MIN_QUOTA = 5;

			void WeightOnlyProfile::update_quotas() {
				if (querySchdl::verbose) {
					cout << "update quotas in weightonly profile" << endl;
				}

				//assing each source statement default quota
				for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); itr++) {
					for (vector<StmtEntry*>::const_iterator itr2 = (*itr)->getEntries()->begin(); itr2 != (*itr)->getEntries()->end(); ++itr2) {
						//if (this->mode == QUOTA_TIME_MODE) {
						//time quota, do not know what to do yet
						//(*itr2)->pStmt->time_quota = WeightOnlyProfile::DEFAULT_QUOTA;
						//(*itr2)->pStmt->time_quota_left = WeightOnlyProfile::DEFAULT_QUOTA;
						//(*itr2)->pStmt->rank.priority = WeightOnlyProfile::DEFAULT_VALUE;
						//} else {
						(*itr2)->pStmt->quota = WeightOnlyProfile::DEFAULT_QUOTA;
						(*itr2)->pStmt->quota_left = WeightOnlyProfile::DEFAULT_QUOTA;
						(*itr2)->pStmt->deadline_quota = 0;	
						(*itr2)->pStmt->rank.priority = WeightOnlyProfile::DEFAULT_VALUE;
						if (querySchdl::verbose) {
							cout << "statement " << (*itr2)->pStmt->name << " quota_left set to " << (*itr2)->pStmt->quota_left << endl;
						}

						//}
					}
				}
			}

			bool OCSEntry::change_significant(stmt_entry* s_entry) {
				if (s_entry == NULL) return false;

				double selectivity = (double)s_entry->total_output.value/s_entry->total_input.value;
				double process_rate = (double)s_entry->total_input.value/s_entry->total_p_time.value;
				bool changed = false;
				if ((selectivity > 0 && s_entry->last_known_selectivity == 0) || (process_rate > 0 && s_entry->last_known_process_rate == 0)) {
					changed = true;
				} else if (s_entry->last_known_selectivity > 0 || s_entry->last_known_process_rate > 0) {
					if (s_entry->last_known_selectivity > 0 && abs((int)(selectivity - s_entry->last_known_selectivity))/s_entry->last_known_selectivity > OcsStrategy::SEL_CHANGE_THRESHOLD) {
						changed = true;
					}

					if (s_entry->last_known_process_rate > 0 && abs((int)(process_rate - s_entry->last_known_process_rate))/s_entry->last_known_process_rate > OcsStrategy::PROC_CHANGE_THRESHOLD) {
						changed = true;
					}
				}
				if (changed && querySchdl::verbose) {
					cout << "stat change determined to be significant. " << endl;
				}

				if (changed) {
					s_entry->last_known_selectivity = selectivity;
					s_entry->last_known_process_rate = process_rate;
				}

				return changed;
			}

			OCSProfile::~OCSProfile() {
				//remove all OCSEntries, which is specific to this class.
				for (vector<OCSEntry*>::iterator itr = ocsInfo.begin(); itr != ocsInfo.end(); ++itr) {
					delete (*itr);
				}
			}

			bool OCSProfile::change_significant(stmt_entry* s_entry) {
				return OCSEntry::change_significant(s_entry);
			}

			bool OCSProfile::isUpdateTrigger(stmt* s) {
				if (s != NULL) return s->monitor_sink_stat;
				return true;
			}

			double OCSProfile::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity*path_selectivity;
			}

			void OCSProfile::printStateTable() {
				querySchdl::SMLOG(10, "Entering OCSProfile::printStateTable");	
				WeightProfile::printStateTable();

				cout << "Algorithm specific states(path_capacity, path_selectivity): " << endl;
				cout.setf(ios_base::fixed);
				for (vector<OCSEntry*>::iterator itr = ocsInfo.begin(); itr != ocsInfo.end(); itr++) {
					cout << "(" << (*((*itr)->stmt_name)) << "," << (*itr)->path_capacity << "," << (*itr)->path_selectivity << ") ";
				}

				cout << endl << endl;
			}


			void OCSProfile::update() {
				querySchdl::SMLOG(10, "Entering OCSProfile::update");	
				WeightProfile::update();

				//first remove all OCSEntries if they exist
				if (!ocsInfo.empty()) {
					vector<OCSEntry*>::size_type oriSize = ocsInfo.size();
					for (vector<OCSEntry*>::iterator itr = ocsInfo.begin(); itr != ocsInfo.end(); ++itr) {
						delete (*itr);
					}
					ocsInfo.clear();
					ocsInfo.reserve(oriSize);
				}

				//allocate an OCSEntry for every statement, and sort by name, to facilitate finding
				//This vector is needed since The weightgroups only has the sources, but to calculate priority for ocs we need information for all stmts.
				//set default values for all priorities, quotas, etc, at this point.
				for (list<stmt*>::iterator itr = this->drv->getStmts()->begin(); itr != this->drv->getStmts()->end(); ++itr) {
					//if (this->mode == QUOTA_TIME_MODE) {
					//time quota, do not know what to do yet
					//(*itr)->time_quota = WeightOnlyProfile::DEFAULT_QUOTA;
					//(*itr)->time_quota_left = WeightOnlyProfile::DEFAULT_QUOTA;
					//} else {
					(*itr)->quota = WeightOnlyProfile::DEFAULT_QUOTA;
					(*itr)->quota_left = WeightOnlyProfile::DEFAULT_QUOTA;
					(*itr)->deadline_quota = 0;
					//}

					(*itr)->rank.priority = WeightProfile::DEFAULT_VALUE;
					(*itr)->rankSet = false;

					ocsInfo.push_back(new OCSEntry(*itr));
				}
				//now sort the vector by stmt_name for ease of searching
				sort(ocsInfo.begin(), ocsInfo.end(), ltse());

				//call update_priority for each stmt with source buffers
				maxPriority = 0;
				//minPriority = 0;
				totalSrcOriPriority = 0;
				totalSrcPriority = 0;
				for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); itr++) {
					for (vector<StmtEntry*>::iterator itr2 = (*itr)->getEntries()->begin(); itr2 != (*itr)->getEntries()->end(); ++itr2) {
						update_priority(*itr2);
						if ((*itr2)->pStmt->rankSet) {
							totalSrcOriPriority += (*itr2)->oriPriority;
							totalSrcPriority += (*itr2)->priority;

							//if (minPriority == 0) {
							//  minPriority = (*itr2)->priority;
							//} else if ((*itr2)->priority < minPriority) {
							//  minPriority = (*itr2)->priority;
							//}

							if ((*itr2)->priority > maxPriority) {
								maxPriority = (*itr2)->priority;
							}
						}
					}
				}

				//now that we have the priority, set the quotas. 
				update_quotas();
			}

			//if a statement does not have any stat yet, then put it at WeightProfile::DEFAULT_VALUE
			StmtEntry* OCSProfile::update_priority(stmt* s) {
				querySchdl::SMLOG(10, "Entering OCSProfile::update_priority");	
				//only deal with the case of setting it to non-default the first time, updating existing affect upstream operators, too complicated to handle here for now.
				if (s == NULL) {    
					return NULL;
				}

				if (querySchdl::verbose) cout << "update priority called for " << s->name << endl;

				OCSEntry* ocse;
				vector<OCSEntry*>::iterator itr = lower_bound(ocsInfo.begin(), ocsInfo.end(), s->name, ltse());
				if (itr != ocsInfo.end() && (*itr)->stmt_name->compare(s->name) == 0) {
					//found entry
					ocse = *itr;
				} else {
					ocse = new OCSEntry(s);
					ocsInfo.insert(itr, ocse);
				}
				if (s->rankSet) {
					return ocse;
				}

				//first make sure I have enough stats to calculate priority, if not, just return
				bool hasStat = false;
				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();
				stmt_entry* stat = monitor->getStmtStat(s);
				if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
					hasStat = true;
				}

				if(!hasStat) {
					if (querySchdl::verbose) cout << "stat not available yet for " << s->name << endl;
					return ocse;
				}

				if (querySchdl::verbose) cout << "stat available for " << s->name << ", calculate priority now." << endl;

				double priority = WeightProfile::DEFAULT_VALUE;
				double path_capacity = WeightProfile::DEFAULT_VALUE;
				double path_selectivity = WeightProfile::DEFAULT_VALUE;
				double outputProcTime = 0;

				//build from end of path, backwards towards the front of path. 
				//This way forks are easily handled (the upstream operator calculate priority based on the highest of is suceeding operators)
				//inductively, the last one Cp(k) = 1/(1/Co(k)), the prior one is Cp(k-1) = (1/Sigma(k-1)) / ((1/(Co(k-1)*Sigma(k-1))) + 1/Cp(k))
				//priority(k) = Op(k) = Cp(k) * Sigma-p(k)

				//first make sure all downstream operators have priorities, if not, update them first
				BufStateTblType* graph = drv->getBufGraph();
				b_entry graph_entry;
				if (graph->find(s->out->name) != graph->end()) {
					graph_entry = (*graph)[s->out->name];
					//if I am a sink, just calculate and return
					if(graph_entry->b_type == t_sink || graph_entry->b_type == t_merge_sink) {
						if (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT) {
							outputProcTime = stat->total_p_time.value/(double)stat->total_input.value;
							path_capacity = 1/outputProcTime;
							path_selectivity = (double)stat->total_output.value/stat->total_input.value;

							if (querySchdl::verbose) {
								cout << "for sink stmt " << s->name << ", the capacity is " << path_capacity << endl;
							}

						} else {
							outputProcTime = 1/stat->last_known_process_rate;
							path_capacity = stat->last_known_process_rate;
							path_selectivity = stat->last_known_selectivity;
						}
						priority = calculatePriority(path_capacity, path_selectivity, stat);
						//priority = path_capacity*path_selectivity;
					} else {
						//there are statements downstream of me.
						stmt* s2 = NULL;
						OCSEntry* se2;
						if (graph_entry->b_type == t_fork || graph_entry->b_type == t_source_fork) {
							//there are multiple immediate downstream statements, choose the highest one.
							int nFork = graph_entry->forward.fork_count;
							//now find out the dummy states, the # should agree with nFork
							int cnt = 0;
							list<b_entry> dummies;
							for (BufStateTblType::iterator itr3 = graph->begin(); itr3 != graph->end(); itr3++) {
								if (itr3->second->b_type != t_fork_dummy) continue;
								if (strcmp(itr3->second->inplace, s->out->name) == 0) {
									dummies.push_back(itr3->second);
									cnt++;
								}    
							}

							if (nFork == cnt) {
								bool available = true;	  
								for(list<b_entry>::iterator itr4 = dummies.begin(); itr4 != dummies.end() && available; itr4++) {
									stmt* s3 = (*itr4)->statement;
									OCSEntry* se3;
									if (!s3->rankSet) {
										//get downstream priority first, before calculating my own		
										se3 = static_cast<OCSEntry*> (update_priority(s3));
									}
									//check again after update, to see if info is available
									if (!s3->rankSet) {
										available = false;
										s2 = NULL;
										se2 = NULL;
										break;
									} else if (s2 = NULL) {
										s2 = s3;
										se2 = se3;
									} else {
										//make sure s2 is set to the branch with the highest priority
										if(s2->rank.priority < s3->rank.priority) {
											s2 = s3;
											se2 = se3;
										}
									}
								}
							}
						} else {
							//there is only one immediate downstream statement
							s2 = graph_entry->statement;
							if (!s2->rankSet) {
								//get downstream priority first, before calculating my own
								if (querySchdl::verbose) cout << "inside update entry to call downstream stmt update first: " << s->name << " call " << s2->name << endl;
								se2 = static_cast<OCSEntry*> (update_priority(s2));	  
							}
						}

						if (s2 != NULL && s2->rankSet) {
							double op_capacity, op_selectivity, op_outputProcTime;
							if (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT) {
								op_outputProcTime = stat->total_p_time.value/(double)stat->total_input.value;
								op_capacity = 1/op_outputProcTime;
								op_capacity = (double)stat->total_input.value/stat->total_p_time.value;
								op_selectivity = (double)stat->total_output.value/stat->total_input.value;
							} else {
								op_outputProcTime = 1/stat->last_known_process_rate;
								op_capacity = stat->last_known_process_rate;
								op_selectivity = stat->last_known_selectivity;
							}
							//if (querySchdl::verbose) {
							//  cout << "calc priority. op_capacity is " << op_capacity << ", op_selectivity is " << op_selectivity << endl;
							//}

							outputProcTime = op_outputProcTime + se2->outputProcTime;
							path_selectivity = op_selectivity*se2->path_selectivity;
							path_capacity = (1/op_selectivity)/((1/(op_capacity*op_selectivity))+(1/se2->path_capacity));
							priority = calculatePriority(path_capacity, path_selectivity, stat);
						}
					}
				}

				ocse->outputProcTime = outputProcTime;
				ocse->path_capacity = path_capacity;
				ocse->path_selectivity = path_selectivity;
				//ocse->oriPriority = ocse->priority = priority;
				ocse->oriPriority = priority;

				if (this->mode == QUOTA_TIME_MODE) {
					//normalize priority by processing time, so that the share of CPU time by this stmt is based on time, not tuple count.
					ocse->priority = priority*path_capacity;
				} else {
					ocse->priority = priority;  
				}

				s->rank.priority = ocse->priority;
				s->rankSet = true;

				return ocse;
			}

			//set quota based on priority, and scale it when necessary
			//scale is tricky for WRR, when it is too big, everything gets processed, anyway
			//when it is too small, difference can not show.
			//also update expectedQMax values for each stmt here, which is the theoritical 
			void OCSProfile::update_quotas() {
				querySchdl::SMLOG(10, "Entering OCSProfile::update_quotas");	
				if (maxPriority == 0) { // && minPriority == 0) {
					//no stats available yet.
					return;
				}

				if (WeightGroupSchedule::AVG_PER_JOB_ROUND == 1) {
					//becomes round robin
					for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); itr++) {
						for (vector<StmtEntry*>::const_iterator itr2 = (*itr)->getEntries()->begin(); itr2 != (*itr)->getEntries()->end(); ++itr2) {
							//(*itr2)->pStmt->quota = WeightGroupSchedule::AVG_PER_JOB_ROUND;
							//(*itr2)->pStmt->quota_left = WeightGroupSchedule::AVG_PER_JOB_ROUND;
							(*itr2)->pStmt->quota = WeightOnlyProfile::DEFAULT_QUOTA;
							(*itr2)->pStmt->quota_left = WeightOnlyProfile::DEFAULT_QUOTA;
							(*itr2)->pStmt->deadline_quota = 0;
							(*itr2)->expectedQMax = 0;
						}
					}
				}

				//if roundTotal is not explicitely passed in, then use (WeightGroupSchedule::AVG_PER_JOB_ROUND)*(NumJobs) as the roundTotal
				if (roundTotal == -1) {
					//first scale down by WeightOnlyProfile::MIN_QUOTA, and scale back later, to make sure the minimum is WeightOnlyProfile::DEFAULT_QUOTA
					roundTotal = (WeightGroupSchedule::AVG_PER_JOB_ROUND/WeightOnlyProfile::MIN_QUOTA) * totalSourceStmt;
				}

				//scale priorities to become quotas, make the total of all source stmts to be roundTotal
				for (vector<WeightGroup*>::iterator itr = vSGroups.begin(); itr != vSGroups.end(); itr++) {
					for (vector<StmtEntry*>::const_iterator itr2 = (*itr)->getEntries()->begin(); itr2 != (*itr)->getEntries()->end(); ++itr2) {
						if (!((*itr2)->pStmt->rankSet)) {
							//no rank yet, stat not available, just use average for now, before gathering enough stats
							(*itr2)->pStmt->quota = WeightGroupSchedule::AVG_PER_JOB_ROUND;
							(*itr2)->pStmt->quota_left = WeightGroupSchedule::AVG_PER_JOB_ROUND;
							(*itr2)->pStmt->deadline_quota = 0;
							(*itr2)->expectedQMax = 0;	  
						} else {
							//stat and priority available.
							OCSEntry* ocse = NULL;
							vector<OCSEntry*>::iterator itr3 = lower_bound(ocsInfo.begin(), ocsInfo.end(), (*itr2)->pStmt->name, ltse());
							if (itr3 != ocsInfo.end() && (*itr3)->stmt_name->compare((*itr2)->pStmt->name) == 0) {
								//found entry
								ocse = *itr3;
							}

							(*itr2)->pStmt->quota =((int)(ceil(((*itr2)->priority*roundTotal)/totalSrcPriority)))*WeightOnlyProfile::MIN_QUOTA;
							(*itr2)->pStmt->quota_left = (*itr2)->pStmt->quota;
							(*itr2)->pStmt->deadline_quota = 0;	  
							//the following is the expected max queue size possible, to be finished within one second
							//expectedQMax = EFFECTIVE_CPU_UTILIZATION*(oriPriority/totalSrcOriPriority)/(1/path_capacity)
							//therefore, for deadline of K seconds, max queue size will be multiplied by K before being used for scheduling decisions
							if (ocse != NULL) {
								(*itr2)->expectedQMax =
									(int)(ceil(((Monitor::EFFECTIVE_CPU_UTILIZATION)*(*itr2)->oriPriority*(ocse->path_capacity))/totalSrcOriPriority));
								if (querySchdl::verbose) {
									cout << "calculate expectedQMax for " << (*itr2)->pStmt->name << ": " << Monitor::EFFECTIVE_CPU_UTILIZATION << "*" << (*itr2)->oriPriority << "*"
										<< ocse->path_capacity << "/" << totalSrcOriPriority << "=" << (*itr2)->expectedQMax << endl;
								}
							}
						}
					}
					if (mode == QUOTA_TIME_MODE) {
						sort((*itr)->getEntries()->begin(), (*itr)->getEntries()->end(), gtSE());
					} else {
						sort((*itr)->getEntries()->begin(), (*itr)->getEntries()->end(), gtOriSE());	
					}
				}
				}

				WeightProfile* WeightOnlyWRRStrategy::createProfile() {
					return new WeightOnlyProfile(this->drv);
				}

				ProfileSchedule* WeightOnlyWRRStrategy::createSchedule() {
					if (this->profile == NULL) this->profile = createProfile();

					ProfileSchedule* schedule = new PriorityProfileSchedule(drv, this->profile, this);

					return schedule;
				}

				WeightGroupSchedule* WeightOnlyWRRStrategy::createWeightGroupSchedule(WeightGroup* wg) {
					return new WRRGroupSchedule(this->drv, this, wg);
				}

				WeightProfile* OCSWRRStrategy::createProfile() {
					return new OCSProfile(this->drv);
				}

				ProfileSchedule* OCSWRRStrategy::createSchedule() {
					if (this->profile == NULL) this->profile = createProfile();

					ProfileSchedule* schedule = new PriorityProfileSchedule(drv, this->profile, this);

					return schedule;
				}

				WeightGroupSchedule* OCSWRRStrategy::createWeightGroupSchedule(WeightGroup* wg) {
					return new WRRGroupSchedule(this->drv, this, wg);
				}

				WeightProfile* OCSWRRDLStrategy::createProfile() {
					return new OCSProfile(this->drv);
				}

				ProfileSchedule* OCSWRRDLStrategy::createSchedule() {
					if (this->profile == NULL) this->profile = createProfile();

					ProfileSchedule* schedule = new PriorityProfileSchedule(drv, this->profile, this);

					return schedule;
				}

				WeightGroupSchedule* OCSWRRDLStrategy::createWeightGroupSchedule(WeightGroup* wg) {
					return new WRRDLGroupSchedule(this->drv, this, wg);
				}

				WeightProfile* OCSPriorityStrategy::createProfile() {
					return new OCSProfile(this->drv, -1, QUOTA_COUNT_MODE);
				}

				ProfileSchedule* OCSPriorityStrategy::createSchedule() {
					if (this->profile == NULL) this->profile = createProfile();

					ProfileSchedule* schedule = new PriorityProfileSchedule(drv, this->profile, this);

					return schedule;
				}

				WeightGroupSchedule* OCSPriorityStrategy::createWeightGroupSchedule(WeightGroup* wg) {
					return new PriorityGroupSchedule(this->drv, this, wg);
				}

				WeightProfile* OCSPriorityDLStrategy::createProfile() {
					return new OCSProfile(this->drv, -1, QUOTA_COUNT_MODE);
				}

				ProfileSchedule* OCSPriorityDLStrategy::createSchedule() {
					if (this->profile == NULL) this->profile = createProfile();

					ProfileSchedule* schedule = new PriorityProfileSchedule(drv, this->profile, this);

					return schedule;
				}

				WeightGroupSchedule* OCSPriorityDLStrategy::createWeightGroupSchedule(WeightGroup* wg) {
					return new PriorityDLGroupSchedule(this->drv, this, wg);
				}

				StrategyFactory* StrategyFactory::_instance = 0;
				StrategyFactory* StrategyFactory::getInstance() {
					if (_instance == 0) {
						_instance = new StrategyFactory();
					}
					return _instance;
				}

				void StrategyFactory::destroy() {
					if (_instance != 0) {
						delete _instance;
						_instance = 0;
					}
				}

				void StrategyFactory::cleanupTrash() {
					//if (querySchdl::verbose) cout << "enter cleanupTrash in StrategyFactory" << endl;

					if (trash.empty()) return;

					//if (querySchdl::verbose) cout << "middle 1" << endl;

					for (vector<ScheduleStrategy*>::iterator itr = trash.begin(); itr != trash.end(); ++itr) {
						//if (querySchdl::verbose) cout << "middle 2" << endl;
						delete (*itr);
					}

					trash.clear();

					//if (querySchdl::verbose) cout << "exit cleanupTrash in StrategyFactory" << endl;
				}

				StrategyFactory::StrategyFactory() {
				}


				StrategyFactory::~StrategyFactory() {
				}

				/**------------------------
				  Everything below are old code for strategies, before instroducing the stride machanism.
				  After which it seems to be more benificial to distinguish weight/priority/quota calculation (such as chain)
				  from the mechanism of actual executing the segments in a certain order (such as stride).
				  ------------------------**/

				Strategy::Strategy(Driver* drv):name("")
				{  
					this->drv = drv;
					drv->setStrategy(this);
				}

				Strategy::~Strategy()
				{  
				}

				void Strategy:: setDriver(Driver* drv) {
					this->drv = drv;
					drv->setStrategy(this);  
					update_all();
				}

				void PriorityStrategy::printStateTable()
				{
					cout << "current strategy has " << stmtsInfo->size() << " stmts, the order of stmts and priority is: " << endl;

					for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
						cout << "(" << itr->second->stmt_name << "," << (0 - itr->first) << ") ";
					}

					cout << endl << endl;

					cout << "priority and rankSet flags in stmts:" << endl;

					for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
						cout << "(" << (*itr)->name << "," << (*itr)->rank.priority << ",rankSet " << ((*itr)->rankSet?"true)":"false)");
					}

					cout << endl << endl;  
				}

				PriorityStrategy::PriorityStrategy(Driver* drv):Strategy(drv)
				{  
					stmtsInfo = new StratInfoType();

					//in constructor, reset all information in statement first.
					for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
						(*itr)->rank.priority = PriorityStrategy::MAX_PRIORITY;
						(*itr)->rankSet = false;
					}
				}

				PriorityStrategy::~PriorityStrategy()
				{  
					for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
						delete itr->second;
					}
					delete stmtsInfo;
				}

				quota_ety::quota_ety(char* name):strat_entry(name),quota(QuotaStrategy::DEFAULT_QUOTA), quota_left(QuotaStrategy::DEFAULT_QUOTA) { }

				quota_ety::quota_ety(char* name, long q):strat_entry(name),quota(q),quota_left(q) { }

				QuotaStrategy::QuotaStrategy(Driver* drv, long quota, bool fixed):Strategy(drv),default_quota(quota),fixed_quota(fixed)
				{
					stmtsInfo = new list<quota_entry*>();

					//in constructor, reset all information in statement first.
					for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
						(*itr)->rank.quota = this->default_quota; //clear old info first, then calculate new one.
						(*itr)->rankSet = false;
					}
				}

				void QuotaStrategy::printStateTable()
				{
					cout << "current strategy has " << stmtsInfo->size() << " stmts, the order of stmts is (format stmt, quota, quota_left): " << endl;

					for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
						cout << "(" << (*itr)->stmt_name << "," << (*itr)->quota << "," << (*itr)->quota_left << ") ";
					}

					cout << endl << endl;

					cout << "quota and rankSet flags in stmts:" << endl;

					for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
						cout << "(" << (*itr)->name << "," << (*itr)->rank.quota << ",rankSet " << ((*itr)->rankSet?"true)":"false)");
					}

					cout << endl << endl;  
				}

				QuotaStrategy::~QuotaStrategy()
				{  
					for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
						delete (*itr);
					}
					delete stmtsInfo;
				}

				SegmentMap::SegmentMap(Driver* drv)
				{  
					//by default, the map is the same as in driver and does not have additional segmentation. Overwritten in children if needed.
					segmentMap = new BufStateTblType();
					BufStateTblType* drvMap = drv->getBufGraph();
					char* name;
					b_entry entry;

					for (BufStateTblType::iterator itr = drvMap->begin(); itr != drvMap->end(); itr++) {
						entry = new buffer_entry(itr->second);
						//always copy over the key, it should not be shared since this is updated later than driver. 
						//if shared, by the time the destructor is called the shared key may already have been deleted.
						name = new char[strlen(itr->first)+1];
						strcpy(name, itr->first);
						(*segmentMap)[name] = entry;
					}
				}

				SegmentMap::~SegmentMap()
				{

					for (BufStateTblType::iterator itr = segmentMap->begin(); itr != segmentMap->end();) {
						char* s = itr->first; 
						delete itr->second;
						itr++;
						delete s;
					}

					delete segmentMap;
				}

				OperatorPriorityStrategy::OperatorPriorityStrategy(Driver* drv):PriorityStrategy(drv)
				{  
				}

				OperatorPriorityStrategy::~OperatorPriorityStrategy()
				{
				}

				void OperatorPriorityStrategy::update_all()
				{  
					if (querySchdl::verbose) {
						cout << "update all priorities in strategy. Currently driver has " << drv->getStmts()->size() << " statements." << endl;
						//cout << "Before update: " << endl;
						//printStateTable();
					}

					//first remove old info table
					for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
						delete itr->second;
					}
					delete stmtsInfo;
					stmtsInfo = new StratInfoType();

					for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
						(*itr)->rank.priority = PriorityStrategy::MAX_PRIORITY;
						(*itr)->rankSet = false;
					}

					//recalculate all priorities and reconstruct table
					for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {    
						//strat_entry* entry = new ocs_strat_entry((*itr)->name);
						//char* name = new char[strlen((*itr)->name)+1];
						//strcpy(name, (*itr)->name);	    	    
						//entry->stmt_name = name;
						//entry->calculate();
						//stmtsInfo->insert(pair<double, strat_entry*>(0 - ((*itr)->priority), entry));
						update_entry(*itr);
					}

					//if (querySchdl::verbose) {
					printStateTable();  
					drv->printStateTable();
					DrvMgr::getInstance()->getMonitor()->printStateTables();
					//}

					return;
				}

				/* not really needed
				//default implementation, always assign PriorityStrategy::MAX_PRIORITY, since specific algorithms know how to calculate priority based on stat
				void OperatorPriorityStrategy::update_entry(stmt* s) {  

				if (s == NULL) {
				return;
				}

				//check if my entry exists, if not, create one.
				//pair<StratInfoType::iterator, StratInfoType::iterator> p = stmtsInfo->equal_range(s->rank.priority);
				bool found = false;
				strat_entry* entry;
				StratInfoType::iterator itr_find;
				//for ( itr_find = p.first; itr_find != p.second && !found; itr_find++) {
				for ( StratInfoType::iterator itr_find = stmtsInfo->begin(); itr_find != stmtsInfo->end() && !found; itr_find++) {
				if(strcmp(itr_find->second->stmt_name, s->name) == 0) {
				found = true;
				entry = (strat_entry*)itr_find->second;
				break;
				}
				}

				if (!found) {
				entry = new strat_entry(s->name);
				s->rank.priority = PriorityStrategy::MAX_PRIORITY;
				stmtsInfo->insert(pair<double, strat_entry*>(0 - (s->rank.priority), entry));    
				}

				return;
				}
				 */


				//modified code that does repeated search from top a lot.
				run_rc OperatorPriorityStrategy::runNextUnit(int* nTuples)
				{  
					//if (querySchdl::verbose) {
					//  cout << "enter operator strategy " << endl;
					//}

					DrvMgr* dm = DrvMgr::getInstance();
					BufStateTblType* bufStateTable = drv->getBufGraph();
					Monitor* monitor = dm->getMonitor();
					bufferMngr* bm = bufferMngr::getInstance();

					bm->new_arrival = 0;    

					int s_rc;

					//loop through all stmts, from high priority to low.
					//make sure at least one tuple is processed before exit the search, back out after every tuple is processed.
					bool first_tuple = true; 
					bool output = false;
					stmt* s;
					bool leave = false;

					//need to run the highest prioritied statement that actually has input. It is not as simple as checking the input buffer, since it could be an fork buffer
					//(currently processing of tuples beyond the first one on input buffer is not supported in statements yet, this may change in the future though. Before that happens, I have to wait on fork)
					for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end() && (!(bm->new_arrival) || first_tuple) && !leave; itr++) {

						if (!(dm->stmtInUse(itr->second->stmt_name))) {
							//since the invalid entries are at the end of the info table, if not removed, there is no need to proceed.
							if (querySchdl::verbose) cout << "reach invalid entries on priority strategy info map."  << endl;
							//return run_no_input; 
							leave = true;
							break;
						}

						s = dm->getStmtByName(itr->second->stmt_name);
						if (s == NULL || !(s->valid)) {
							if (querySchdl::verbose) {
								cout << "get invalid or NULL statement in priority strategy." << endl;
							}
							continue; //this really should not happen either
						}

						stmt_entry* stat = monitor->getStmtStat(s);
						//first see whether this statement needs setting its priority (due to newly available stats)
						//isUpdateTrigger(s) ensures only update if it is the sink statement. Since we may waste a lot of time update non-sink statements, when sinks are not ready.
						if (s->rank.priority == PriorityStrategy::MAX_PRIORITY && isUpdateTrigger(s)) {
							if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
								if (querySchdl::verbose) {
									cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update priority for statement " << s->name << endl;
								}

								//this is not included in regular scheduling overhead.
								struct timeval tv_start;
								struct timezone tz;
								gettimeofday(&tv_start, &tz);

								update_all(); //now I have to leave the loop since underlying data strcture no longer exists.

								struct timeval tv_end;
								gettimeofday(&tv_end, &tz);
								double diff_proc = timeval_subtract(tv_end, tv_start);
								if (monitor->total_processing_time.on) {
									monitor->total_processing_time.value += diff_proc;
								}

								leave = true;
								break;
							}
						}

						//if selectivity or process rate changed significantly (more than some threshold), then update all priorities to reflect latest stat data
						if (stat!= NULL && s->rank.priority != PriorityStrategy::MAX_PRIORITY && change_significant(stat)) {      

							//this is not included in regular scheduling overhead.
							struct timeval tv_start;
							struct timezone tz;
							gettimeofday(&tv_start, &tz);

							update_all(); //now I have to leave the loop since underlying data strcture no longer exists.

							struct timeval tv_end;
							gettimeofday(&tv_end, &tz);
							double diff_proc = timeval_subtract(tv_end, tv_start);
							if (monitor->total_processing_time.on) {
								monitor->total_processing_time.value += diff_proc;
							}      

							if (querySchdl::verbose) {
								cout << "stat changes are above threshold, update all priorities." << endl;
								printStateTable();
							} 
							leave = true;
							break;     
						}

						//move to individual cases, since monitor->exeStmt() will do it again, duplications.
						//monitor->updateLastInTurn(s);

						b_entry entry;
						s_rc = s_success;
						if ((!(bm->new_arrival) || first_tuple) && !leave) {

							//if (querySchdl::verbose) {
							//cout << "about to run stmt " << s->name << ", Tuple remained to process for current round is " << (*nTuples) << endl;
							//}

							switch (s->type) {

								case stmt_normal:
								case stmt_join: 
									//a normal statement may be on a fork, same for the input buffer of the join statement (not the window buffers)
									entry = (*(bufStateTable))[s->in->name];

									//only these two types can be source. Check at source only to avoid artificial increase of delay for downstream operators
									if ((entry->b_type == t_source || entry->b_type == t_source_fork) && (*nTuples) <= 0) {
										if (querySchdl::verbose) cout << "finished one driver round." << endl;
										leave = true;
										break;
										//return run_success;
									}

									if (entry->b_type == t_fork || entry->b_type == t_source_fork) {
										//find the dummy entry for my branch, in the mean time check to see if all dummies are processed for this round. If so, reset flag for all.
										list<b_entry> dummies;
										bool finished = true;
										for (BufStateTblType::iterator itr2 = bufStateTable->begin(); itr2 != bufStateTable->end(); itr2++) {
											if (itr2->second->b_type == t_fork_dummy) {
												if (strcmp(itr2->second->inplace, s->in->name) == 0) {
													dummies.push_back(itr2->second);

													if (!(itr2->second->back.processed)) finished = false;

													if (strcmp(itr2->second->statement->name, s->name) == 0) {
														entry = itr2->second;
													}
												}
											}    
										}

										//if all branches are finished, reset all flags, and pop the tuple on this buffer
										if (finished) {
											for (list<b_entry>::iterator itr2 = dummies.begin(); itr2 != dummies.end(); itr2++) {
												(*itr2)->back.processed = false;
											}
											s->in->pop();
											if ((*(bufStateTable))[s->in->name]->b_type == t_source_fork) (*nTuples)--;
										}

										if (entry->back.processed || s->in->empty()) {
											if (s->in->empty() && monitor->total_empty_buffer_hit.on) {
												monitor->total_empty_buffer_hit.value++;
											}
											monitor->updateLastInTurn(s);
											//if strategy is changed, this flag is set, return immediately
											if (this->drv->stateTableChanged) {
												//this->drv->stateTableChanged = false;
												if (querySchdl::verbose) cout << "strategy Changed 1" << endl;
												return run_success; //leave immediately after underlying table change.
											}

											s_rc = s_no_input;
											continue;
										} else {
											s_rc = (int)(monitor->exeStmt(s, drv));
											if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
												//I know I am not union statement here, just pop myself as a dummy
												entry->pop();
												first_tuple = false;
												if (s_rc > 0) output = true;
												//proceed if I am still in reigions of stat not available (which always has the highest priorities).
												//Only jump out if I have entered stmts with stats
												if (s->rank.priority != PriorityStrategy::MAX_PRIORITY) {
													leave = true;
													break;
												}	      
											} else if(s_rc == s_no_input) {
												//this happens when it is a join. For normal case I already checked input buffer to be non-empty
												continue;
											}
										}
									} else {
										//I am a normal statement, input is not on a fork. Just check buffer, and run if not empty
										if (s->in->empty()) {
											if (monitor->total_empty_buffer_hit.on) {
												monitor->total_empty_buffer_hit.value++;
											}

											monitor->updateLastInTurn(s);
											//if strategy is changed, this flag is set, return immediately
											if (this->drv->stateTableChanged) {
												//this->drv->stateTableChanged = false;	      
												if (querySchdl::verbose) cout << "strategy Changed 2" << endl;
												return run_success; //leave immediately after underlying table change.
											}

											s_rc = s_no_input;
											continue;
										}
										else {
											s_rc = (int)(monitor->exeStmt(s, drv));
											if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
												//I know I am not union statement here, just pop input
												s->in->pop();
												first_tuple = false;
												if (s_rc > 0) output = true;

												if ((*(bufStateTable))[s->in->name]->b_type == t_source) (*nTuples)--;

												//proceed if I am still in reigions of stat not available (which always has the highest priorities).
												//Only jump out if I have entered stmts with stats
												if (s->rank.priority != PriorityStrategy::MAX_PRIORITY) {	      
													//search from top again.
													leave = true;
													break;
												}	      
											} else if(s_rc == s_no_input) {
												//this should not have happened, since this is a normal statement and I checked input buffer
												continue;
											}
										}
									}	
									break;

								case stmt_t_union: 
								case stmt_tl_union:
									//let the stmt check whether input buffer is empty is actually cheaper in the union case, than to do it here.
									s_rc = (int)(monitor->exeStmt(s, drv));
									if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
										//"in" will be set to the buffer to pop
										//in our current implementation of union, the input buffers are not possible to be on forks.
										s->in->pop();
										first_tuple = false;
										if (s_rc > 0) output = true;

										if ((*(bufStateTable))[s->in->name]->b_type == t_source) (*nTuples)--; //this is auctually not necessary, since it can not be source in our current implementation.

										//proceed if I am still in reigions of stat not available (which always has the highest priorities).
										//Only jump out if I have entered stmts with stats
										if (s->rank.priority != PriorityStrategy::MAX_PRIORITY) {	    
											//search from top again.
											leave = true;
											break;
										}

										//in our current implementation of union, the input buffers are not possible to be on forks.
										//otherwise, the following block of code would have been necessary to check if the "in" is set to a fork buffer
										// for pop of fork on unions
										//pop myself, unless the following buffer is a union. If union, I then need to look up the correct one to pop
										//if (s->type == stmt_t_union || s->type == stmt_tl_union || s->type == stmt_join) {
										//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
										//buffer* b = s->in;
										//if the other "in" is not a fork buffer, we directly pop it.
										//If it is a fork buffer, then we need to know which dummy to record a pop.
										//if (((*bufStateTable)[b->name])->b_type != t_fork
										//&& ((*bufStateTable)[b->name])->b_type != t_source_fork && ((*bufStateTable)[b->name])->b_type != t_merge_fork) {
										//b->pop();
										//if (((*bufStateTable)[b->name])->b_type == t_source) (*nTuples)--;

										//_dg("poped");
										// _dg(b->name);
										// if (b->empty()) _dg("empty"); else _dg("not empty");
										//} else {
										//The buffer I need to pop is a fork.
										//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
										//  for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
										//if (itr->second->b_type != t_fork_dummy) continue;
										//if (strcmp(itr->second->inplace, b->name) == 0) {
										// if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
										//    (itr->second->pop());
										//  }
										//}    
										//  }
										//}
										//} else {
										//just pop myself as a dummy
										//state->pop();
										//}
									} else if(s_rc == s_no_input) {
										continue;
									}	
									break;

								default:
									continue;
							} //end switch (s->type)      
						} //end if

						//if (querySchdl::verbose && output) {
						//there are output tuples, print debug info for monitor stats
						//cout << "new output tuple, print stats and priorities during the stmt walking" << endl;
						//monitor->printStateTables();
						//printStateTable();
						//}

					} //end for

					if (bm->new_arrival && querySchdl::verbose) {
						cout << "new arrival, return back to front of queue" << endl;
					}

					if (querySchdl::verbose && output) {
						//there are output tuples, print debug info for monitor stats
						//cout << "new output tuple, print stats and priorities after finishing stmt walking" << endl;
						//monitor->printStateTables();
						//printStateTable();
					}

					if(s_rc == s_no_input) {
						return run_no_input;
					} else {
						return run_success;
					}
				}


				/*
				// code that has the most twist, to go back out after output
				run_rc OperatorPriorityStrategy::runNextUnit(int* nTuples)
				{  
				//if (querySchdl::verbose) {
				//  cout << "enter operator strategy " << endl;
				//}

				DrvMgr* dm = DrvMgr::getInstance();
				BufStateTblType* bufStateTable = drv->getBufGraph();
				Monitor* monitor = dm->getMonitor();
				bufferMngr* bm = bufferMngr::getInstance();

				bm->new_arrival = 0;    

				int s_rc;

				//loop through all stmts, from high priority to low.
				//used to make sure at least one tuple is processed before backed out, to avoid wasting too much seaching time
				//since we do not know whether the new arrival is surely on higher priority statements
				bool first_tuple = true; 
				bool output = false;
				stmt* s;
				bool leave = false;

				//int tmpcnt = 0;

				for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end() && ((itr->first == (-PriorityStrategy::MAX_PRIORITY)) || ((!(bm->new_arrival) || first_tuple) && !output)) && !leave; itr++) {

				if (!(dm->stmtInUse(itr->second->stmt_name))) {
				//since the invalid entries are at the end of the info table, if not removed, there is no need to proceed.
				if (querySchdl::verbose) cout << "reach invalid entries on priority strategy info map."  << endl;
				//return run_no_input; 
				leave = true;
				break;
				}

				//need to run the highest prioritied statement that actually has input. It is not as simple as checking the input buffer, since it could be an fork buffer
				//(currently processing of tuples beyond the first one on input buffer is not supported in statements yet, this may change in the future though. Before that happens, I have to wait on fork)
				s = dm->getStmtByName(itr->second->stmt_name);
				if (s == NULL || !(s->valid)) {
				if (querySchdl::verbose) {
				cout << "get invalid or NULL statement in priority strategy." << endl;
				}
				continue; //this really should not happen either
				}


				stmt_entry* stat = monitor->getStmtStat(s);
				//first see whether this statement needs setting its priority (due to newly available stats)
				//isUpdateTrigger(s) ensures only update if it is the sink statement. Since we may waste a lot of time update non-sink statements, when sinks are not ready.
				if (s->rank.priority == PriorityStrategy::MAX_PRIORITY && isUpdateTrigger(s)) {
				if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
				if (querySchdl::verbose) {
				cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update priority for statement " << s->name << endl;
				}
				update_all(); //now I have to leave the loop since underlying data strcture no longer exists.
				leave = true;
				break;
				}
				}

				//if selectivity or process rate changed significantly (more than some threshold), then update all priorities to reflect latest stat data
				if (stat!= NULL && s->rank.priority != PriorityStrategy::MAX_PRIORITY && change_significant(stat)) {      
				update_all(); //now I have to leave the loop since underlying data strcture no longer exists.
				if (querySchdl::verbose) {
				cout << "stat changes are above threshold, update all priorities." << endl;
				printStateTable();
				} 
				leave = true;
				break;     
				}

				//monitor->updateLastInTurn(s);

				b_entry entry;
				s_rc = s_success;
				bool first_for_max = true;
				first_tuple = true;
				while (s_rc > 0 && 
						(((!(bm->new_arrival) || first_tuple) && !leave && !output && itr->first != (-PriorityStrategy::MAX_PRIORITY)) 
						 || (itr->first == (-PriorityStrategy::MAX_PRIORITY) && first_for_max))) {

					//tmpcnt++;

					//if (querySchdl::verbose) {
					//cout << "about to run stmt " << s->name << ", Tuple remained to process for current round is " << (*nTuples) << endl;
					//}

					switch (s->type) {

						case stmt_normal:
						case stmt_join: 
							//a normal statement may be on a fork, same for the input buffer of the join statement (not the window buffers)
							entry = (*(bufStateTable))[s->in->name];

							//only these two types can be source. Check at source only to avoid artificial increase of delay for downstream operators
							if ((entry->b_type == t_source || entry->b_type == t_source_fork) && (*nTuples) <= 0) {
								if (querySchdl::verbose) cout << "finished one driver round." << endl;
								leave = true;
								break;
								//return run_success;
							}

							if (entry->b_type == t_fork || entry->b_type == t_source_fork) {
								//find the dummy entry for my branch, in the mean time check to see if all dummies are processed for this round. If so, reset flag for all.
								list<b_entry> dummies;
								bool finished = true;
								for (BufStateTblType::iterator itr2 = bufStateTable->begin(); itr2 != bufStateTable->end(); itr2++) {
									if (itr2->second->b_type == t_fork_dummy) {
										if (strcmp(itr2->second->inplace, s->in->name) == 0) {
											dummies.push_back(itr2->second);

											if (!(itr2->second->back.processed)) finished = false;

											if (strcmp(itr2->second->statement->name, s->name) == 0) {
												entry = itr2->second;
											}
										}
									}    
								}

								//if all branches are finished, reset all flags, and pop the tuple on this buffer
								if (finished) {
									for (list<b_entry>::iterator itr2 = dummies.begin(); itr2 != dummies.end(); itr2++) {
										(*itr2)->back.processed = false;
									}
									s->in->pop();
									if ((*(bufStateTable))[s->in->name]->b_type == t_source_fork) (*nTuples)--;
								}

								if (entry->back.processed || s->in->empty()) {
									if (s->in->empty() && monitor->total_empty_buffer_hit.on) {
										monitor->total_empty_buffer_hit.value++;
									}

									monitor->updateLastInTurn(s);
									//if strategy is changed, this flag is set, return immediately
									if (this->drv->stateTableChanged) {
										if (querySchdl::verbose) cout << "strategy Changed 3" << endl;
										return run_success; //leave immediately after underlying table change.
									}

									s_rc = s_no_input;
									continue;
								} else {
									s_rc = (int)(monitor->exeStmt(s, drv));
									if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
										//I know I am not union statement here, just pop myself as a dummy
										entry->pop();
										first_tuple = false;
										first_for_max = false;
										if (s_rc > 0) {
											output = true;
										}

										if (!(bm->new_arrival)) {
											//no arrival, do myself again or next one down the list
											continue;
										} else {
											//new arrival, search from top again, but only after I finished the stat-not-available stmts.
											if (itr->first != (-PriorityStrategy::MAX_PRIORITY)) {
												leave = true;
												break;
											}
											//return run_success;
										}
										//return run_success;
									} else if(s_rc == s_no_input) {
										//this happens when it is a join. For normal case I already checked input buffer to be non-empty
										continue;
									}
								}
							} else {
								//I am a normal statement, input is not on a fork. Just check buffer, and run if not empty
								if (s->in->empty()) {
									if (monitor->total_empty_buffer_hit.on) {
										monitor->total_empty_buffer_hit.value++;
									}

									monitor->updateLastInTurn(s);
									//if strategy is changed, this flag is set, return immediately
									if (this->drv->stateTableChanged) {
										if (querySchdl::verbose) cout << "strategy Changed 4" << endl;
										return run_success; //leave immediately after underlying table change.
									}

									s_rc = s_no_input;
									continue;
								}
								else {
									s_rc = (int)(monitor->exeStmt(s, drv));
									if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
										//I know I am not union statement here, just pop input
										s->in->pop();
										first_tuple = false;
										first_for_max = false;
										if (s_rc > 0) {
											output = true;
										}

										if ((*(bufStateTable))[s->in->name]->b_type == t_source) (*nTuples)--;

										if (!(bm->new_arrival)) {
											//no arrival, do myself again or next one down the list
											continue;
										} else {
											//new arrival, search from top again, but only after I finished the stat-not-available stmts.
											if (itr->first != (-PriorityStrategy::MAX_PRIORITY)) {
												leave = true;
												break;
											}
											//return run_success;
										}

										//return run_success;
									} else if(s_rc == s_no_input) {
										//this should not have happened, since this is a normal statement and I checked input buffer
										continue;
									}
								}
							}

							break;

						case stmt_t_union: 
						case stmt_tl_union:
							//let the stmt check whether input buffer is empty is actually cheaper in the union case, than to do it here.
							s_rc = (int)(monitor->exeStmt(s, drv));
							if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
								//"in" will be set to the buffer to pop
								//in our current implementation of union, the input buffers are not possible to be on forks.
								s->in->pop();
								first_tuple = false;
								first_for_max = false;
								if (s_rc > 0) {
									output = true;
								}
								if ((*(bufStateTable))[s->in->name]->b_type == t_source) (*nTuples)--; //this is auctually not necessary, since it can not be source in our current implementation.

								if (!(bm->new_arrival)) {
									//no arrival, run myself again or next one down the list
									continue;
								} else {
									//new arrival, search from top again, but only after I finished the stat-not-available stmts.
									if (itr->first != (-PriorityStrategy::MAX_PRIORITY)) {
										leave = true;
										break;
									}
									//return run_success;
								}
								//return run_success;

								//in our current implementation of union, the input buffers are not possible to be on forks.
								//otherwise, the following block of code would have been necessary to check if the "in" is set to a fork buffer
								// for pop of fork on unions
								//pop myself, unless the following buffer is a union. If union, I then need to look up the correct one to pop
								//if (s->type == stmt_t_union || s->type == stmt_tl_union || s->type == stmt_join) {
								//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
								//buffer* b = s->in;
								//if the other "in" is not a fork buffer, we directly pop it.
								//If it is a fork buffer, then we need to know which dummy to record a pop.
								//if (((*bufStateTable)[b->name])->b_type != t_fork
								//&& ((*bufStateTable)[b->name])->b_type != t_source_fork && ((*bufStateTable)[b->name])->b_type != t_merge_fork) {
								//b->pop();
								//if (((*bufStateTable)[b->name])->b_type == t_source) (*nTuples)--;

								//_dg("poped");
								// _dg(b->name);
								//if (b->empty()) _dg("empty"); else _dg("not empty");
								//} else {
								//The buffer I need to pop is a fork.
								//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
								//for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
								//if (itr->second->b_type != t_fork_dummy) continue;
								//if (strcmp(itr->second->inplace, b->name) == 0) {
								//  if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
								//    (itr->second->pop());
								//  }
								//}    
								//}
								//}
								//} else {
								//just pop myself as a dummy
								//state->pop();
								//}
							} else if(s_rc == s_no_input) {
								continue;
							}

							break;

						default:
							continue;
					} //end switch (s->type)  

				} //end while

				//if (querySchdl::verbose && output) {
				//there are output tuples, print debug info for monitor stats
				//cout << "new output tuple, print stats and priorities during the stmt walking" << endl;
				//monitor->printStateTables();
				//printStateTable();
				//}

				//if (querySchdl::verbose && output) {
				//cout << "tmpcnt is " << tmpcnt << ", priority is " << itr->first << ", equal is " << (itr->first == (-PriorityStrategy::MAX_PRIORITY) ? "true" : "false") << ", stmt is " << itr->second->stmt_name << endl;
				//}

		} //end for

		if (bm->new_arrival && querySchdl::verbose) {
			cout << "new arrival, return back to front of queue" << endl;
		}

		if (querySchdl::verbose && output) {
			//there are output tuples, print debug info for monitor stats
			//cout << "new output tuple, print stats and priorities after finishing stmt walking" << endl;
			//monitor->printStateTables();
			//printStateTable();
		}

		if (querySchdl::verbose && leave) cout << "leave is true" << endl;

		if(s_rc == s_no_input) {
			return run_no_input;
		} else {
			return run_success;
		}
		}
		*/

			/*
			//original code that does not go back on output
			run_rc OperatorPriorityStrategy::runNextUnit(int* nTuples)
			{  
			//if (querySchdl::verbose) {
			//  cout << "enter operator strategy " << endl;
			//}

			DrvMgr* dm = DrvMgr::getInstance();
			BufStateTblType* bufStateTable = drv->getBufGraph();
			Monitor* monitor = dm->getMonitor();
			bufferMngr* bm = bufferMngr::getInstance();

			bm->new_arrival = 0;    

			int s_rc;

			//loop through all stmts, from high priority to low.
			//used to make sure at least one tuple is processed before backed out, to avoid wasting too much seaching time
			//since we do not know whether the new arrival is surely on higher priority statements
			bool first_tuple = true; 
			bool output = false;
			stmt* s;
			bool leave = false;

			for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end() && (!(bm->new_arrival) || first_tuple) && !leave; itr++) {

			if (!(dm->stmtInUse(itr->second->stmt_name))) {
			//since the invalid entries are at the end of the info table, if not removed, there is no need to proceed.
			if (querySchdl::verbose) cout << "reach invalid entries on priority strategy info map."  << endl;
			//return run_no_input; 
			leave = true;
			break;
			}

			//need to run the highest prioritied statement that actually has input. It is not as simple as checking the input buffer, since it could be an fork buffer
			//(currently processing of tuples beyond the first one on input buffer is not supported in statements yet, this may change in the future though. Before that happens, I have to wait on fork)
			s = dm->getStmtByName(itr->second->stmt_name);
			if (s == NULL || !(s->valid)) {
			if (querySchdl::verbose) {
			cout << "get invalid or NULL statement in priority strategy." << endl;
			}
			continue; //this really should not happen either
			}


			stmt_entry* stat = monitor->getStmtStat(s);
			//first see whether this statement needs setting its priority (due to newly available stats)
			//isUpdateTrigger(s) ensures only update if it is the sink statement. Since we may waste a lot of time update non-sink statements, when sinks are not ready.
			if (s->rank.priority == PriorityStrategy::MAX_PRIORITY && isUpdateTrigger(s)) {
			if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
			if (querySchdl::verbose) {
			cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update priority for statement " << s->name << endl;
			}
			update_all(); //now I have to leave the loop since underlying data strcture no longer exists.
			leave = true;
			break;
			}
			}

			//if selectivity or process rate changed significantly (more than some threshold), then update all priorities to reflect latest stat data
			if (stat!= NULL && s->rank.priority != PriorityStrategy::MAX_PRIORITY && change_significant(stat)) {      
			update_all(); //now I have to leave the loop since underlying data strcture no longer exists.
			if (querySchdl::verbose) {
			cout << "stat changes are above threshold, update all priorities." << endl;
			printStateTable();
			} 
			leave = true;
			break;     
			}

			//monitor->updateLastInTurn(s);

			b_entry entry;
		s_rc = s_success;
		while (s_rc > 0 && (!(bm->new_arrival) || first_tuple) && !leave) {

			//if (querySchdl::verbose) {
			//cout << "about to run stmt " << s->name << ", Tuple remained to process for current round is " << (*nTuples) << endl;
			//}

			switch (s->type) {

				case stmt_normal:
				case stmt_join: 
					//a normal statement may be on a fork, same for the input buffer of the join statement (not the window buffers)
					entry = (*(bufStateTable))[s->in->name];

					//only these two types can be source. Check at source only to avoid artificial increase of delay for downstream operators
					if ((entry->b_type == t_source || entry->b_type == t_source_fork) && (*nTuples) <= 0) {
						if (querySchdl::verbose) cout << "finished one driver round." << endl;
						leave = true;
						break;
						//return run_success;
					}

					if (entry->b_type == t_fork || entry->b_type == t_source_fork) {
						//find the dummy entry for my branch, in the mean time check to see if all dummies are processed for this round. If so, reset flag for all.
						list<b_entry> dummies;
						bool finished = true;
						for (BufStateTblType::iterator itr2 = bufStateTable->begin(); itr2 != bufStateTable->end(); itr2++) {
							if (itr2->second->b_type == t_fork_dummy) {
								if (strcmp(itr2->second->inplace, s->in->name) == 0) {
									dummies.push_back(itr2->second);

									if (!(itr2->second->back.processed)) finished = false;

									if (strcmp(itr2->second->statement->name, s->name) == 0) {
										entry = itr2->second;
									}
								}
							}    
						}

						//if all branches are finished, reset all flags, and pop the tuple on this buffer
						if (finished) {
							for (list<b_entry>::iterator itr2 = dummies.begin(); itr2 != dummies.end(); itr2++) {
								(*itr2)->back.processed = false;
							}
							s->in->pop();
							if ((*(bufStateTable))[s->in->name]->b_type == t_source_fork) (*nTuples)--;
						}

						if (entry->back.processed || s->in->empty()) {
							if (s->in->empty() && monitor->total_empty_buffer_hit.on) {
								monitor->total_empty_buffer_hit.value++;
							}

							monitor->updateLastInTurn(s);
							//if strategy is changed, this flag is set, return immediately
							if (this->drv->stateTableChanged) {
								if (querySchdl::verbose) cout << "strategy Changed 5" << endl;
								return run_success; //leave immediately after underlying table change.
							}

							s_rc = s_no_input;
							continue;
						} else {
							s_rc = (int)(monitor->exeStmt(s, drv));
							if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
								//I know I am not union statement here, just pop myself as a dummy
								entry->pop();
								first_tuple = false;
								if (s_rc > 0) output = true;

								if (!(bm->new_arrival)) {
									//no arrival, do myself again or next one down the list
									continue;
								} else {
									//new arrival, search from top again.
									leave = true;
									break;
									//return run_success;
								}
								//return run_success;
							} else if(s_rc == s_no_input) {
								//this happens when it is a join. For normal case I already checked input buffer to be non-empty
								continue;
							}
						}
					} else {
						//I am a normal statement, input is not on a fork. Just check buffer, and run if not empty
						if (s->in->empty()) {
							if (monitor->total_empty_buffer_hit.on) {
								monitor->total_empty_buffer_hit.value++;
							}

							monitor->updateLastInTurn(s);
							//if strategy is changed, this flag is set, return immediately
							if (this->drv->stateTableChanged) {
								if (querySchdl::verbose) cout << "strategy Changed 5" << endl;
								return run_success; //leave immediately after underlying table change.
							}

							s_rc = s_no_input;
							continue;
						}
						else {
							s_rc = (int)(monitor->exeStmt(s, drv));
							if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
								//I know I am not union statement here, just pop input
								s->in->pop();
								first_tuple = false;
								if (s_rc > 0) output = true;

								if ((*(bufStateTable))[s->in->name]->b_type == t_source) (*nTuples)--;

								if (!(bm->new_arrival)) {
									//no arrival, do myself again or next one down the list
									continue;
								} else {
									//new arrival, search from top again.
									leave = true;
									break;
									//return run_success;
								}

								//return run_success;
							} else if(s_rc == s_no_input) {
								//this should not have happened, since this is a normal statement and I checked input buffer
								continue;
							}
						}
					}

					break;

				case stmt_t_union: 
				case stmt_tl_union:
					//let the stmt check whether input buffer is empty is actually cheaper in the union case, than to do it here.
					s_rc = (int)(monitor->exeStmt(s, drv));
					if (s_rc > 0 || s_rc == s_no_output || s_rc == s_failure) {
						//"in" will be set to the buffer to pop
						//in our current implementation of union, the input buffers are not possible to be on forks.
						s->in->pop();
						first_tuple = false;
						if (s_rc > 0) output = true;

						if ((*(bufStateTable))[s->in->name]->b_type == t_source) (*nTuples)--; //this is auctually not necessary, since it can not be source in our current implementation.

						if (!(bm->new_arrival)) {
							//no arrival, run myself again or next one down the list
							continue;
						} else {
							//new arrival, search from top again.
							leave = true;
							break;
							//return run_success;
						}
						//return run_success;

						//in our current implementation of union, the input buffers are not possible to be on forks.
						//otherwise, the following block of code would have been necessary to check if the "in" is set to a fork buffer
						// for pop of fork on unions
						//pop myself, unless the following buffer is a union. If union, I then need to look up the correct one to pop
						//if (s->type == stmt_t_union || s->type == stmt_tl_union || s->type == stmt_join) {
						//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
						//buffer* b = s->in;
						//if the other "in" is not a fork buffer, we directly pop it.
						//If it is a fork buffer, then we need to know which dummy to record a pop.
						//if (((*bufStateTable)[b->name])->b_type != t_fork
						//&& ((*bufStateTable)[b->name])->b_type != t_source_fork && ((*bufStateTable)[b->name])->b_type != t_merge_fork) {
						//b->pop();
						//if (((*bufStateTable)[b->name])->b_type == t_source) (*nTuples)--;

						//_dg("poped");
						// _dg(b->name);
						//if (b->empty()) _dg("empty"); else _dg("not empty");
						//} else {
						//The buffer I need to pop is a fork.
						//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
						//for (itr = bufStateTable->begin(); itr != bufStateTable->end(); itr++) {
						//if (itr->second->b_type != t_fork_dummy) continue;
						//if (strcmp(itr->second->inplace, b->name) == 0) {
						//  if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
						//    (itr->second->pop());
						//  }
						//}    
						//}
						//}
						//} else {
						//just pop myself as a dummy
						//state->pop();
						//}
					} else if(s_rc == s_no_input) {
						continue;
					}

					break;

				default:
					continue;
			} //end switch (s->type)  

			//exit to search from top again, when there is output
			if (output) {
				leave = true;
				break;
			}
		} //end while

		//if (querySchdl::verbose && output) {
		//there are output tuples, print debug info for monitor stats
		//cout << "new output tuple, print stats and priorities during the stmt walking" << endl;
		//monitor->printStateTables();
		//printStateTable();
		//}

		} //end for

		if (bm->new_arrival && querySchdl::verbose) {
			cout << "new arrival, return back to front of queue" << endl;
		}

		if (querySchdl::verbose && output) {
			//there are output tuples, print debug info for monitor stats
			//cout << "new output tuple, print stats and priorities after finishing stmt walking" << endl;
			//monitor->printStateTables();
			//printStateTable();
		}

		if(s_rc == s_no_input) {
			return run_no_input;
		} else {
			return run_success;
		}
		}
		*/

			SegmentPriorityStrategy::SegmentPriorityStrategy(Driver* drv):PriorityStrategy(drv), SegmentMap(drv)
			{  
			}

		SegmentPriorityStrategy::~SegmentPriorityStrategy()
		{
		}

		void SegmentPriorityStrategy::update_all()
		{  
			return;
		}

		run_rc SegmentPriorityStrategy::runNextUnit(int* nTuples)
		{  
			return run_success;
		}

		SegmentQuotaStrategy::SegmentQuotaStrategy(Driver* drv, long quota, bool fixed):QuotaStrategy(drv, quota, fixed),SegmentMap(drv),last_itr(NULL)/*,stateTableChanged(false)*/
		{
			extraInfo = new ExtraInfoMapType();  
		}

		SegmentQuotaStrategy::~SegmentQuotaStrategy()
		{
			//do nothing here for segmentMap and stmtsInfo, since all entries are removed in parent's descructor
			for (ExtraInfoMapType::iterator itr = extraInfo->begin(); itr != extraInfo->end();) {
				char* s = itr->first; 
				delete itr->second;
				itr++;
				delete s;
				//delete itr->first;  //can not do this since hash_map is not fail-safe
				//delete itr->second;
			}
			delete extraInfo;
		}

		//keep it simple for default behavior, just remove everything (both segmentMap and stmtsInfo) and rebuild based on driver info. 
		//Extra segmentation is not handled, currently it will be all lost. to be added later.
		//all quotas need to be re-calculated in child classes, if they are not fixed in the algorithm.
		void SegmentQuotaStrategy::update_all()
		{  
			DrvMgr* dm = DrvMgr::getInstance();
			Monitor* monitor = dm->getMonitor();
			if (querySchdl::verbose) {
				cout << "update all segments in Segment Quota strategy. Currently driver has " << drv->getStmts()->size() << " statements." << endl;
				cout << "current monitor data " << endl;
				monitor->printStateTables();
			}

			for (BufStateTblType::iterator itr = segmentMap->begin(); itr != segmentMap->end();) {
				char* s = itr->first; 
				delete itr->second;
				itr++;
				delete s;
			}  
			delete segmentMap;

			segmentMap = new BufStateTblType();
			BufStateTblType* drvMap = drv->getBufGraph();
			char* name;
			b_entry entry;

			for (BufStateTblType::iterator itr = drvMap->begin(); itr != drvMap->end(); itr++) {
				if (itr->second->statement != NULL) {
					itr->second->statement->rank.quota = this->default_quota; //clear old info first, before calculate new one later
					itr->second->statement->rankSet = false;
				}

				entry = new buffer_entry(itr->second);
				//always copy over the key, it should not be shared since this is updated later than driver. 
				//if shared, by the time the destructor is called the shared key may already have been deleted.
				name = new char[strlen(itr->first)+1];
				strcpy(name, itr->first);
				(*segmentMap)[name] = entry;
			}

			for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
				delete (*itr);
			}
			delete stmtsInfo;

			stmtsInfo = new list<quota_entry*>();

			for (ExtraInfoMapType::iterator itr = extraInfo->begin(); itr != extraInfo->end();) {

				char* s = itr->first;
				delete itr->second;
				itr++;
				delete s;
				//delete itr->first;  //can not do this since hash_map is not fail-safe
				//delete itr->second;
			}
			delete extraInfo;

			extraInfo = new ExtraInfoMapType();

			//initialize the stmtsInfo list with segments information.
			//each stmt with some source buffer (later will include those become source buffer after segmentation) has an entry. 
			//execution will be bush-depth-first on each stmt, round robin on stmts, with quota
			for (BufStateTblType::iterator itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
				if (itr->second->b_type == t_source || itr->second->b_type == t_source_fork) {
					//if(find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(itr->second->statement->name)) == stmtsInfo->end()) {
					//this statement has source buffer and is not in the list yet
					//quota_entry* entry = new quota_entry(itr->second->statement->name, default_quota);
					//stmtsInfo->push_back(entry);
					update_entry(itr->second->statement);
					//}
				}
			}

			//if (querySchdl::verbose) {
			//  printStateTable();  
			//}

			this->last_itr = stmtsInfo->begin(); //container modified, old value no longer valid.

			this->drv->stateTableChanged = true;

			return;
		}

		//default implementation, always use default_quota. since only specific algorithms know how to calculate quota based on stats.
		//so here, just create a new entry if it did not exist, otherwise do nothing, since we never need to update quota
		//this default implementation does not utilize extraInfo at all.
		void SegmentQuotaStrategy::update_entry(stmt* s) {  
			if (s == NULL) {
				if (querySchdl::verbose) {
					cout << "try to update_entry on NULL stmt in Segment Quota strategy." << endl;
				}
				return;
			}

			if(find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(s->name)) == stmtsInfo->end()) {
				quota_entry* entry = new quota_entry(s->name, this->default_quota);
				stmtsInfo->push_back(entry);
				s->rank.quota = this->default_quota;
				//for this default implementation, this is enough to call it rankSet
				s->rankSet = true;
			}

			return;
		}

		//default implementation
		//execution will be round robin on each source buffer (in topology maintained here, which may have extra break points, besides the original source buffers)  with quota.
		//on each source buffer it will be bushy-depth-first, with backtrack.
		//different segment quota strategies may try to rewrite this method, but usually not the runState() method which does majority of the graph execution.
		run_rc SegmentQuotaStrategy::runNextUnit(int* nTuples)
		{  
			//if (querySchdl::verbose) {
			//cout << "enter run segment quota strategy " << endl;
			//}

			DrvMgr* dm = DrvMgr::getInstance();
			Monitor* monitor = dm->getMonitor();

			bool output = false;
			//bool input = false;
			run_rc rc = run_success;
			stmt* s;
			list<quota_entry*>::iterator itr;
			if (&(this->last_itr) != NULL) itr= this->last_itr;
			else itr = stmtsInfo->begin();

			this->drv->stateTableChanged = false;
			for (; itr != stmtsInfo->end() && (*nTuples) > 0; itr++) {
				if (!(dm->stmtInUse((*itr)->stmt_name))) {
					//precaution, in case invalid statements has not been removed from driver.
					//this actually should not happen.
					if (querySchdl::verbose) cout << "reach invalid entries on segment quota strategy stmt list."  << endl;
					continue;
				}

				s = dm->getStmtByName((*itr)->stmt_name);
				if (s == NULL || !(s->valid)) {
					if (querySchdl::verbose) {
						cout << "get invalid or NULL statement in segment quota strategy." << endl;
					}
					continue; //this really should not happen either
				}

				/* move to run_state, since in segment as unit, only sources called here.
				//first see whether this statement needs updating its quota, for now ignore quota changes based on feedback. This only takes care of changes due to initial stat availability
				if (s->rank.quota == this->default_quota && !s->rankSet && !this->fixed_quota && isUpdateTrigger(s)) {
				stmt_entry* stat = monitor->getStmtStat(s);
				if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
				if (querySchdl::verbose) {
				cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update quota for statement " << s->name << endl;
				}
				update_all();
				this->last_itr = stmtsInfo->begin();
				return run_success; //need to return since underlying list<> changed
				}
				}
				 */

				rc = run_success;
				while ((*nTuples) > 0 && (*itr)->quota_left > 0 && rc == run_success) {
					//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
					rc = runState((*segmentMap)[s->in->name], nTuples, &output);
					//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
					if (this->drv->stateTableChanged) {
						//break; //leave immediately after underlying table change.
						//this->drv->stateTableChanged = false;
						return run_success;
					}
				}

				if (this->drv->stateTableChanged) {
					//this->drv->stateTableChanged = false;
					//break; //leave immediately after underlying table change.
					return run_success;
				}

				if ((*nTuples) > 0) (*itr)->quota_left = 0;

				this->last_itr = itr;

				if (/*(*itr)->quota_left <= 0 &&*/ (*nTuples) > 0 && strcmp(stmtsInfo->back()->stmt_name, s->name) == 0) {
					//just finished the last one in the full round, it is time to reset quota_left for everyone.
					for (list<quota_entry*>::iterator itr2 = stmtsInfo->begin(); itr2 != stmtsInfo->end(); itr2++) {
						(*itr2)->quota_left = (*itr2)->quota;
					}
					this->last_itr = stmtsInfo->begin();
				}

				//if ((*nTuples) <= 0) {
				//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
				//}
				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << *nTuples << endl;
					//cout << "new output tuple, print stats and strategy quota state during segment walking." << endl; 
					//monitor->printStateTables();
					//QuotaStrategy::printStateTable();
				}

			} //end for (segments)

			if (querySchdl::verbose && output) {
				//there are output tuples, print debug info for monitor stats
				//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
				//monitor->printStateTables();
				//QuotaStrategy::printStateTable();
				//printStateTable();
			}

			//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
			return rc;
		}

		//responsible to run the graph started from this buffer, and if applicable to pop input buffer 
		//when it is source also decrement nTuples, decrement quota_left
		//set output flag whenever at least one tuple is produced.
		//this is the bulk of segment quota strategy code. different algorithms may rewrite runNextUnit, but usually not this.
		run_rc SegmentQuotaStrategy::runState(b_entry state, int* nTuples, bool* output) 
		{
			if (querySchdl::verbose && state->b_type != t_sink && state->b_type != t_merge_sink) {
				//cout << "about to run stmt " << state->statement->name << ", Tuple remained to process for current round is " << (*nTuples) << endl;
			}

			DrvMgr* dm = DrvMgr::getInstance();
			Monitor* monitor = dm->getMonitor();

			//monitor->updateLastInTurn(state->statement);

			//first see whether this statement needs updating its quota, for now ignore quota changes based on feedback. This only takes care of changes due to initial stat availability
			stmt* s = state->statement;
			if (s!= NULL) {
				stmt_entry* stat = monitor->getStmtStat(s);
				if (s->rank.quota == this->default_quota && !s->rankSet && !this->fixed_quota && isUpdateTrigger(s)) {
					if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
						if (querySchdl::verbose) {
							cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update quota for statement " << s->name << endl;
						}

						//this is not included in regular scheduling overhead.
						struct timeval tv_start;
						struct timezone tz;
						gettimeofday(&tv_start, &tz);

						update_all(); //this set flag stateTableChanged.

						struct timeval tv_end;
						gettimeofday(&tv_end, &tz);
						double diff_proc = timeval_subtract(tv_end, tv_start);
						if (monitor->total_processing_time.on) {
							monitor->total_processing_time.value += diff_proc;
						}      

						this->last_itr = stmtsInfo->begin();

						return run_success; //need to return since underlying list<> and segment table changed
					}
				}

				//if selectivity or process rate changed significantly (more than some threshold), then update all priorities to reflect latest stat data
				if (stat!= NULL && s->rankSet && !this->fixed_quota && change_significant(stat)) {
					//this is not included in regular scheduling overhead.
					struct timeval tv_start;
					struct timezone tz;
					gettimeofday(&tv_start, &tz);

					update_all(); //this set flag stateTableChanged.

					struct timeval tv_end;
					gettimeofday(&tv_end, &tz);
					double diff_proc = timeval_subtract(tv_end, tv_start);
					if (monitor->total_processing_time.on) {
						monitor->total_processing_time.value += diff_proc;
					}      

					if (querySchdl::verbose) {
						cout << "stat changes are above threshold, update all priorities." << endl;
						printStateTable();
					} 
					this->last_itr = stmtsInfo->begin();

					return run_success; //need to return since underlying list<> and segment table changed
				}
			}

			if (state->b_type == t_source || state->b_type == t_source_fork) {
				if (state->buf->empty()) {
					if (monitor->total_empty_buffer_hit.on) {
						monitor->total_empty_buffer_hit.value++;
					}

					//if (querySchdl::verbose) cout << "source buffer empty, finish this segment." << endl;
					monitor->updateLastInTurn(state->statement);

					return run_no_input;
				}

				//Also need to check my own quota_left. This is needed since I may be a result of branch backtracking
				//if multiple backtracks go to the same source buffer, then my quota may have been exhausted. Can not solely rely on checking in RunNextUnit()
				list<quota_entry*>::iterator itr = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(state->statement->name));
				if (itr != stmtsInfo->end()) {
					if ((*itr)->quota_left <= 0) {
						return run_success;
					}
				}

				if ((*nTuples) <= 0) {
					if (querySchdl::verbose) cout << "finished one driver round." << endl;
					return run_success;
				}

			}

			//dbt* t = NULL;
			run_rc rc;
			stmt_rc s_rc;

			buffer* buf;
			int nFork = 0;
			int cnt = 0;
			BufStateTblType::iterator itr;
			list<b_entry>::iterator itr2;
			list<b_entry> dummies;

			switch (state->action) {
				case act_cjump:
					if (querySchdl::verbose) cout << "state " << state->buf->name << ", act cJump, tuple quota to process is " << *nTuples << endl;
					//currently, this can be a normal or source buffer
					if (state->b_type != t_normal && state->b_type != t_source) {
						if (querySchdl::verbose) cout << "internal state table error. cjump action with wrong buffer type: "
							<< buffer_t_strs[state->b_type] << endl;
						return run_failure;
					}

					if (state->buf->empty()) {
						if (monitor->total_empty_buffer_hit.on) {
							monitor->total_empty_buffer_hit.value++;
						}

						//directly move the state since I am empty and not an unsynchronized union
						//I know this can not be a merge buffer
						//the source case is already checked before the case statement
						monitor->updateLastInTurn(state->statement);
						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							if (querySchdl::verbose) cout << "strategy Changed 6" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						return runState((*segmentMap)[state->back.back_buf], nTuples, output);
					}

					if ((int)(s_rc = (monitor->exeStmt(state->statement, drv))) > 0 || (int) s_rc == s_no_output || (int)s_rc == s_failure) {
						if (output != NULL && s_rc > 0) (*output) = true;

						if (querySchdl::verbose) cout << "return code in cJump is " << (int) s_rc << endl;

						//I need to pop the tupple now
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
							//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
							buffer* b = state->statement->in;
							//if the other "in" is not a fork buffer, we directly pop it.
							//If it is a fork buffer, then we need to know which dummy to record a pop.
							b_entry entry2 = (*segmentMap)[b->name];
							if (entry2->b_type != t_fork && entry2->b_type != t_source_fork && entry2->b_type != t_merge_fork) {
								b->pop();
								if (querySchdl::verbose) cout << "poped " << b->name << (b->empty()?" empty":" not empty") << endl;

								if (entry2->b_type == t_source) {
									(*nTuples)--;
									//also need to update its corresponding quota_left
									list<quota_entry*>::iterator itr3 = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(entry2->statement->name));
									if (itr3 != stmtsInfo->end()) {
										((*itr3)->quota_left)--;
										if ((*itr3)->quota_left < 0) { //allow off 1 to let downstream operators to finish
											return run_success;
										}
									}

									if ((*nTuples) < 0) { //allow off 1 to let downstream operators to finish
										if (querySchdl::verbose) cout << "finished one driver round." << endl;
										return run_success;
									}
								}	  	  
							} else {
								//The buffer I need to pop is a fork.
								//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
								for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
									if (itr->second->b_type != t_fork_dummy) continue;
									if (strcmp(itr->second->inplace, b->name) == 0) {
										if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
											(itr->second->pop());
										}
									}    
								}
							}
						} else {
							state->buf->pop();
							if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

							if (state->b_type == t_source) {
								(*nTuples)--;
								//also need to update its corresponding quota_left	  
								list<quota_entry*>::iterator itr2 = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(state->statement->name));
								if (itr2 != stmtsInfo->end()) {
									((*itr2)->quota_left)--;
									if ((*itr2)->quota_left < 0) { //allow off 1 to let downstream operators to finish
										return run_success;
									}
								}

								if ((*nTuples) < 0) { //allow off 1 to let downstream operators to finish
									if (querySchdl::verbose) cout << "finished one driver round." << endl;
									return run_success;
								}
							}	  	
						}  

						//now move the state
						if ((int) s_rc > 0) {
							//I know this can not be a fork, just use the correct field
							char* nextBuf = state->forward.forward_buf;
							if ((*segmentMap)[nextBuf]->b_type == t_sink
									|| (*segmentMap)[nextBuf]->b_type == t_merge_sink) {  //condition on this line used to be commented out to do merge and find a buffer to execute (among the merge buffers)
										rc = runState((*segmentMap)[state->inplace], nTuples);
										if (rc == run_failure) return rc;
									} else {
										rc = runState((*segmentMap)[nextBuf], nTuples);
										if (rc == run_failure) return rc;
									}

							return run_success;
						} else if ((int)s_rc == s_no_output) {
							rc = runState((*segmentMap)[state->inplace], nTuples, output);

							if (rc == run_failure) return rc;
							return run_success;
						} else if ((int)s_rc == s_failure) {
							return run_failure;
						}
					} else if ((int)s_rc == s_no_input) {
						if (querySchdl::verbose) cout << "return code in cJump is " << (int) s_rc << endl;

						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
							//retrack to the buffer which is before the one set to "in" of the statement
							return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
						} else if (state->statement->type == stmt_join) {
							//retrack to the buffer which is before the one set to "back_buf" of the statement
							jStmt* s = static_cast<jStmt*> (state->statement);
							if (s->back_buf != NULL && s->back_buf != 0) {
								b_entry back_entry = (*segmentMap)[s->back_buf->name];

								if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
									return run_no_input;	    
								} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
									if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
										return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
									} else {
										if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
										return run_failure;
									}
								} else {
									return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
								}
							}
						} 
						//else {	
						if (state->b_type == t_source) {
							return run_no_input; //this should not have happened
						} else {
							//I know this can not be a merge buffer
							return runState((*segmentMap)[state->back.back_buf], nTuples, output);
						}
						//}
					}
					break;

				case act_merge:
					//currently, this can only be a merge_sink buffer
					if (querySchdl::verbose) cout << "state " << state->buf->name << ", act merge, tuple quota to process is " << *nTuples << endl;

					if (state->b_type != t_merge_sink) {
						if (querySchdl::verbose) cout << "internal state table error. merge action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
						return run_failure;
					}

					/*
					//check input buffers of the previous union statement
					uStmt* s;

					if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
					s = static_cast<uStmt*> (state->back.union_stmt);

					return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, false);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
					s = static_cast<uStmt*> (state->back.union_stmt);

					return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, true);
					} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
					return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, true);	
					} else {
					if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
					}      
					 */

					return run_success; //just do nothing in this state, not needed.
					break;

				case act_merge_jump:
					if (querySchdl::verbose) cout << "state " << state->buf->name << ", act merge_jump, tuple quota to process is " << *nTuples << endl;

					//currently, this can only be a merge buffer
					if (state->b_type != t_merge) {
						if (querySchdl::verbose) cout << "internal state table error. merge jump action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
						return run_failure;
					}

					//if my buffer has tuples in there, execute. If not, back track to the correct buffer
					if (state->buf->empty()) {
						if (monitor->total_empty_buffer_hit.on) {
							monitor->total_empty_buffer_hit.value++;
						}

						//check input buffers of the previous union statement
						monitor->updateLastInTurn(state->statement);
						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							if (querySchdl::verbose) cout << "strategy Changed 7" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						uStmt* s;

						if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);	
							return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);
							return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);	
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
							return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);
						} else {
							if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
							return run_failure;
						}      
					} else {
						//process the tuple
						if ((int)(s_rc = monitor->exeStmt(state->statement, drv)) > 0 || (int)s_rc == s_no_output || (int)s_rc == s_failure) {
							if (output != NULL && s_rc > 0) (*output) = true;

							//I need to pop the tupple now
							if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
								//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
								buffer* b = state->statement->in;
								//if the other "in" is not a fork buffer, we directly pop it.
								//If it is a fork buffer, then we need to know which dummy to record a pop.
								b_entry entry2 = (*segmentMap)[b->name];
								if (entry2->b_type != t_fork && entry2->b_type != t_source_fork && entry2->b_type != t_merge_fork) {
									b->pop();
									if (querySchdl::verbose) cout << "poped " << b->name << (b->empty()?" empty":" not empty") << endl;

									if (entry2->b_type == t_source) {
										(*nTuples)--;
										//also need to update its corresponding quota_left
										list<quota_entry*>::iterator itr3 = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(entry2->statement->name));
										if (itr3 != stmtsInfo->end()) {
											((*itr3)->quota_left)--;
											if ((*itr3)->quota_left < 0) { //allow off 1 to let downstream operators to finish
												return run_success;
											}
										}

										if ((*nTuples) < 0) { //allow off 1 to let downstream operators to finish
											if (querySchdl::verbose) cout << "finished one driver round." << endl;
											return run_success;
										}
									}	  	  
								} else {
									//The buffer I need to pop is a fork.
									//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
									for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
										if (itr->second->b_type != t_fork_dummy) continue;
										if (strcmp(itr->second->inplace, b->name) == 0) {
											if (strcmp(itr->second->forward.forward_buf, state->forward.forward_buf) == 0) {
												(itr->second->pop());
											}
										}    
									}
								}
							} else {
								state->buf->pop();
								if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

								if (state->b_type == t_source) {
									(*nTuples)--;
									//also need to update its corresponding quota_left

									list<quota_entry*>::iterator itr2 = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(state->statement->name));
									if (itr2 != stmtsInfo->end()) {
										((*itr2)->quota_left)--;
										if ((*itr2)->quota_left < 0) { //allow off 1 to let downstream operators to finish
											return run_success;
										}
									}

									if ((*nTuples) < 0) { //allow off 1 to let downstream operators to finish
										if (querySchdl::verbose) cout << "finished one driver round." << endl;
										return run_success;
									}
								}	  	
							}  	

							//move the state
							if ((int) s_rc > 0) {
								//I know this can not be a fork, just use the correct field
								char* nextBuf = state->forward.forward_buf;
								if ((*segmentMap)[nextBuf]->b_type == t_sink) { //can not be merge_sink either
									return runState((*segmentMap)[state->inplace], nTuples);

								} else {
									return runState((*segmentMap)[nextBuf], nTuples);
								}
							} else if ((int)s_rc == s_no_output) {
								return runState((*segmentMap)[state->inplace], nTuples, output);
							} else if ((int)s_rc == s_failure) {
								return run_failure;
							}
						} else if ((int)s_rc == s_no_input) {
							if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
								//retrack to the buffer which is before the one set to "in" of the statement
								return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
							} else if (state->statement->type == stmt_join) {
								//retrack to the buffer which is before the one set to "back_buf" of the statement
								jStmt* s = static_cast<jStmt*> (state->statement);
								if (s->back_buf != NULL && s->back_buf != 0) {
									b_entry back_entry = (*segmentMap)[s->back_buf->name];

									if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
										return run_success;	    
									} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
										if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
											uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
											return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
										} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
											uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
											return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
										} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
											return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
										} else {
											if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
											return run_failure;
										}
									} else {
										return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
									}
								}
							}
							//else {
							//check input buffers of the previous union statement
							uStmt* s;

							if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
								s = static_cast<uStmt*> (state->back.union_stmt);
								return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
							} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
								s = static_cast<uStmt*> (state->back.union_stmt);
								return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
							} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
								return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);	
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

					if (state->buf->empty()) {
						monitor->updateLastInTurn(state->statement);
						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							if (querySchdl::verbose) cout << "strategy Changed 8" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						//I know I am not a merge buffer
						//the source case is already handled.
						return runState((*segmentMap)[state->back.back_buf], nTuples, output);
					}

					nFork = state->forward.fork_count;
					//now find out the dummy states, the # should agree with nFork
					cnt = 0;
					for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
						if (itr->second->b_type != t_fork_dummy) continue;
						if (strcmp(itr->second->inplace, state->buf->name) == 0) {
							dummies.push_back(itr->second);
							cnt++;
						}    
					}
					if (nFork != cnt) {
						cerr << "internal error, number of dummy states does not match the fork count for buffer: " << state->buf->name << endl;
						return run_failure;
					}

					itr2 = dummies.begin();
					while (1) {
						if (state->b_type == t_source_fork) {
							//I am a source, first test if I already processed my quota
							if ((*nTuples) <= 0) {
								return run_success;
							}
						}

						//check if this branch has been processed before.
						b_entry entry = *itr2;
						if (entry->back.processed == true) {
							nFork--;
							itr2++;
						} else {    
							rc = runState(entry, nTuples, output);      

							if (nFork == cnt && rc == run_no_input) {
								if (state->b_type == t_source_fork) {
									//if (querySchdl::verbose) cout << "source buffer empty, finish this branch." << endl;	  
									return run_success;
								} else {
									//this is not be a merge buffer
									return runState((*segmentMap)[state->back.back_buf], nTuples, output);
								}
							} else if (rc == run_failure) {
								return run_failure;
							}

							//advance the branch iterator only if I am processed (I may not, if my downstream is a join)
							if ((*itr2)->back.processed != true) {
								continue;
							} else {
								if (querySchdl::verbose) cout << "move fork pointer to process next one." << endl;
								nFork--;
								itr2++;
							}
						}

						if (nFork > 0 && itr2 == dummies.end()) {
							cerr << "internal error, dummy state list length and fork count mismatch for buffer: " << state->buf->name << endl;
							return run_failure;
						}

						if (nFork == 0 && itr2 == dummies.end()) {
							//the last fork branch is already called
							//make sure all are processed, otherwise, continue to loop
							bool finished = true;
							nFork = cnt;
							for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end() && finished; itr3++) {
								if ((*itr3)->back.processed == false) {
									finished = false;
									itr2 = itr3;
								} else {
									nFork--;
								}
							}

							if (finished) {	  
								//reset every flag, since we have finished one round for all branches.
								for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
									(*itr3)->back.processed = false;
								}

								state->buf->pop();
								if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

								if (state->b_type == t_source_fork) {
									(*nTuples)--;
									//also need to update its corresponding quota_left
									list<quota_entry*>::iterator itr2 = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(state->statement->name));
									if (itr2 != stmtsInfo->end()) {
										((*itr2)->quota_left)--;
										if ((*itr2)->quota_left < 0) { //allow off 1 to let downstream operators to finish
											return run_success;
										}
									}

									if ((*nTuples) < 0) { //allow off 1 to let downstream operators to finish
										if (querySchdl::verbose) cout << "finished one driver round." << endl;
										return run_success;
									}
								}	  	

								//now move the state, first for the buffer empty case
								if (state->b_type == t_source_fork) {
									if (state->buf->empty()) {
										if (monitor->total_empty_buffer_hit.on) {
											monitor->total_empty_buffer_hit.value++;
										}

										monitor->updateLastInTurn(state->statement);
										//if (querySchdl::verbose) cout << "source buffer empty, finish this branch." << endl;
										return run_success;
									}
								} else if (state->buf->empty()) {
									if (monitor->total_empty_buffer_hit.on) {
										monitor->total_empty_buffer_hit.value++;
									}

									monitor->updateLastInTurn(state->statement);
									//if strategy is changed, this flag is set, return immediately
									if (this->drv->stateTableChanged) {
										if (querySchdl::verbose) cout << "strategy Changed 9" << endl;
										return run_success; //leave immediately after underlying table change.
									}

									//I know I am not a merge buffer
									return runState((*segmentMap)[state->back.back_buf], nTuples, output);
								}

								//non-empty case
								//restart the while loop
								nFork = cnt;
								itr2 = dummies.begin();
								if (querySchdl::verbose) cout << "fork flags reset for next round" << endl;
							} // end if (finished)
						} //end if (reaching the end of branches)
					} //end while
					break;

				case act_merge_cfork:
					if (querySchdl::verbose) cout << "state " << state->buf->name << ", act merge_cFork, tuple quota to process is " << *nTuples << endl;
					//currently, this can only be a merge-fork buffer
					if (state->b_type != t_merge_fork) {
						if (querySchdl::verbose) cout << "internal state table error. merge cfork action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
						return run_failure;
					}

					//if my buffer has tuples in there, execute. If not, back track to the correct buffer
					if (state->buf->empty()) {
						if (monitor->total_empty_buffer_hit.on) {
							monitor->total_empty_buffer_hit.value++;
						}

						monitor->updateLastInTurn(state->statement);
						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							if (querySchdl::verbose) cout << "strategy Changed 10" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						//check input buffers of the previous union statement
						uStmt* s;

						if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);	
							return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
							s = static_cast<uStmt*> (state->back.union_stmt);
							return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);	
						} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
							return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);
						} else {
							if (querySchdl::verbose) cout << "internal error. merge jump action with wrong statement type. "  << endl;
							return run_failure;
						}      
					} else {
						//we move forward      
						nFork = state->forward.fork_count;
						//now find out the dummy states, the # should agree with nFork
						cnt = 0;
						for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
							if (itr->second->b_type != t_fork_dummy) continue;
							if (strcmp(itr->second->inplace, state->buf->name) == 0) {
								dummies.push_back(itr->second);
								cnt++;
							}    
						}
						if (nFork != cnt) {
							cerr << "internal error, number of dummy states does not match the fork count for buffer: " << state->buf->name << endl;
							return run_failure;
						}

						itr2 = dummies.begin();
						while (1) {
							//check if this branch has been processed before.
							b_entry entry = (*itr2);
							if (entry->back.processed == true) {
								nFork--;
								itr2++;
							} else {
								rc = runState(entry, nTuples, output);
								if (nFork == cnt && rc == run_no_input) {
									if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
										//retrack to the buffer which is before the one set to "in" of the statement
										return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
									} else if (state->statement->type == stmt_join) {
										//retrack to the buffer which is before the one set to "back_buf" of the statement
										jStmt* s = static_cast<jStmt*> (state->statement);
										if (s->back_buf != NULL && s->back_buf != 0) {
											b_entry back_entry = (*segmentMap)[s->back_buf->name];

											if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
												return run_success;	    
											} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
												if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
													uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
													return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
												} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
													uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
													return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
												} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
													return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
												} else {
													if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
													return run_failure;
												}
											} else {
												return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
											}
										}
									} else {
										//need to go to the upstream of myself.
										//check input buffers of the previous union statement
										uStmt* s;	    
										if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_t_union) {
											s = static_cast<uStmt*> (state->back.union_stmt);
											return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
										} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_tl_union) {
											s = static_cast<uStmt*> (state->back.union_stmt);
											return runState((*segmentMap)[(*(s->union_bufs.begin()))->name], nTuples, output);
										} else if (state->back.union_stmt != 0 && state->back.union_stmt->type == stmt_join) {
											return runState((*segmentMap)[state->back.union_stmt->in->name], nTuples, output);	
										} else {
											if (querySchdl::verbose) cout << "internal error. merge cfork action with wrong statement type. "  << endl;
											return run_failure;
										}
									}
								} else if (rc == run_failure) {
									return run_failure;
								}

								//advance the branch iterator only if I am processed (I may not, if my downstream is a union)
								if ((*itr2)->back.processed != true) {
									continue;
								} else {
									if (querySchdl::verbose) cout << "move fork pointer to process next one." << endl;
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
								//make sure all are processed, otherwise, continue to loop
								bool finished = true;
								nFork = cnt;
								for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end() && finished; itr3++) {
									if ((*itr3)->back.processed == false) {
										finished = false;
										itr2 = itr3;
									} else {
										nFork--;
									}
								}

								if (finished) {
									//reset every flag, since we have finished one round for all branches.
									for (list<b_entry>::iterator itr3 = dummies.begin(); itr3!= dummies.end(); itr3++) {
										(*itr3)->back.processed = false;
									}

									state->buf->pop();
									if (querySchdl::verbose) cout << "poped " << state->buf->name << (state->buf->empty()?" empty":" not empty") << endl;

									//restart the while loop, and reset all processed flag on branches.
									nFork = cnt;
									itr2 = dummies.begin();
									if (querySchdl::verbose) cout << "fork flags reset for next round" << endl;
								} // end if (finished)
							} //end if (reaching the end of branches)
						} //end while
					} //end if (empty buffer)
					break;

				case act_jump:
					if (querySchdl::verbose) cout << "state " << state->inplace << state->forward.forward_buf << ", act jump, tuple quota to process is " << *nTuples << endl;
					//currently, this can be a fork dummy buffer
					//this will not check for source, or pop the tuple, unlike cjump.
					if (state->b_type != t_fork_dummy) {
						if (querySchdl::verbose) cout << "internal state table error. jump action with wrong buffer type: " << buffer_t_strs[state->b_type] << endl;
						return run_failure;
					}

					//I am already processed in this round of fork
					if (state->back.processed) {
						return run_success;
					}

					if ((int)(monitor->exeStmt(state->statement, drv)) > 0 || (int)s_rc == s_no_output || (int)s_rc == s_failure) {
						//pop myself, unless the following buffer is a union. If union, I then need to look up the correct one to pop
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union || state->statement->type == stmt_join) {
							//The next is a union statement, which means I may not pop myself, instead I pop the buffer returned in (*in) of the statement
							buffer* b = state->statement->in;
							//if the other "in" is not a fork buffer, we directly pop it.
							//If it is a fork buffer, then we need to know which dummy to record a pop.
							b_entry entry2 = (*segmentMap)[b->name];
							if (entry2->b_type != t_fork && entry2->b_type != t_source_fork && entry2->b_type != t_merge_fork) {
								b->pop();
								if (querySchdl::verbose) cout << "poped " << b->name << (b->empty()?" empty":" not empty") << endl;

								if (entry2->b_type == t_source) {
									(*nTuples)--;
									//also need to update its corresponding quota_left
									list<quota_entry*>::iterator itr3 = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(entry2->statement->name));
									if (itr3 != stmtsInfo->end()) {
										((*itr3)->quota_left)--;
										if ((*itr3)->quota_left < 0) { //allow off 1 to let downstream operators to finish
											return run_success;
										}
									}

									if ((*nTuples) < 0) { //allow off 1 to let downstream operators to finish
										if (querySchdl::verbose) cout << "finished one driver round." << endl;
										return run_success;
									}
								}	  	  
							} else {
								//The buffer I need to pop is a fork.
								//since I only get the original buffer pointers in the statement itself, I need to look up the dummy here.
								for (itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
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
						if ((int) s_rc > 0) {
							//I know this can not be a fork, just use the correct field
							char* nextBuf = state->forward.forward_buf;
							if ((*segmentMap)[nextBuf]->b_type == t_sink
									|| (*segmentMap)[nextBuf]->b_type == t_merge_sink) {  //condition on this line used to be commented out to do merge and find a buffer to execute (among the merge buffers)
										return run_success;
									} else {
										rc = runState((*segmentMap)[nextBuf], nTuples);
										if (rc == run_failure) return rc;
									}
							return run_success;
						} else if ((int)s_rc == s_no_output) {
							return run_success;
						} else if ((int)s_rc == s_failure) {
							return run_failure;
						} 
					} else if ((int)s_rc == s_no_input) {
						if (state->statement->type == stmt_t_union || state->statement->type == stmt_tl_union) {
							//retrack to the buffer which is before the one set to "in" of the statement
							return runState((*segmentMap)[(((*segmentMap)[state->statement->in->name])->back.back_buf)], nTuples, output);
						} else if (state->statement->type == stmt_join) {
							//retrack to the buffer which is before the one set to "back_buf" of the statement
							jStmt* s = static_cast<jStmt*> (state->statement);
							if (s->back_buf != NULL && s->back_buf != 0) {
								b_entry back_entry = (*segmentMap)[s->back_buf->name];

								if (back_entry->b_type == t_source || back_entry->b_type == t_source_fork) {
									return run_success;	    
								} else if(back_entry->b_type == t_merge || back_entry->b_type == t_merge_fork) {
									if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_t_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_tl_union) {
										uStmt* s2 = static_cast<uStmt*> (back_entry->back.union_stmt);
										return runState((*segmentMap)[(*(s2->union_bufs.begin()))->name], nTuples, output);	
									} else if (back_entry->back.union_stmt != 0 && back_entry->back.union_stmt->type == stmt_join) {
										return runState((*segmentMap)[back_entry->back.union_stmt->in->name], nTuples, output);
									} else {
										if (querySchdl::verbose) cout << "internal error. merge buffer with wrong statement type. "  << endl;
										return run_failure;
									}
								} else {
									return runState((*segmentMap)[(((*segmentMap)[s->back_buf->name])->back.back_buf)], nTuples, output);
								}
							}
						} //else {
						return run_no_input;
						//}
					}
					break;    

				case act_none:
					if (querySchdl::verbose) cout << "null action for buffer: " << state->buf->name << endl;
					return run_success;

				default:
					if (querySchdl::verbose) cout << "action not recognizable for buffer: " << state->buf->name << endl;
					return run_failure;
			}

			//all correct returns should have happened in the switch statement. Error if I reach here.
			if (querySchdl::verbose) cout << "strategy error, switch statement does not return for: " << state->buf->name << endl;
			return run_failure;
		}


		void SegmentQuotaStrategy::printStateTable()
		{
			QuotaStrategy::printStateTable();  
			drv->printStateTable(this->segmentMap);
			return;
		}

		OcsStrategy::OcsStrategy(Driver* drv):OperatorPriorityStrategy(drv)
		{
			name = "OcsOp";

			//calculate all priorities and construct table
			for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {    
				update_entry(*itr);
			}

			if (querySchdl::verbose) {
				printStateTable();  
			}
		}

		OcsStrategy::~OcsStrategy()
		{
		}

		//just update all, do it simple for now. 
		void OcsStrategy::update() {
			return update_all();
		}

		void OcsStrategy::printStateTable()
		{
			PriorityStrategy::printStateTable();

			cout << "Algorithm specific states(path_capacity and path_selectivity): " << endl;
			cout.setf(ios_base::fixed);
			for (StratInfoType::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
				ocs_strat_entry* entry = (ocs_strat_entry*) itr->second;
				cout << "(" << entry->stmt_name << "," << entry->path_capacity << "," << entry->path_selectivity << ") ";
			}

			cout << endl << endl;
		}

		bool ocs_entry::change_significant(stmt_entry* s_entry) {
			if (s_entry == NULL) return false;

			double selectivity = (double)s_entry->total_output.value/s_entry->total_input.value;
			double process_rate = (double)s_entry->total_input.value/s_entry->total_p_time.value;
			bool changed = false;
			if ((selectivity > 0 && s_entry->last_known_selectivity == 0) || (process_rate > 0 && s_entry->last_known_process_rate == 0)) {
				changed = true;
			} else if (s_entry->last_known_selectivity > 0 || s_entry->last_known_process_rate > 0) {
				if (s_entry->last_known_selectivity > 0 && abs((int)(selectivity - s_entry->last_known_selectivity))/s_entry->last_known_selectivity > OcsStrategy::SEL_CHANGE_THRESHOLD) {
					changed = true;
				}

				if (s_entry->last_known_process_rate > 0 && abs((int)(process_rate - s_entry->last_known_process_rate))/s_entry->last_known_process_rate > OcsStrategy::PROC_CHANGE_THRESHOLD) {
					changed = true;
				}
			}
			if (changed && querySchdl::verbose) {
				cout << "stat change determined to be significant. " << endl;
			}

			if (changed) {
				s_entry->last_known_selectivity = selectivity;
				s_entry->last_known_process_rate = process_rate;
			}

			return changed;
		}

		bool OcsStrategy::change_significant(stmt_entry* s_entry) {
			return ocs_entry::change_significant(s_entry);
		}

		bool OcsStrategy::isUpdateTrigger(stmt* s) {
			if (s != NULL) return s->monitor_sink_stat;
			return true;
		}

		//if a statement does not have any stat yet, then put it at MAX_PRIORITY, so that it will always be processed first until enough stat is gathered.
		void OcsStrategy::update_entry(stmt* s) {  

			//only deal with the case of setting it to non-default the first time, updating existing affect upstream operators, too complicated to handle here for now.
			if (s == NULL || s->rank.priority != PriorityStrategy::MAX_PRIORITY) {    
				return;
			}

			if (querySchdl::verbose) cout << "update entry called for " << s->name << endl;

			//check if my entry exists, if not, create one.
			//pair<StratInfoType::iterator, StratInfoType::iterator> p = stmtsInfo->equal_range(s->rank.priority);
			bool found = false;
			ocs_strat_entry* entry;
			StratInfoType::iterator itr_find;
			//for ( itr_find = p.first; itr_find != p.second && !found; itr_find++) {
			for ( StratInfoType::iterator itr_find = stmtsInfo->begin(); itr_find != stmtsInfo->end() && !found; itr_find++) {
				if(strcmp(itr_find->second->stmt_name, s->name) == 0) {
					found = true;
					entry = (ocs_strat_entry*)itr_find->second;
					break;
				}
			}

			if (!found) {
				entry = new ocs_strat_entry(s->name);
			}

			//first make sure I have enough stats to calculate priority, if not, just return
			bool hasStat = false;
			DrvMgr* dm = DrvMgr::getInstance();
			Monitor* monitor = dm->getMonitor();
			stmt_entry* stat = monitor->getStmtStat(s);
			if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
				hasStat = true;
			}

			if(!hasStat) {
				if (querySchdl::verbose) cout << "stat not available yet for " << s->name << endl;
				if(!found) {
					stmtsInfo->insert(pair<double, strat_entry*>(0 - (s->rank.priority), entry));    
				}
				return;
			}

			if (querySchdl::verbose) cout << "stat available for " << s->name << ", calculate priority now." << endl;

			double priority = PriorityStrategy::MAX_PRIORITY;
			double path_capacity = PriorityStrategy::MAX_PRIORITY;
			double path_selectivity = PriorityStrategy::MAX_PRIORITY;

			//build from end of path, backwards towards the front of path. 
			//This way forks are easily handled (the upstream operator calculate priority based on the highest of is suceeding operators)
			//inductively, the last one Cp(k) = 1/(1/Co(k)), the prior one is Cp(k-1) = (1/Sigma(k-1)) / ((1/(Co(k-1)*Sigma(k-1))) + 1/Cp(k))
			//priority(k) = Op(k) = Cp(k) * Sigma-p(k)

			//first make sure all downstream operators have priorities, if not, update them first
			BufStateTblType* graph = drv->getBufGraph();
			b_entry graph_entry;
			if (graph->find(s->out->name) != graph->end()) {
				graph_entry = (*graph)[s->out->name];
				//if I am a sink, just calculate and return
				if(graph_entry->b_type == t_sink || graph_entry->b_type == t_merge_sink) {
					if (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT) {
						path_capacity = (double)stat->total_input.value/stat->total_p_time.value;
						path_selectivity = (double)stat->total_output.value/stat->total_input.value;

						if (querySchdl::verbose) {
							cout << "for sink stmt " << s->name << ", the capacity is " << path_capacity << endl;
						}

					} else {
						path_capacity = stat->last_known_process_rate;
						path_selectivity = stat->last_known_selectivity;
					}
					priority = calculatePriority(path_capacity, path_selectivity, stat);
					//priority = path_capacity*path_selectivity;
				} else {
					//there are statements downstream of me.
					stmt* s2 = NULL;
					if (graph_entry->b_type == t_fork || graph_entry->b_type == t_source_fork) {
						//there are multiple immediate downstream statements, choose the highest one.
						int nFork = graph_entry->forward.fork_count;
						//now find out the dummy states, the # should agree with nFork
						int cnt = 0;
						list<b_entry> dummies;
						for (BufStateTblType::iterator itr3 = graph->begin(); itr3 != graph->end(); itr3++) {
							if (itr3->second->b_type != t_fork_dummy) continue;
							if (strcmp(itr3->second->inplace, s->out->name) == 0) {
								dummies.push_back(itr3->second);
								cnt++;
							}    
						}

						if (nFork == cnt) {
							bool available = true;	  
							for(list<b_entry>::iterator itr4 = dummies.begin(); itr4 != dummies.end() && available; itr4++) {
								stmt* s3 = (*itr4)->statement;
								if (s3->rank.priority == PriorityStrategy::MAX_PRIORITY) {
									//get downstream priority first, before calculating my own
									update_entry(s3);	  
								}
								//check again after update, to see if info is available
								if (s3->rank.priority == PriorityStrategy::MAX_PRIORITY) {
									available = false;
									s2 = NULL;
									break;
								} else if (s2 = NULL) {
									s2 = s3;
								} else {
									//make sure s2 is set to the branch with the highest priority
									if(s2->rank.priority < s3->rank.priority) s2 = s3;
								}
							}
						}
					} else {
						//there is only one immediate downstream statement
						s2 = graph_entry->statement;
						if (s2->rank.priority == PriorityStrategy::MAX_PRIORITY) {
							//get downstream priority first, before calculating my own
							if (querySchdl::verbose) cout << "inside update entry to call downstream stmt update first: " << s->name << " call " << s2->name << endl;
							update_entry(s2);	  
						}
					}

					if (s2 != NULL && s2->rank.priority != PriorityStrategy::MAX_PRIORITY) {
						//downstream stats all exists and have priority values, I can update my own now.
						if (querySchdl::verbose) cout << "update self-value after getting downstream stmt priority: " << s->name << endl;
						ocs_strat_entry* entry2 = NULL;
						bool found2 = false;

						//if (querySchdl::verbose) {
						//  cout << " before calculate found2.." << endl;
						//  printStateTable();
						//}

						//pair<StratInfoType::iterator, StratInfoType::iterator> p2 = stmtsInfo->equal_range(s2->rank.priority);
						//for ( StratInfoType::iterator itr2 = p2.first; itr2 != p2.second && !found2; itr2++) {
						for ( StratInfoType::iterator itr2 = stmtsInfo->begin(); itr2 != stmtsInfo->end() && !found2; itr2++) {
							//if (querySchdl::verbose) {
							//  cout << "in calc found2, s2->name is " << s2->name << ", itr2->second->stmt_name is " << itr2->second->stmt_name << endl;
							//}
							if(strcmp(itr2->second->stmt_name, s2->name) == 0) {
								found2 = true;
								entry2 = (ocs_strat_entry*)itr2->second;
							}
						}

						//if (querySchdl::verbose) cout << "found2 is " << found2 << ", s2->rank.priority is " << s2->rank.priority << endl;

						if (entry2 != NULL) {
							double op_capacity, op_selectivity;
							if (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT) {
								op_capacity = (double)stat->total_input.value/stat->total_p_time.value;
								op_selectivity = (double)stat->total_output.value/stat->total_input.value;
							} else {
								op_capacity = stat->last_known_process_rate;
								op_selectivity = stat->last_known_selectivity;
							}
							//if (querySchdl::verbose) {
							//  cout << "calc priority. op_capacity is " << op_capacity << ", op_selectivity is " << op_selectivity << endl;
							//}
							path_selectivity = op_selectivity*entry2->path_selectivity;
							path_capacity = (1/op_selectivity)/((1/(op_capacity*op_selectivity))+(1/entry2->path_capacity));
							priority = calculatePriority(path_capacity, path_selectivity, stat);
						}
					}

					}
				}

				entry->path_capacity = path_capacity;
				entry->path_selectivity = path_selectivity;
				s->rank.priority = priority;
				s->rankSet = true;

				//since this is by definition in acending order, use the minus of priority as the key, to ensure high priority at front
				//if (found) stmtsInfo->erase(itr_find);
				if (found) { 
					for ( StratInfoType::iterator itr_find2 = stmtsInfo->begin(); itr_find2 != stmtsInfo->end(); itr_find2++) {
						if(strcmp(itr_find2->second->stmt_name, s->name) == 0) {
							stmtsInfo->erase(itr_find2);
							break;
						}
					}
				}
				stmtsInfo->insert(pair<double, strat_entry*>(0 - (s->rank.priority), entry));    

				return;
			}

			double OcsStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity*path_selectivity;
			}

			PcsStrategy::PcsStrategy(Driver* drv):OcsStrategy(drv)
			{
				name = "PcsOp";
			}

			PcsStrategy::~PcsStrategy()
			{
			}

			double PcsStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity;
			}

			TrainStrategy::TrainStrategy(Driver* drv):OcsStrategy(drv)
			{
				name = "TrainOp";
			}

			TrainStrategy::~TrainStrategy()
			{
			}

			double TrainStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity*(double)stat->input_tuple_size;
			}

			RRStrategy::RRStrategy(Driver* drv, long quota, bool fixed):SegmentQuotaStrategy(drv, quota, fixed)
			{
				name = "RR";

				//initialize the stmtsInfo list with segments information.
				//each stmt with some source buffer (later will include those become source buffer after segmentation) has an entry. 
				//execution will be bush-depth-first on each stmt, round robin on stmts, with quota
				for (BufStateTblType::iterator itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
					if (itr->second->statement != NULL) itr->second->statement->rankSet = true;
					if (itr->second->b_type == t_source || itr->second->b_type == t_source_fork) {
						//if(find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(itr->second->statement->name)) == stmtsInfo->end()) {
						//this statement has source buffer and is not in the list yet
						//quota_entry* entry = new quota_entry(itr->second->statement->name, quota);
						//stmtsInfo->push_back(entry);
						update_entry(itr->second->statement);
						//}
					}
				}

				if (querySchdl::verbose) {
					printStateTable();  
				}
			}

			RRStrategy::~RRStrategy()
			{
			}

			//just update all, do it simple for now. 
			void RRStrategy::update() {
				return update_all();
			}

			//just update all, do it simple for now. 
			void RRStrategy::update_all() {
				SegmentQuotaStrategy::update_all();
				//in RR we are sure every rank is set.
				for (BufStateTblType::iterator itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
					if (itr->second->statement != NULL) itr->second->statement->rankSet = true;
				}  
			}

			bool RRStrategy::change_significant(stmt_entry* s_entry) {
				//no stat is considered, pure basic RR
				return false;
			}

			//long OcsQuotaStrategy::TOP_QUOTA = 2971; //for 30 queries, 100x
			//long OcsQuotaStrategy::TOP_QUOTA = 871; //for 30 queries, 30x
			//long OcsQuotaStrategy::TOP_QUOTA = 421; //for 30 queries, 15x
			//long OcsQuotaStrategy::TOP_QUOTA = 151; //for 30 queries, 6x
			//long OcsQuotaStrategy::TOP_QUOTA = 61;  //for 30 queries, 3x
			//long OcsQuotaStrategy::TOP_QUOTA = 31;  //for 30 queries, 2x

			//long OcsQuotaStrategy::TOP_QUOTA = 46; //for 9 queries, 6x
			long OcsQuotaStrategy::TOP_QUOTA = 441; //for 9 queries, 50x

			long OcsQuotaStrategy::NEW_QUOTA = 10;

			//long OcsQuotaStrategy::TOP_QUOTA = 10;
			//long OcsQuotaStrategy::NEW_QUOTA = 2;

			OcsQuotaStrategy::OcsQuotaStrategy(Driver* drv, long quota, bool fixed):SegmentQuotaStrategy(drv, quota, fixed)
			{
				name = "OcsSwpQC";

				//initialize the stmtsInfo list with segments information.
				//each stmt with some source buffer (later will include those become source buffer after segmentation) has an entry. 
				//execution will be bush-depth-first on each stmt, round robin on stmts, with quota
				for (BufStateTblType::iterator itr = segmentMap->begin(); itr != segmentMap->end(); itr++) {
					if (itr->second->b_type == t_source || itr->second->b_type == t_source_fork) {
						//the following two lines done in parents now
						//itr->second->statement->rank.quota = this->default_quota; //clear old info first, then calculate new one.
						//itr->second->statement->rankSet = false;

						//if(find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(itr->second->statement->name)) == stmtsInfo->end()) {
						//this statement has source buffer and is not in the list yet
						//quota_entry* entry = new quota_entry(itr->second->statement->name, quota);
						//stmtsInfo->push_back(entry);
						update_entry(itr->second->statement);
						//}
					}
				}

				//set quotas based on calculated priorities, which is extra here.
				//first, copy priority info from extraInfo over to stmtInfo, then sort the stmtsInfo list by priority, before assign quotas accordingly.
				for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
					if (extraInfo->find((*itr)->stmt_name) != extraInfo->end()) {
						(*itr)->rank.priority = (*extraInfo)[(*itr)->stmt_name]->rank.priority;
					} else {
						(*itr)->rank.priority = PriorityStrategy::MAX_PRIORITY;
					}
				}

				stmtsInfo->sort(greater_on_rank());

				list<quota_entry*>::iterator itr2;
				for ( itr2 = stmtsInfo->begin(); itr2 != stmtsInfo->end() && (*itr2)->rank.priority == PriorityStrategy::MAX_PRIORITY; itr2++) {
					(*itr2)->quota = NEW_QUOTA;
					(*itr2)->quota_left = NEW_QUOTA;
				}  

				//this is the largest priority (besides the MAX ones, which is really just default)
				if (itr2 != stmtsInfo->end()) {
					(*itr2)->quota = TOP_QUOTA;
					(*itr2)->quota_left = TOP_QUOTA;
				}

				if (querySchdl::verbose) {
					printStateTable();  
				}  
			}

			OcsQuotaStrategy::~OcsQuotaStrategy()
			{
			}

			//just update all, do it simple for now. 
			void OcsQuotaStrategy::update() {
				return update_all();
			}

			void OcsQuotaStrategy::update_all() {
				SegmentQuotaStrategy::update_all();

				//the above default implementation only updates entries to set default quota, although priorities would have been calculated already. 
				//set quota based on ocs priorities here. It can not be done individually, since here a global view of all priorities is needed.
				//currently, the decision is that to assign most quota to the first one, which is to be re-distributed later.

				//first, copy over priority info from extraInfo over to stmtInfo, then sort the stmtsInfo list by priority, before assign quotas accordingly.
				for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
					if (extraInfo->find((*itr)->stmt_name) != extraInfo->end()) {
						(*itr)->rank.priority = (*extraInfo)[(*itr)->stmt_name]->rank.priority;
					}
				}

				stmtsInfo->sort(greater_on_rank());

				list<quota_entry*>::iterator itr2;
				for ( itr2 = stmtsInfo->begin(); itr2 != stmtsInfo->end() && (*itr2)->rank.priority == PriorityStrategy::MAX_PRIORITY; itr2++) {
					(*itr2)->quota = NEW_QUOTA;
					(*itr2)->quota_left = NEW_QUOTA;
				}  

				//this is the largest priority (besides the MAX ones, which is really just default)
				if (itr2 != stmtsInfo->end()) {
					(*itr2)->quota = TOP_QUOTA;
					(*itr2)->quota_left = TOP_QUOTA;
				}

				this->last_itr = stmtsInfo->begin(); //container re-sorted, old value no longer valid.

				if (querySchdl::verbose) {
					printStateTable();  
				}

				return;
			}

			bool OcsQuotaStrategy::change_significant(stmt_entry* s_entry) {
				return ocs_entry::change_significant(s_entry);  
			}

			bool OcsQuotaStrategy::isUpdateTrigger(stmt* s) {
				if (s != NULL) return s->monitor_sink_stat;
				return true;
			}

			double OcsQuotaStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity*path_selectivity;
			}

			void OcsQuotaStrategy::printStateTable()
			{
				cout << "current strategy has " << stmtsInfo->size() << " stmts, the order of stmts is (format stmt, quota, quota_left, priority): " << endl;

				for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
					cout << "(" << (*itr)->stmt_name << "," << (*itr)->quota << "," << (*itr)->quota_left << "," << (*itr)->rank.priority << ") ";
				}

				cout << endl << endl;

				cout << "current extra info in strategy (stmt_name, priority, path_capacity, path_selectivity): " << endl;

				for (ExtraInfoMapType::iterator itr = extraInfo->begin(); itr != extraInfo->end(); itr++) {
					ocs_quota_extra_entry* ety = (ocs_quota_extra_entry*) itr->second;
					cout << "(" << itr->first << "," << ety->rank.priority << "," << ety->path_capacity << "," << ety->path_selectivity << ") ";
				}

				cout << endl << endl;

				cout << "quota and rankSet flags in stmts:" << endl;

				for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
					cout << "(" << (*itr)->name << "," << (*itr)->rank.quota << ",rankSet " << ((*itr)->rankSet?"true)":"false)");
				}

				cout << endl << endl;  

				drv->printStateTable(this->segmentMap);
				return;
			}

			//this function now has to take care of calculating ocs related information in extraInfo too, besides handling stmtsInfo.
			void OcsQuotaStrategy::update_entry(stmt* s) {  
				if (s == NULL) {
					return;
				}

				if (querySchdl::verbose) cout << "update entry called for " << s->name << endl;

				//check if my entry exists, if not, create one.
				list<quota_entry*>::iterator itr_find;
				if((itr_find = find_if(stmtsInfo->begin(), stmtsInfo->end(), eqse(s->name))) == stmtsInfo->end()) {
					quota_entry* entry = new quota_entry(s->name, this->default_quota);
					stmtsInfo->push_back(entry);
					s->rank.quota = this->default_quota;
				} else {
					//if it is already set, simply use it, no update.
					//if ((*itr_find)->rank.priority != PriorityStrategy::MAX_PRIORITY) return;
					if (s->rankSet) return;
				}

				//now calculate quota based on ocs information. And if such information was not available, build it first.
				//this function recursively calculate my ocs priority, which is also responsible to build up the extraInfo list, if necessary
				//extraInfo should have ocs_quota_entries, and appropriate numbers set.
				updatePriority(s);

				return;
			}

			//recursively calculate my ocs priority, which is also responsible to build up the extraInfo list, if necessary
			//extraInfo should have ocs_quota_entries, and appropriate numbers set.
			double OcsQuotaStrategy::updatePriority(stmt* s) {

				//check if my entry exists, if not, create one.
				//this entry should exist for every statement, unlike the stmtsInfo entry only exist for sources, which drives segments.
				extra_info_entry* entry1; 
				if(extraInfo->find(s->name) == extraInfo->end()) {
					entry1 = new ocs_quota_extra_entry();
					char * s_name = new char[strlen(s->name)+1];
					strcpy(s_name, s->name);
					(*extraInfo)[s_name] = entry1;
				} else {
					entry1 = (*extraInfo)[s->name];
					if (entry1->rank.priority != PriorityStrategy::MAX_PRIORITY) {
						//already has priority, simply return
						return entry1->rank.priority;
					}
				}

				ocs_quota_extra_entry* entry =  (ocs_quota_extra_entry*) entry1;

				//first make sure I have enough stats to calculate priority, if not, just return
				bool hasStat = false;
				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();
				stmt_entry* stat = monitor->getStmtStat(s);
				if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
					hasStat = true;
				}

				if(!hasStat) {
					if (querySchdl::verbose) cout << "stat not available yet for " << s->name << endl;
					return PriorityStrategy::MAX_PRIORITY;
				}

				if (querySchdl::verbose) cout << "stat available for " << s->name << ", calculate priority now." << endl;

				double priority = PriorityStrategy::MAX_PRIORITY;
				double path_capacity = PriorityStrategy::MAX_PRIORITY;
				double path_selectivity = PriorityStrategy::MAX_PRIORITY;

				//build from end of path, backwards towards the front of path. 
				//This way forks are easily handled (the upstream operator calculate priority based on the highest of is suceeding operators)
				//inductively, the last one Cp(k) = 1/(1/Co(k)), the prior one is Cp(k-1) = (1/Sigma(k-1)) / ((1/(Co(k-1)*Sigma(k-1))) + 1/Cp(k))
				//priority(k) = Op(k) = Cp(k) * Sigma-p(k)

				//first make sure all downstream operators have priorities, if not, update them first
				BufStateTblType* graph = drv->getBufGraph();
				b_entry graph_entry;
				if (graph->find(s->out->name) != graph->end()) {
					graph_entry = (*graph)[s->out->name];
					//if I am a sink, just calculate and return
					if(graph_entry->b_type == t_sink || graph_entry->b_type == t_merge_sink) {
						if (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT) {
							path_capacity = (double)stat->total_input.value/stat->total_p_time.value;
							path_selectivity = (double)stat->total_output.value/stat->total_input.value;
						} else {
							path_capacity = stat->last_known_process_rate;
							path_selectivity = stat->last_known_selectivity;
						}
						priority = calculatePriority(path_capacity, path_selectivity, stat);
					} else {
						//there are statements downstream of me.
						stmt* s2 = NULL;
						double max_branch_priority = 0;
						if (graph_entry->b_type == t_fork || graph_entry->b_type == t_source_fork) {
							//there are multiple immediate downstream statements, choose the highest one.
							int nFork = graph_entry->forward.fork_count;
							//now find out the dummy states, the # should agree with nFork
							int cnt = 0;
							list<b_entry> dummies;
							for (BufStateTblType::iterator itr3 = graph->begin(); itr3 != graph->end(); itr3++) {
								if (itr3->second->b_type != t_fork_dummy) continue;
								if (strcmp(itr3->second->inplace, s->out->name) == 0) {
									dummies.push_back(itr3->second);
									cnt++;
								}    
							}

							if (nFork == cnt) {
								bool available = true;	 
								double branch_priority;
								for(list<b_entry>::iterator itr4 = dummies.begin(); itr4 != dummies.end() && available; itr4++) {
									stmt* s3 = (*itr4)->statement;
									branch_priority = updatePriority(s3);	  

									//check again after update, to see if info is available
									if (branch_priority == PriorityStrategy::MAX_PRIORITY) {
										available = false;
										s2 = NULL;
										break;
									} else if (s2 = NULL) {
										s2 = s3;
										max_branch_priority = branch_priority;
									} else {
										//make sure s2 is set to the branch with the highest priority
										if(max_branch_priority < branch_priority) max_branch_priority = branch_priority;
									}
								}
							}
						} else {
							//there is only one immediate downstream statement
							s2 = graph_entry->statement;
							max_branch_priority = updatePriority(s2);
						}

						if (s2 != NULL && max_branch_priority != PriorityStrategy::MAX_PRIORITY) {
							//downstream stats all exists and have priority values, I can update my own now.
							if (querySchdl::verbose) cout << "update self-value after getting downstream stmt priority: " << s->name << endl;
							ocs_quota_extra_entry* entry2 = NULL;
							if(extraInfo->find(s2->name) != extraInfo->end()) {
								entry2 = (ocs_quota_extra_entry*) ((*extraInfo)[s2->name]);
								double op_capacity, op_selectivity;
								if (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT) {
									op_capacity = (double)stat->total_input.value/stat->total_p_time.value;
									op_selectivity = (double)stat->total_output.value/stat->total_input.value;
								} else {
									op_capacity = stat->last_known_process_rate;
									op_selectivity = stat->last_known_selectivity;
								}
								//if (querySchdl::verbose) {
								//  cout << "calc priority. op_capacity is " << op_capacity << ", op_selectivity is " << op_selectivity << endl;
								//}
								path_selectivity = op_selectivity*entry2->path_selectivity;
								path_capacity = (1/op_selectivity)/((1/(op_capacity*op_selectivity))+(1/entry2->path_capacity));
								priority = calculatePriority(path_capacity, path_selectivity, stat);
							}
						}
					}
				}

				entry->path_capacity = path_capacity;
				entry->path_selectivity = path_selectivity;
				entry->rank.priority = priority;
				s->rankSet = true;

				return priority;
			}

			static bool strat_test_switch = true;

			extern bool monitor_exp_mode;
			static long segQuota_roundCnt = 0;
			//static int round_measure_interval = 100;
			static bool ever_has_input_in_exp = false; 

			//overwrites default implementation of parent, mainly to hanlde quota carry-overs as describe below.
			//execution goes from order of high_ocs_priority to low_ocs_priority, due to the sort order of the list. 
			//the first operator get all of the TOP_QUOTA, everyone else gets 1 tuple as quota. 
			//whatever is left from finishing the existing tuples for the first stmt is carried over to the second, and so on, until anytime quota is used up, then the remainder each process 1 tuple. The idea is to balance priority with fairness a little bit.
			run_rc OcsQuotaStrategy::runNextUnit(int* nTuples) {

				//if (querySchdl::verbose) {
				//cout << "enter run segment quota strategy " << endl;
				//}

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();

				bool output = false;
				//bool input = false;
				run_rc rc = run_success;
				stmt* s;
				list<quota_entry*>::iterator itr;
				if (&(this->last_itr) != NULL) itr= this->last_itr;
				else itr = stmtsInfo->begin();

				this->drv->stateTableChanged = false;
				for (; itr != stmtsInfo->end() && (*nTuples) > 0; itr++) {
					if (!(dm->stmtInUse((*itr)->stmt_name))) {
						//precaution, in case invalid statements has not been removed from driver.
						//this actually should not happen.
						if (querySchdl::verbose) cout << "reach invalid entries on segment quota strategy stmt list."  << endl;
						continue;
					}

					s = dm->getStmtByName((*itr)->stmt_name);
					if (s == NULL || !(s->valid)) {
						if (querySchdl::verbose) {
							cout << "get invalid statement in ocs quota strategy." << endl;
						}
						continue; //this really should not have happened
					}

					if (querySchdl::verbose && s->type == stmt_normal && !s->in->empty()) {
						cout << "try to execute stmt " << s->name << " for quota left: " << (*itr)->quota_left << endl;
						strat_test_switch = false;
					}

					if (querySchdl::verbose && s->type == stmt_normal && s->in->empty() && !strat_test_switch) {
						cout << "stmt buffer empty " << s->name << " for quota left: " << (*itr)->quota_left << endl;
						strat_test_switch = true;
					}

					/* move to run_state so that it can be done for every statement, instead of only the sources here.
					//first see whether this statement needs updating its quota, for now ignore quota changes based on feedback. This only takes care of changes due to initial stat availability
					if (s->rank.quota == this->default_quota && !s->rankSet && !this->fixed_quota && isUpdateTrigger(s)) {
					stmt_entry* stat = monitor->getStmtStat(s);
					if(stat!= NULL && (stat->total_input.value >= NUM_INPUT_BEFORE_SET_STAT || (stat->last_known_selectivity > 0 && stat->last_known_process_rate > 0))) {
					if (querySchdl::verbose) {
					cout << "at least " << NUM_INPUT_BEFORE_SET_STAT << " inputs processed, update quota for statement " << s->name << endl;
					}
					update_all();
					this->last_itr = stmtsInfo->begin();
					return run_success; //need to return since underlying list<> changed
					}
					}
					 */

					if (monitor_exp_mode && ever_has_input_in_exp && s->monitor_src_stat && s->in->bufSize() > 0 /*&& (segQuota_roundCnt % round_measure_interval) == 0*/) {
						//output source buffer sizes every once in a while, under exp mode only, and only after there has been input
						//cout << "in round " << segQuota_roundCnt << ", stmt " << (*itr)->stmt_name << " has initial input buffer size " << s->in->bufSize() << endl;
					}

					rc = run_success;
					int start_num = *nTuples;
					while ((*nTuples) > 0 && (*itr)->quota_left > 0 && rc == run_success) {
						//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
						rc = runState((*segmentMap)[s->in->name], nTuples, &output);
						//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
						if (this->drv->stateTableChanged) {
							//break; //leave immediately after underlying table change.
							//this->drv->stateTableChanged = false;
							return run_success;
						}
					}

					if (!ever_has_input_in_exp && *nTuples < start_num) ever_has_input_in_exp = true;

					if (this->drv->stateTableChanged) {
						//this->drv->stateTableChanged = false;
						//break; //leave immediately after underlying table change.
						return run_success;
					}

					/*---------ocsQuota specific code begin ----------*/
					//carry my quota over, if there is anything left.
					if ((*nTuples) > 0 && (*itr)->quota_left > 0) {
						long overflow = (*itr)->quota_left;
						//if (querySchdl::verbose) {
						//cout << "overflow of quota in OcsQuotaStrategy after trying " << s->name << ", " << overflow << " carried over" << endl;
						//}
						(*itr)->quota_left = 0;
						itr++;
						if (itr != stmtsInfo->end()) (*itr)->quota_left += overflow;
						itr--;
						//cout << "itr_last is now referring to " << (*itr)->stmt_name << endl;
					}

					this->last_itr = itr;

					/*---------ocsQuota specific code end ----------*/

					if (/*(*itr)->quota_left <= 0 &&*/ (*nTuples) > 0 && strcmp(stmtsInfo->back()->stmt_name, s->name) == 0) {
						//just finished the last one in the full round, it is time to reset quota_left for everyone.
						for (list<quota_entry*>::iterator itr2 = stmtsInfo->begin(); itr2 != stmtsInfo->end(); itr2++) {
							(*itr2)->quota_left = (*itr2)->quota;
						}
						this->last_itr = stmtsInfo->begin();

						if (monitor_exp_mode && ever_has_input_in_exp) {
							//under exp mode only, and only after there has been input
							segQuota_roundCnt++;
						}
					}

					//if ((*nTuples) <= 0) {
					//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
					//}
					if (querySchdl::verbose && output) {
						//there are output tuples, print debug info for monitor stats
						//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << *nTuples << endl;
						//cout << "new output tuple, print stats and strategy quota state during segment walking." << endl; 
						//monitor->printStateTables();
						//QuotaStrategy::printStateTable();
					}

				} //end for (segments)

				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
					//monitor->printStateTables();
					//QuotaStrategy::printStateTable();
					//printStateTable();
				}

				//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
				return rc;
			}

			//long OcsRouletteSegStrategy::NEW_RANK_ROULETTE = 5;

			OcsRouletteSegStrategy::OcsRouletteSegStrategy(Driver* drv, long default_quota):OcsQuotaStrategy(drv, default_quota)
			{
				name = "OcsRoulette";
				//prepare for random number generation
				srand ( time(NULL) );
			}

			OcsRouletteSegStrategy::~OcsRouletteSegStrategy()
			{
			}

			//simplified from parent
			void OcsRouletteSegStrategy::printStateTable()
			{
				cout << "current strategy has " << stmtsInfo->size() << " stmts, the order of stmts is (format stmt, priority): " << endl;

				for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
					cout << "(" << (*itr)->stmt_name << "," << (*itr)->rank.priority << ") ";
				}

				cout << endl << endl;

				cout << "current extra info in strategy (stmt_name, priority, path_capacity, path_selectivity): " << endl;

				for (ExtraInfoMapType::iterator itr = extraInfo->begin(); itr != extraInfo->end(); itr++) {
					ocs_quota_extra_entry* ety = (ocs_quota_extra_entry*) itr->second;
					cout << "(" << itr->first << "," << ety->rank.priority << "," << ety->path_capacity << "," << ety->path_selectivity << ") ";
				}

				cout << endl << endl;

				cout << "rankSet flags in stmts:" << endl;

				for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
					cout << "(" << (*itr)->name << ",rankSet " << ((*itr)->rankSet?"true)":"false)");
				}

				cout << endl << endl;  

				drv->printStateTable(this->segmentMap);
				return;
			}

			//this one uses a roulette machanism to select next one to process.
			//quota and quota_left are not used, only priority information is used.
			run_rc OcsRouletteSegStrategy::runNextUnit(int* nTuples) {

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();

				//build list of stmts to run roulette on. in this segmented case, only the stmts that have source buffers.
				list<quota_entry*> pool;
				long total_rank = 0;
				long avg_rank = 0;
				//if (querySchdl::verbose) cout << "mapping for roulette: ";
				//we know the stmtsInfo list is sorted by priority, so iterate from end, to get all the set priorities first.
				for (list<quota_entry*>::reverse_iterator itr = stmtsInfo->rbegin(); itr != stmtsInfo->rend(); itr++) {
					//reset this to high value to prevent quota from having any effect. not very efficient, but ok for now.
					(*itr)->quota = TOP_QUOTA;
					(*itr)->quota_left = TOP_QUOTA;

					if ((*itr)->rank.priority != PriorityStrategy::MAX_PRIORITY) {
						total_rank += (long)((*itr)->rank.priority);
						//if (querySchdl::verbose) cout << "(" << (*itr)->stmt_name << "," << (long)((*itr)->rank.priority) << ") ";
					} else {
						if (avg_rank == 0 && total_rank > 0) {
							avg_rank = total_rank/pool.size();
						} else if (avg_rank == 0) {
							avg_rank = 1;
						}
						total_rank += avg_rank;
						//if (querySchdl::verbose) cout << "(" << (*itr)->stmt_name << "," << avg_rank << ") ";
					}
					pool.push_back((*itr));
				}
				//if (querySchdl::verbose) cout << endl << endl;

				bool output = false;
				//bool input = false;
				run_rc rc = run_success;
				stmt* s;
				bool leave = false;
				while ((*nTuples) > 0 && pool.size() > 0 && !leave /*rc == run_success*/) {
					//find the next stmt to run.
					s = roulette(&pool, &total_rank, &avg_rank);

					if (s == NULL) {
						continue; //this happens when the input buffer of the chosen stmt is empty
					}

					if (querySchdl::verbose) cout << "roulette fetch stmt " << s->name << ", nTuples value " << *nTuples << ", pool size " << pool.size() << endl;  
					int count1 = 1;
					rc = runState((*segmentMap)[s->in->name], &count1, &output);
					if (count1 < 1) {
						(*nTuples)--;
					}
					if (querySchdl::verbose) cout << "return code in run segment is " << rc << ", nTuples value " << *nTuples << ", pool size " << pool.size() << endl;

					if (rc == run_no_input) {
						for (list<quota_entry*>::iterator itr = pool.begin(); itr != pool.end(); itr++) {
							if (strcmp((*itr)->stmt_name, s->name) == 0) {
								long cur_rank;
								if ((*itr)->rank.priority == PriorityStrategy::MAX_PRIORITY) {
									cur_rank = avg_rank;
								} else {
									cur_rank = (long)((*itr)->rank.priority);
								}

								total_rank -= cur_rank;
								pool.erase(itr);
								break;
							}
						}
					}

					if (this->drv->stateTableChanged) {
						//this->drv->stateTableChanged = false;
						leave = true;
						//break; //leave immediately after underlying table change.
						return run_success;
					}
				} //end while

				if ((*nTuples) <= 0) {
					if (querySchdl::verbose) cout << "finished one driver round." << endl;
				}

				//if (pool.size() == 0) {
				//  if (querySchdl::verbose) cout << "all buffers empty, finish one round." << endl;
				//}

				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << *nTuples << endl;
					//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
					//monitor->printStateTables();
					//printStateTable();
				}

				//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
				return rc;
			}

			//actually throw the dice. Also maintain the pool of stmts, remove those do not have input.
			stmt* OcsRouletteSegStrategy::roulette(list<quota_entry*>* pool, long* total_rank, long* avg_rank) {
				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();

				long next = (long)(((double)rand()/RAND_MAX)*(*total_rank));
				long cur_total = 0;
				long cur_rank;
				for (list<quota_entry*>::iterator itr = pool->begin(); itr != pool->end(); itr++) {
					if ((*itr)->rank.priority == PriorityStrategy::MAX_PRIORITY) {
						cur_rank = *avg_rank;
					} else {
						cur_rank = (long)((*itr)->rank.priority);
					}

					cur_total += cur_rank;

					if (cur_total >= next) {
						stmt* s = dm->getStmtByName((*itr)->stmt_name);
						if (s == NULL) {
							*total_rank -= cur_rank;
							pool->erase(itr);  //we know list is fail-safe
							return NULL;
						} else if (!(s->valid) && querySchdl::verbose) {
							cout << "get invalid statement in segment ocs roulette strategy." << endl;
							*total_rank -= cur_rank;
							pool->erase(itr);  //we know list is fail-safe
							return NULL;
						}

						//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
						if (s->in->empty()) {
							if (monitor->total_empty_buffer_hit.on) {
								monitor->total_empty_buffer_hit.value++;
							}

							monitor->updateLastInTurn(s);
							//if strategy is changed, this flag is set, return immediately
							if (this->drv->stateTableChanged) {
								if (querySchdl::verbose) cout << "strategy Changed 11" << endl;
								return NULL; //leave immediately after underlying table change.
							}

							*total_rank -= cur_rank;
							pool->erase(itr);  //we know list is fail-safe
							return NULL;
						} else {
							if (querySchdl::verbose) cout << "roulette total rank: " << *total_rank << ", avg_rank " << *avg_rank 
								<< ", spin result " << next << ", hit range " << (cur_total - cur_rank) << "-" << cur_total << endl;  
							return s;
						}
					}
				}

				cout << "roulette total rank: " << *total_rank << ", avg_rank " << *avg_rank 
					<< ", spin result " << next << ", hit number " << cur_total << endl;  

				cerr << "roulette rand number lies outside of appropriate range, internall error. " << endl;

				return NULL;
			}

			OcsSegStrategy::OcsSegStrategy(Driver* drv, long default_quota):OcsQuotaStrategy(drv, default_quota)
			{
				name = "OcsSeg";
			}

			OcsSegStrategy::~OcsSegStrategy()
			{
			}

			//simplified from parent
			void OcsSegStrategy::printStateTable()
			{
				cout << "current strategy has " << stmtsInfo->size() << " stmts, the order of stmts is (format stmt, priority): " << endl;

				for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end(); itr++) {
					cout << "(" << (*itr)->stmt_name << "," << (*itr)->rank.priority << ") ";
				}

				cout << endl << endl;

				cout << "current extra info in strategy (stmt_name, priority, path_capacity, path_selectivity): " << endl;

				for (ExtraInfoMapType::iterator itr = extraInfo->begin(); itr != extraInfo->end(); itr++) {
					ocs_quota_extra_entry* ety = (ocs_quota_extra_entry*) itr->second;
					cout << "(" << itr->first << "," << ety->rank.priority << "," << ety->path_capacity << "," << ety->path_selectivity << ") ";
				}

				cout << endl << endl;

				cout << "rankSet flags in stmts:" << endl;

				for (list<stmt*>::iterator itr = drv->getStmts()->begin(); itr != drv->getStmts()->end(); itr++) {
					cout << "(" << (*itr)->name << ",rankSet " << ((*itr)->rankSet?"true)":"false)");
				}

				cout << endl << endl;  

				drv->printStateTable(this->segmentMap);
				return;
			}

			//static bool testocssegstart = false;

			//this one always go to the highest priority path, whenever a new tuple arrives.
			//quota and quota_left are not used, only priority information is used.
			run_rc OcsSegStrategy::runNextUnit(int* nTuples) {

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();

				bufferMngr* bm = bufferMngr::getInstance();

				bm->new_arrival = 0;    
				bool first_tuple = true; 

				bool output = false;
				bool leave = false;

				run_rc rc = run_success;
				stmt* s;

				this->drv->stateTableChanged = false;

				//if (testocssegstart) cout << 1 << endl;

				int beginCount = *nTuples;
				for (list<quota_entry*>::iterator itr = stmtsInfo->begin(); itr != stmtsInfo->end() && (*nTuples) > 0  && (!(bm->new_arrival) || first_tuple) && !leave; itr++) {

					//reset this to high value to prevent quota from having any effect. not very efficient, but ok for now.
					(*itr)->quota = TOP_QUOTA;
					(*itr)->quota_left = TOP_QUOTA;

					if (!(dm->stmtInUse((*itr)->stmt_name))) {
						//precaution, in case invalid statements has not been removed from driver.
						//this actually should not happen.
						if (querySchdl::verbose) cout << "reach invalid entries on segment quota strategy stmt list."  << endl;
						continue;
					}

					s = dm->getStmtByName((*itr)->stmt_name);
					if (s == NULL || !(s->valid)) {
						if (querySchdl::verbose) {
							cout << "get invalid statement in ocs quota strategy." << endl;
						}
						continue; //this really should not have happened
					}

					if (s->in->empty()) {
						if (monitor->total_empty_buffer_hit.on) {
							monitor->total_empty_buffer_hit.value++;
						}

						monitor->updateLastInTurn(s);
						//if strategy is changed, this flag is set, return immediately
						if (this->drv->stateTableChanged) {
							//this->drv->stateTableChanged = false;
							if (querySchdl::verbose) cout << "strategy Changed 12" << endl;
							return run_success; //leave immediately after underlying table change.
						}

						continue;
					}

					rc = run_success;
					//if (testocssegstart) cout << 2 << endl;

					while ((*nTuples) > 0 && rc == run_success && (!(bm->new_arrival) || first_tuple) && !leave) {
						//if (testocssegstart) cout << "first tuple is " << (first_tuple?"true":"false") << ", new arrival is " << (bm->new_arrival?"true":"false") << ", nTuples is " << *nTuples << ", stmt is " << s->name << endl;

						//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
						int count1 = 1; 
						rc = runState((*segmentMap)[s->in->name], &count1, &output);
						if (count1 < 1) {
							(*nTuples)--;
						} else {
							break;
						}
						if (*nTuples < beginCount) {
							first_tuple = false;
							//testocssegstart = true;
						}
						//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;
						if (this->drv->stateTableChanged) {
							leave = true;
							//break; //leave immediately after underlying table change.
							//this->drv->stateTableChanged = false;
							return run_success;
						}
					}

					if (this->drv->stateTableChanged) {
						//if (testocssegstart) cout << 3 << endl;
						//this->drv->stateTableChanged = false;
						leave = true;
						//break; //leave immediately after underlying table change.
						return run_success;
					}

					//if ((*nTuples) <= 0) {
					//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
					//}

					if (querySchdl::verbose && output) {
						//there are output tuples, print debug info for monitor stats
						//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << *nTuples << endl;
					}

				} //end for (segments)

				if (bm->new_arrival && querySchdl::verbose) {
					cout << "new arrival, return back to front of queue" << endl;
				}

				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
					//monitor->printStateTables();
					//printStateTable();
				}

				//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
				return rc;

			}

			OcsSegNoOptmztnStrategy::OcsSegNoOptmztnStrategy(Driver* drv):OcsSegStrategy(drv)
			{
				name = "OcsNoOptSeg";
			}

			OcsSegNoOptmztnStrategy::~OcsSegNoOptmztnStrategy()
			{
			}

			//this one always go to the highest priority path, after processing every tuple
			//quota and quota_left are not used, only priority information is used.
			run_rc OcsSegNoOptmztnStrategy::runNextUnit(int* nTuples) {

				DrvMgr* dm = DrvMgr::getInstance();
				Monitor* monitor = dm->getMonitor();

				bufferMngr* bm = bufferMngr::getInstance();

				bool first_tuple = true; 
				bool output = false;
				bool leave = false;

				run_rc rc = run_success;
				stmt* s;

				this->drv->stateTableChanged = false;

				int beginCount = *nTuples;
				while ((*nTuples) > 0 && !leave) {
					first_tuple = true; 

					list<quota_entry*>::iterator itr;
					for (itr = stmtsInfo->begin(); itr != stmtsInfo->end() && (*nTuples) > 0  && (first_tuple) && !this->drv->stateTableChanged; itr++) {

						//reset this to high value to prevent quota from having any effect. not very efficient, but ok for now.
						(*itr)->quota = TOP_QUOTA;
						(*itr)->quota_left = TOP_QUOTA;

						if (!(dm->stmtInUse((*itr)->stmt_name))) {
							//precaution, in case invalid statements has not been removed from driver.
							//this actually should not happen.
							if (querySchdl::verbose) cout << "reach invalid entries on segment quota strategy stmt list."  << endl;
							continue;
						}

						s = dm->getStmtByName((*itr)->stmt_name);
						if (s == NULL || !(s->valid)) {
							if (querySchdl::verbose) {
								cout << "get invalid statement in ocs quota strategy." << endl;
							}
							continue; //this really should not have happened
						}

						if (s->in->empty()) {
							if (monitor->total_empty_buffer_hit.on) {
								monitor->total_empty_buffer_hit.value++;
							}

							monitor->updateLastInTurn(s);
							//if strategy is changed, this flag is set, return immediately
							if (this->drv->stateTableChanged) {
								//this->drv->stateTableChanged = false;
								if (querySchdl::verbose) cout << "strategy Changed 13" << endl;
								return run_success; //leave immediately after underlying table change.
							}

							continue;
						}

						rc = run_success;

						//currently, at source we can not have union statements (which is compiled to have normal statements at source). So we can simply use the "in" buffer state.
						int count1 = 1; 
						rc = runState((*segmentMap)[s->in->name], &count1, &output);
						if (count1 < 1) {
							(*nTuples)--;
							first_tuple = false;
						}

						//if (querySchdl::verbose) cout << "return code in run segment is " << rc << endl;

						//if ((*nTuples) <= 0) {
						//  if (querySchdl::verbose) cout << "finished one driver round." << endl;
						//}

						if (querySchdl::verbose && output) {
							//there are output tuples, print debug info for monitor stats
							//cout << "finished trying statement " << s->name << ", return code is " << rc << ", tuple left for driver round is " << *nTuples << endl;
						}

					} //end for (segments)

					if (this->drv->stateTableChanged) {
						//this->drv->stateTableChanged = false;
						return run_success;
					}

					if (itr == stmtsInfo->end()) {
						leave = true;
						break;
					}

				}//end while

				if (querySchdl::verbose && output) {
					//there are output tuples, print debug info for monitor stats
					//cout << "new output tuple, print stats and strategy quota state after finished segment walking" << endl;
					//monitor->printStateTables();
					//printStateTable();
				}

				//if (querySchdl::verbose) cout << "return code at the end of runNextUnit is " << rc << endl;  
				return rc;
			}

			PcsSegStrategy::PcsSegStrategy(Driver* drv):OcsSegStrategy(drv)
			{
				name = "PcsSeg";
			}

			PcsSegStrategy::~PcsSegStrategy()
			{
			}

			double PcsSegStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity;
			}

			TrainSegStrategy::TrainSegStrategy(Driver* drv):OcsSegStrategy(drv)
			{
				name = "TrainSeg";
			}

			TrainSegStrategy::~TrainSegStrategy()
			{
			}

			double TrainSegStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity*(double)stat->input_tuple_size;
			}

			TrainQuotaStrategy::TrainQuotaStrategy(Driver* drv):OcsQuotaStrategy(drv)
			{
				name = "TrainSwpQCx";
			}

			TrainQuotaStrategy::~TrainQuotaStrategy()
			{
			}

			double TrainQuotaStrategy::calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat) {
				return path_capacity*(double)stat->input_tuple_size;
			}

		} // end of namespace ESL
