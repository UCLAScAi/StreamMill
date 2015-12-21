#include "monitor.h"
#include "adllib.h"
//#include "basic.h"
#include "const.h"
#include "util.h"
#include "querySchdl.h"
#include "ios/ios.h"
#include "strategy.h"
#include <fstream>
extern "C"{
#include <dbug.h>
}

namespace ESL {

	const int Monitor::WINDOW_SIZE_SEC = 1;

	//for now make this 1, and while running strategy multiply with actual utilization value (or use 0.5, if the actual value is too low due to low load)
	double Monitor::EFFECTIVE_CPU_UTILIZATION = 1;

	//bool strat_changed = false; //for testing only

	//flag set when doing scheduling experiments only
	bool monitor_exp_mode = false;
	bool exp_batch_activate_mode = false;

	//these are for exp only too
	int activated = 0;
	int num_queries = 9;

	Monitor::Monitor()
	{

		querySchdl::SMLOG(10, "Entering Monitor::Monitor");	
		
		collectStats = false;

		stmtStats = new StmtStatMapType();
		inputStats = new InputStatMapType();
		outputStats = new OutputStatMapType();

		struct timezone tz;
		gettimeofday(&start_time, &tz);
		win_start_sec = 0;
		total_empty_buffer_hit.on = false; //not counting unless we are doing experiments and there are already tuples coming in.
		win_cumul_total_buf_bytes = 0;
		win_cumul_total_source_buf_counts = 0;
		win_buf_size_meas_cnt = 0;
		win_input_processed_cnt = 0;
		last_win_total_p_time = 0;
		newWindow = false;
	}

	Monitor::~Monitor()
	{  
		querySchdl::SMLOG(10, "Entering Monitor::~Monitor");	
		for (StmtStatMapType::iterator itr = stmtStats->begin(); itr != stmtStats->end(); ++itr) {
			//delete itr->first; //shared with out map, delete once only, to be done later
			delete itr->second;
		}
		delete stmtStats;

		for (InputStatMapType::iterator itr = inputStats->begin(); itr != inputStats->end();) {
			char* s = itr->first; 
			delete itr->second;
			itr++;
			delete s;
			//delete itr->first;  //can not do this since hash_map is not fail-safe
			//delete itr->second;
		}
		delete inputStats;

		for (OutputStatMapType::iterator itr = outputStats->begin(); itr != outputStats->end();) {
			char* s = itr->first; 
			delete itr->second;
			itr++;
			delete s;
		}
		delete outputStats;

		for (set<perf_stat_info*, ltsspair>::iterator itr = stat_to_send_bufs.begin(); itr != stat_to_send_bufs.end(); ++itr) {
			delete (*itr);
		}  
	}

	stmt_entry* Monitor::getStmtStat(stmt* s) {
		if (stmtStats->find(s->name) == stmtStats->end()) {
			return NULL;
		} else {
			return (*stmtStats)[s->name];
		}
	}

	//static const char* monitor_exp_outloc = "./outStat.log";
	static const char* monitor_exp_numloc = "./outNums.log";
	//static ofstream outStatFile(monitor_exp_outloc, ios_base::app);
	static ofstream outNumsFile(monitor_exp_numloc, ios_base::app);
	static int mon_exp_start_sec = 0;

	static bool aggregated_throughput_testing = false;
	static int aggregated_throughput_counter = 0;
	static timeval aggregated_throughput_starttime;
	static timeval aggregated_throughput_endtime;
	static const int aggregated_throughput_desired_total = 15000;

	stmt_rc Monitor::exeStmt(stmt* s, Driver* drv) {
		querySchdl::SMLOG(10, "Entering Monitor::exeStmt");	
		if (s == NULL || !(s->valid)) return s_failure; //this should not have happened.

		if (querySchdl::verbose) cout << "entering monitor->exestmt()" << endl;

		if (newWindow) {
			newWindow = false;
		}

		if (s->type == stmt_normal || s->type == stmt_join) {
			if (s->in->empty()) {
				if (s->type == stmt_join) {
					jStmt* s2 = (jStmt*) s;
					s2->back_buf = s2->in;
				}

				//struct timeval tv_exit;
				//gettimeofday(&tv_exit, &tz);
				//double diff_exit = timeval_subtract(tv_exit, tv);
				//if (total_processing_time.on) {
				//total_processing_time.value += diff_exit;
				//}

				if (collectStats) {      
					if (total_empty_buffer_hit.on) {
						total_empty_buffer_hit.value++;
					}
					updateLastInTurn(s);
					if (querySchdl::verbose && drv->stateTableChanged) cout << 1 << " ";
				}

				return s_no_input;
			}
		}

		if (!collectStats) {
			if (aggregated_throughput_testing && aggregated_throughput_counter == 0) {
				struct timezone tz;
				gettimeofday(&aggregated_throughput_starttime, &tz);
			}

			// this is used in modes when stats should not be collected, 
			// such as when we need to avoid overhead when testing certain performances
			int rc = (int) (s->exe());

			if (aggregated_throughput_testing) {
				aggregated_throughput_counter++;
				//if (aggregated_throughput_counter % 1000 == 0) {
				//cout << "processed 1000 tuples" << endl;
				//}

				if (aggregated_throughput_counter == aggregated_throughput_desired_total) {
					struct timezone tz;
					gettimeofday(&aggregated_throughput_endtime, &tz);
					double totalDiff = timeval_subtract(aggregated_throughput_endtime, 
										aggregated_throughput_starttime);
					cout << "finished processing " << aggregated_throughput_desired_total 
						<< " tuples." << endl;
					cout << "total processing time is " << totalDiff << " seconds." << endl;
				}
			}

			return (stmt_rc)rc;
		}

		//from here on stats will be collected for statements and overall      

		updateLastInTurn(s);
		if (querySchdl::verbose && drv->stateTableChanged) cout << 2 << " ";

		struct timeval tv_begin;
		struct timezone tz;
		gettimeofday(&tv_begin, &tz);

		stmt_entry* s_entry = NULL;
		output_entry* o_entry = NULL;
		input_entry* i_entry = NULL;

		if (stmtStats->find(s->name) != stmtStats->end()) {
			s_entry = (*stmtStats)[s->name];
		}

		if (s->monitor_sink_stat && outputStats->find(s->name) != outputStats->end()) {
			o_entry = (*outputStats)[s->name];
		}

		if (s->monitor_src_stat && inputStats->find(s->in->name) != inputStats->end()) {
			i_entry = (*inputStats)[s->in->name];
		}

		if(i_entry != NULL && i_entry->start_time.on) {
			if(i_entry->start_time.value.tv_sec == 0) {
				i_entry->start_time.value.tv_sec = tv_begin.tv_sec;
				i_entry->start_time.value.tv_usec = tv_begin.tv_usec;
			}
		}
		if(s_entry != NULL && s_entry->start_time.on) {
			if(s_entry->start_time.value.tv_sec == 0) {
				s_entry->start_time.value.tv_sec = tv_begin.tv_sec;
				s_entry->start_time.value.tv_usec = tv_begin.tv_usec;
			}
		}

		//struct timeval tv;
		//gettimeofday(&tv, &tz);

		struct timeval tv;
		gettimeofday(&tv, &tz);

		int rc = (int) (s->exe());

		//well, recursive UDA did not work, neither did UDFs, have to use this way for now, to make slow operators.
		//slow down the first operator on each path
		if (monitor_exp_mode) {
			//medium processing time
			if (strcmp(s->name, "a131_179_64_67_982067") == 0 || strcmp(s->name, "a131_179_64_67_984652") == 0 || strcmp(s->name, "a131_179_64_67_432364") == 0) {
				int testval = 65635;
				for (int l = 0; l < 50000; l++) {
					testval*testval/(testval-4);
				}
			} else if (strcmp(s->name, "a131_179_64_67_852489") == 0 || strcmp(s->name, "a131_179_64_67_835152") == 0 || strcmp(s->name, "a131_179_64_67_384858") == 0 || strcmp(s->name, "a131_179_64_67_91059") == 0) {
				//long processing time
				int testval = 65635;
				for (int l = 0; l < 500000; l++) {
					testval*testval/(testval-4);
				}
			}
		}

		//the pre-occupied queue batch experiments, use slow operator to make the mem requirement smaller
		if (monitor_exp_mode && exp_batch_activate_mode) {
			//currently for machine stream.cs
			if (strcmp(s->name, "a131_179_64_67_543236") == 0 || strcmp(s->name, "a131_179_64_67_992941") == 0 || strcmp(s->name, "a131_179_64_67_544728") == 0 || strcmp(s->name, "a131_179_64_67_994513") == 0 || strcmp(s->name, "a131_179_64_67_544306") == 0 || strcmp(s->name, "a131_179_64_67_992092") == 0 || strcmp(s->name, "a131_179_64_67_541875") == 0 || strcmp(s->name, "a131_179_64_67_989667") == 0 || strcmp(s->name, "a131_179_64_67_563454") == 0) {
				//long processing time
				int testval = 65635;
				for (int l = 0; l < 500000; l++) {
					testval*testval/(testval-4);
				}
			}
		}

		//deadline numbers temporarily set here, for now it is tcp9 (tcp 10 is the same as tcp9 without deadline)
		//for testing stmts on machine nile
		if (strcmp(s->name, "a131_179_64_67_384858") == 0) {
			if (s->deadline == 0) {
				s->deadline = 1;
				if (drv->getStrategy(true) != NULL) drv->getStrategy(true)->getWeightProfile()->setStmtWithDeadline();
			}
		} else if (strcmp(s->name, "a131_179_64_67_852489") == 0) {
			if (s->deadline == 0) s->deadline = 1;
		} else if (strcmp(s->name, "a131_179_64_67_984652") == 0) {
			if (s->deadline == 0) s->deadline = 1;
		}

		//for testing stmts on machine stream
		/*
		   if (strcmp(s->name, "a131_179_64_67_563454") == 0) {
		   if (s->deadline == 0) {
		   s->deadline = 1;
		   if (drv->getStrategy(true) != NULL) drv->getStrategy(true)->getWeightProfile()->setStmtWithDeadline();
		   }
		   } else if (strcmp(s->name, "a131_179_64_67_541875") == 0) {
		   if (s->deadline == 0) s->deadline = 1;
		   } else if (strcmp(s->name, "a131_179_64_67_544306") == 0) {
		   if (s->deadline == 0) s->deadline = 1;
		   }
		 */

		struct timeval tv2;
		gettimeofday(&tv2, &tz);

		//this is the processing time of the stmt
		double diff = timeval_subtract(tv2, tv);

		if (diff > 0.0001 && strcmp(s->name, "a131_179_64_67_982067") != 0 && strcmp(s->name, "a131_179_64_67_984652") != 0
				&& strcmp(s->name, "a131_179_64_67_432364") != 0 && strcmp(s->name, "a131_179_64_67_852489") != 0
				&& strcmp(s->name, "a131_179_64_67_835152") != 0 && strcmp(s->name, "a131_179_64_67_384858") != 0
				&& strcmp(s->name, "a131_179_64_67_91059") != 0 && strcmp(s->name, "a131_179_64_67_543236") != 0
				&& strcmp(s->name, "a131_179_64_67_992941") != 0 && strcmp(s->name, "a131_179_64_67_544728") != 0
				&& strcmp(s->name, "a131_179_64_67_994513") != 0 && strcmp(s->name, "a131_179_64_67_544306") != 0
				&& strcmp(s->name, "a131_179_64_67_992092") != 0 && strcmp(s->name, "a131_179_64_67_541875") != 0
				&& strcmp(s->name, "a131_179_64_67_989667") != 0 && strcmp(s->name, "a131_179_64_67_563454") != 0) {
			cout << "outlier proc time for " << s->name << " is " << diff << endl;
			diff = 0.000005;
		}

		if(s_entry != NULL) {
			s_entry->last_in_turn.value.tv_sec = tv2.tv_sec;
			s_entry->last_in_turn.value.tv_usec = tv2.tv_usec;
		}

		if (rc == s_failure) {
			return (stmt_rc) rc;
		} 

		//check arrival time of the tuple used. For union stmt, s->in is set to the buffer used, for others, s->in is naturally the buffer used.
		struct timeval tv_arrival;
		s->in->get(&tv_arrival);

		//this is the raw value for latency
		double diff3 = timeval_subtract(tv2, tv_arrival);
		//if (diff3 > 1) {
		//  cout << "long latency for " << s->name << " is " << diff3 << endl;
		//}

		if (diff3 > 100) {
			cout << "stmt is " << s->name << ", tv2 is " << (tv2.tv_sec + (double)tv2.tv_usec/1000000) << ", tv_arrival is " << (tv_arrival.tv_sec + (double)tv_arrival.tv_usec/1000000) << ", latency is " << diff3 << endl;
		}

		bool arrival_abnormal = false; //temporarily use to account for bug in arrival time. 
		if (tv_arrival.tv_sec < this->start_time.tv_sec) {
			arrival_abnormal = true;
		}

		if (arrival_abnormal) {
			cout << "arrival timestamp abnormal, tuple skipped for monitoring in stmt " << s->name << endl;
		}

		if ((rc == s_no_output || rc > 0) && !arrival_abnormal) {

			if (!total_empty_buffer_hit.on && monitor_exp_mode) {
				total_empty_buffer_hit.on = true; //turn this sensor on only after we receive input.
			}

			if (querySchdl::verbose && (s->type == stmt_normal || s->type == stmt_join) && (s_entry->input_tuple_size == 0 || (s_entry->output_tuple_size == 0 && rc > 0 && !s->monitor_sink_stat))) {
				struct timeval tvtest;
				cout << "try to get input tuple arrival time and size" << endl;
				cDBT data(MAX_STR_LEN, &tvtest);
				s->in->get(&data);
				cout << "arrival time of input tuple is " << data.atime->tv_sec << "." << data.atime->tv_usec << ", tuple size is " << data.keysz+data.datasz << endl;
				//cout << "arrived tuple is ";
				//data.print();
			}

			if (s_entry != NULL && (s_entry->input_tuple_size == 0 || (s_entry->output_tuple_size == 0 && rc > 0 && !s->monitor_sink_stat))) {
				struct timeval tvtest;

				if (s_entry->output_tuple_size == 0 && rc > 0 && !s->monitor_sink_stat) {
					cDBT data(MAX_STR_LEN, &tvtest);
					s->out->get(&data);
					s_entry->output_tuple_size = (data.keysz+data.datasz);
					if (querySchdl::verbose) cout << "output tuple size is set to " << s_entry->output_tuple_size << endl;
				}

				if (s_entry->input_tuple_size == 0) {

					cDBT data(MAX_STR_LEN, &tvtest);

					if (s->type == stmt_normal || s->type == stmt_join) {
						s->in->get(&data);
					} else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
						uStmt* s1 = (uStmt*) s;
						for (list<buffer*>::iterator itr = s1->union_bufs.begin(); itr != s1->union_bufs.end(); itr++) {
							if (!(*itr)->empty()) {
								(*itr)->get(&data);
								break;
							}
						}
					}

					s_entry->input_tuple_size =(data.keysz+data.datasz);
					if (querySchdl::verbose) cout << "input tuple size is set to " << s_entry->input_tuple_size << endl;
				}
			}

			if(s_entry != NULL && s_entry->total_input.on) {
				s_entry->total_input.value++;
				s_entry->total_input.last_updated.tv_sec = tv2.tv_sec;
				s_entry->total_input.last_updated.tv_usec = tv2.tv_usec;      
			}
			if(s_entry != NULL && s_entry->win_total_input.on) {
				s_entry->win_total_input.value++;
				s_entry->win_total_input.last_updated.tv_sec = tv2.tv_sec;
				s_entry->win_total_input.last_updated.tv_usec = tv2.tv_usec;      
			}

			if(s_entry != NULL && s_entry->win_cumul_input_buf_size.on) {
				if(s_entry->win_total_input.on && (s_entry->win_total_input.value % STMT_BUF_MEASURE_INTERVAL) == 1) {
					long buf_size = 0;
					if (s->type == stmt_normal || s->type == stmt_join) {
						buf_size = s->in->bufSize();
					} else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
						uStmt* s1 = (uStmt*) s;
						for (list<buffer*>::iterator itr = s1->union_bufs.begin(); itr != s1->union_bufs.end(); itr++) {
							if (!(*itr)->empty()) {
								buf_size += (*itr)->bufSize();
								break;
							}
						}
					}

					s_entry->win_cumul_input_buf_size.value += (buf_size+1);
				}
			}

			if(i_entry != NULL && i_entry->total_input.on) {
				i_entry->total_input.value++;
				i_entry->total_input.last_updated.tv_sec = tv2.tv_sec;
				i_entry->total_input.last_updated.tv_usec = tv2.tv_usec;
			}
			if(i_entry != NULL && i_entry->win_total_input.on) {
				i_entry->win_total_input.value++;
				i_entry->win_total_input.last_updated.tv_sec = tv2.tv_sec;
				i_entry->win_total_input.last_updated.tv_usec = tv2.tv_usec;
			}

			if(i_entry != NULL && i_entry->win_cumul_input_buf_size.on) {
				if(i_entry->win_total_input.on && (i_entry->win_total_input.value % STMT_BUF_MEASURE_INTERVAL) == 1) {
					i_entry->win_cumul_input_buf_size.value += (s->in->bufSize()+1);	
					//cout << "input buf size is " << (s->in->bufSize()+1) << endl;
				}
			}

			double wait = timeval_subtract(tv, tv_arrival);
			if(i_entry != NULL && i_entry->win_max_tuple_wait_time.on) {
				if (i_entry->win_max_tuple_wait_time.value < wait) {
					i_entry->win_max_tuple_wait_time.value = wait;
				}
				i_entry->win_max_tuple_wait_time.last_updated.tv_sec = tv2.tv_sec;
				i_entry->win_max_tuple_wait_time.last_updated.tv_usec = tv2.tv_usec;
			}

			if(i_entry != NULL && i_entry->win_total_tuple_wait_time.on) {
				i_entry->win_total_tuple_wait_time.value += wait;
				i_entry->win_total_tuple_wait_time.last_updated.tv_sec = tv2.tv_sec;
				i_entry->win_total_tuple_wait_time.last_updated.tv_usec = tv2.tv_usec;
			}

			if (s->monitor_src_stat && win_max_tuple_wait_time.on) {
				if (win_max_tuple_wait_time.value < wait) {
					win_max_tuple_wait_time.value = wait;
				}
				win_max_tuple_wait_time.last_updated.tv_sec = tv2.tv_sec;
				win_max_tuple_wait_time.last_updated.tv_usec = tv2.tv_usec;
			}

			if (s->monitor_src_stat && win_total_tuple_wait_time.on) {
				win_total_tuple_wait_time.value += wait;
				win_total_tuple_wait_time.last_updated.tv_sec = tv2.tv_sec;
				win_total_tuple_wait_time.last_updated.tv_usec = tv2.tv_usec;
			}

			if(s->monitor_src_stat && win_total_input.on) {
				win_total_input.value++;
				win_total_input.last_updated.tv_sec = tv2.tv_sec;
				win_total_input.last_updated.tv_usec = tv2.tv_usec;      
			}

			if (rc > 0) {
				if(s_entry != NULL && s_entry->total_output.on) {
					s_entry->total_output.value += rc;
					s_entry->total_output.last_updated.tv_sec = tv2.tv_sec;
					s_entry->total_output.last_updated.tv_usec = tv2.tv_usec;
				}
				if(s_entry != NULL && s_entry->win_total_output.on) {
					s_entry->win_total_output.value += rc;
					s_entry->win_total_output.last_updated.tv_sec = tv2.tv_sec;
					s_entry->win_total_output.last_updated.tv_usec = tv2.tv_usec;
				}

				if(o_entry != NULL && o_entry->total_output.on) {
					o_entry->total_output.value += rc;
					o_entry->total_output.last_updated.tv_sec = tv2.tv_sec;
					o_entry->total_output.last_updated.tv_usec = tv2.tv_usec;
				}
				if(o_entry != NULL && o_entry->win_total_output.on) {
					o_entry->win_total_output.value += rc;
					o_entry->win_total_output.last_updated.tv_sec = tv2.tv_sec;
					o_entry->win_total_output.last_updated.tv_usec = tv2.tv_usec;
				}

				if(s->monitor_sink_stat && win_total_output.on) {
					win_total_output.value += rc;
					win_total_output.last_updated.tv_sec = tv2.tv_sec;
					win_total_output.last_updated.tv_usec = tv2.tv_usec;
				}
			}

			//since tuples will at least be removed and possibly produced, check total buffer size periodically.
			if (win_max_total_buf_bytes.on) {
				win_input_processed_cnt++;
				if ((win_input_processed_cnt % MEM_MEASURE_INTERVAL) == 0) {
					win_buf_size_meas_cnt++;

					//first get the current total buf usage
					long total_buf_bytes = 0;
					long total_source_buf_counts = 0;
					DrvMgr* dm = DrvMgr::getInstance();
					for (list<Driver*>::iterator itr = dm->getDrivers()->begin(); itr != dm->getDrivers()->end(); itr++) {
						BufStateTblType* graph = (*itr)->getBufGraph();
						for (BufStateTblType::iterator itr2 = graph->begin(); itr2 != graph->end(); itr2++) {
							b_entry state = itr2->second;
							if (state->b_type != t_sink && state->b_type != t_merge_sink && state->b_type != t_fork_dummy && state->b_type != t_none) {
								total_buf_bytes += state->buf->bufByteSize();
								if (state->b_type == t_source || state->b_type == t_source_fork) {
									total_source_buf_counts += state->buf->bufSize();
								}
							}
						}
					}
					win_cumul_total_buf_bytes += total_buf_bytes;
					win_cumul_total_source_buf_counts += total_source_buf_counts;
					if (win_max_total_buf_bytes.value < total_buf_bytes) {
						win_max_total_buf_bytes.value = total_buf_bytes;
					}
				}
			}
		}


		//diff is calcualted above
		//long tv_sec = (long)diff;
		//long tv_usec = (long) ((diff - tv_sec)*1000000);

		if(s_entry != NULL && s_entry->total_p_time.on && !arrival_abnormal) {
			s_entry->total_p_time.value += diff;
			s_entry->total_p_time.last_updated.tv_sec = tv2.tv_sec;
			s_entry->total_p_time.last_updated.tv_usec = tv2.tv_usec;
		}  

		if(win_total_p_time.on && !arrival_abnormal) {
			win_total_p_time.value += diff;
			win_total_p_time.last_updated.tv_sec = tv2.tv_sec;
			win_total_p_time.last_updated.tv_usec = tv2.tv_usec;
		}  

		if(s_entry != NULL && s_entry->win_total_p_time.on && !arrival_abnormal) {
			s_entry->win_total_p_time.value += diff;
			s_entry->total_p_time.last_updated.tv_sec = tv2.tv_sec;
			s_entry->total_p_time.last_updated.tv_usec = tv2.tv_usec;
		}  

		if(o_entry != NULL && (o_entry->total_latency.on || o_entry->max_latency.on || o_entry->win_total_latency.on || o_entry->win_max_latency.on) && rc > 0 && !arrival_abnormal) {
			//tv_sec = (long)diff3;
			//tv_usec = (long) ((diff3 - tv_sec)*1000000); 

			if (o_entry->max_latency.on && o_entry->max_latency.value < diff3) {
				o_entry->max_latency.value = diff3;
				o_entry->max_latency.last_updated.tv_sec = tv2.tv_sec;
				o_entry->max_latency.last_updated.tv_usec = tv2.tv_usec;
			}

			if (o_entry->total_latency.on) {
				//rc is # of output tuples, treat them as exiting the system at the same time.    
				//total latency for all output tuples.
				//tv_usec *= rc;
				//tv_sec *= rc;
				o_entry->total_latency.value += diff3*rc;
				o_entry->total_latency.last_updated.tv_sec = tv2.tv_sec;
				o_entry->total_latency.last_updated.tv_usec = tv2.tv_usec;
			}

			//now the windowed stats.
			if (o_entry->win_total_latency.on) {
				//rc is # of output tuples, treat them as exiting the system at the same time.    
				//total latency for all output tuples.
				o_entry->win_total_latency.value += diff3*rc;
			}

			if (o_entry->win_max_latency.on && o_entry->win_max_latency.value < diff3) {
				o_entry->win_max_latency.value = diff3;
				o_entry->win_max_latency.last_updated.tv_sec = tv2.tv_sec;
				o_entry->win_max_latency.last_updated.tv_usec = tv2.tv_usec;
			}
		} 

		if (s->monitor_sink_stat && win_total_latency.on && !arrival_abnormal) {
			//rc is # of output tuples, treat them as exiting the system at the same time.    
			//total latency for all output tuples.
			win_total_latency.value += diff3*rc;
		}

		if (s->monitor_sink_stat && win_max_latency.on && win_max_latency.value < diff3 && !arrival_abnormal) {
			win_max_latency.value = diff3;
			win_max_latency.last_updated.tv_sec = tv2.tv_sec;
			win_max_latency.last_updated.tv_usec = tv2.tv_usec;
		}

		//now manage the sensor flags. For now input and output entries and all windowed entries are either always on or always off, or set by the user.
		if (s_entry != NULL && s_entry->total_input.on && s_entry->total_input.value > 0 && (s_entry->total_input.value % NUM_TUPLES_TO_MONITOR_BEFORE_REST) == 0) {
			//for now the three stmt entries are set and unset together
			//keep it simple for now. monitor NUM_TUPLES_TO_MONITOR tuples, 
			//and wait for a period of time that will have 10*NUM_TUPLES_TO_MONITOR tuple based on current speed before waking up again
			//restart the measurement from scratch every 10*NUM_TUPLES_TO_MONITOR tuples are actually monitored (equivelant to every 100*NUM_TUPLES_TO_MONITOR tuples)
			double diff4 = timeval_subtract(tv2, s_entry->start_time.value);
			long tv_sec = (long)diff4;  
			long tv_usec = (long) ((diff4 - tv_sec)*1000000);

			struct timeval tv4;
			tv4.tv_usec = tv2.tv_usec + 10*(tv_usec);
			long sec = (long) (tv4.tv_usec/1000000);
			tv4.tv_sec = tv2.tv_sec + 10*(tv_sec) + sec;    

			s_entry->total_input.wakeup_time.tv_sec = tv4.tv_sec;
			s_entry->total_input.wakeup_time.tv_usec = tv4.tv_usec;
			s_entry->total_input.on = false;

			s_entry->total_output.wakeup_time.tv_sec = tv4.tv_sec;
			s_entry->total_output.wakeup_time.tv_usec = tv4.tv_usec;
			s_entry->total_output.on = false;

			s_entry->total_p_time.wakeup_time.tv_sec = tv4.tv_sec;
			s_entry->total_p_time.wakeup_time.tv_usec = tv4.tv_usec;
			s_entry->total_p_time.on = false;

			if (s_entry->total_input.value > 0 && (s_entry->total_input.value % (10*NUM_TUPLES_TO_MONITOR_BEFORE_REST)) == 0) {
				//cache the old values
				s_entry->last_known_selectivity = s_entry->total_output.value/s_entry->total_input.value;
				s_entry->last_known_process_rate = s_entry->total_input.value/s_entry->total_p_time.value;

				//start from scratch
				s_entry->total_input.value = 0;
				s_entry->total_input.last_updated.tv_sec = 0;
				s_entry->total_input.last_updated.tv_usec = 0;

				s_entry->total_output.value = 0;
				s_entry->total_output.last_updated.tv_sec = 0;
				s_entry->total_output.last_updated.tv_usec = 0;

				s_entry->total_p_time.value = 0;
				s_entry->total_p_time.last_updated.tv_sec = 0;
				s_entry->total_p_time.last_updated.tv_usec = 0;

				s_entry->start_time.value.tv_sec = 0;
				s_entry->start_time.value.tv_usec = 0;
			}
		}

		//do this in exp mode only. Increase operator cost such that overhead looks more respectable. 
		//(operator cost low means relative overhead appears high)
		//if (monitor_exp_mode) {
		//  int testval = 65635;
		//  for (int l = 0; l < 40000; l++) {
		//    testval*testval/(testval-4);
		//  }
		// }

		struct timeval tv_exit;
		gettimeofday(&tv_exit, &tz);
		double diff_exit = timeval_subtract(tv_exit, tv_begin);

		//if (diff_exit > 0.000001) {
		//cout << "proc_monitor time: " << diff_exit << endl;
		//}

		//if (diff_exit > 0.01) {
		//cout << "proc_monitor time over 0.01s: " << diff_exit << endl;
		//}

		//if (diff_exit > 0.001) {
		//cout << "outlier proc_monitor time for " << s->name << " is " << diff_exit << endl;
		//diff_exit = 0.00002;
		//}

		if (total_processing_time.on) {
			total_processing_time.value += diff_exit;
		}

		if (querySchdl::verbose && drv->stateTableChanged) cout << 10 << " ";

		if (querySchdl::verbose) cout << "exiting monitor->exestmt()" << endl;

		return (stmt_rc) rc;
	}

	void Monitor::updateLastInTurn(stmt* s) {
		//This method is been called very frequently so I have to comment the log
		//querySchdl::SMLOG(10, "Entering Monitor::updateLastInTurn");	
		if (s == NULL || !(s->valid)) return;
		DrvMgr* dm = DrvMgr::getInstance();

		stmt_entry* s_entry = NULL;

		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv, &tz);

		/*
		   if (stmtStats->find(s->name) != stmtStats->end()) {
		   s_entry = (*stmtStats)[s->name];
		   }

		   if(s_entry != NULL) {
		   s_entry->last_in_turn.value.tv_sec = tv.tv_sec;
		   s_entry->last_in_turn.value.tv_usec = tv.tv_usec;
		   }
		 */

		//these are used for window stats caculation in sensors.
		double cur_time; 
		cur_time = timeval_subtract(tv, this->start_time);
		long cur_sec = (long) cur_time;

		//if (win_start_sec == 0) win_start_sec = cur_sec;

		if (cur_sec >= (this->win_start_sec+Monitor::WINDOW_SIZE_SEC)) {    
			struct timeval tv_start;
			gettimeofday(&tv_start, &tz);

			if (querySchdl::verbose) {
				cout << "new window for monitor. current cur_sec " << cur_sec << ", current win_start_sec " << win_start_sec << ", window size is " << WINDOW_SIZE_SEC << endl;
				//printf("0 ");
				//fflush(stdout);
			}
			//if (querySchdl::verbose) cout << "0.1 ";

			//the only thing to update before any possible output, is to measure a final total buf size, this especially useful when there is not enough activity going on (less than 10 tuples per second), so that we ensure at least one measurement happen for each WINDOW_SIZE_SEC
			win_buf_size_meas_cnt++;

			//first get the current total buf usage
			//also update each StmtEntry with source buf for its current queue size (for latency estimation). This is now not done for union/join buffers
			//!!!!!!!!!!!!--- not done for union/join -----------!!!!!!!!!!!!!!
			long total_buf_bytes = 0;
			long total_source_buf_counts = 0;

			//if (querySchdl::verbose) cout << "0.5 ";

			DrvMgr* dm = DrvMgr::getInstance();

			//if (querySchdl::verbose) cout << "1 ";

			if (querySchdl::verbose) {
				//printf("1 ");
				//fflush(stdout);
			}

			for (list<Driver*>::iterator itr = dm->getDrivers()->begin(); itr != dm->getDrivers()->end(); itr++) {
				if (querySchdl::verbose) {
					//printf("1.1 ");
					//fflush(stdout);
				}

				BufStateTblType* graph = (*itr)->getBufGraph();
				for (BufStateTblType::iterator itr2 = graph->begin(); itr2 != graph->end(); itr2++) {
					b_entry state = itr2->second;
					if (state->b_type != t_sink && state->b_type != t_merge_sink && state->b_type != t_fork_dummy && state->b_type != t_none) {
						if (querySchdl::verbose) {
							//printf("1.2(%s) ", state->buf->name);
							//fflush(stdout);
						}

						total_buf_bytes += state->buf->bufByteSize();

						if (querySchdl::verbose) {
							//printf("1.5(%s) ", state->buf->name);
							//fflush(stdout);
						}

						if (state->b_type == t_source || state->b_type == t_source_fork) {
							int bufSize = state->buf->bufSize();
							total_source_buf_counts += bufSize;

							if (querySchdl::verbose) {
								//printf("1.8(%s) ", state->buf->name);
								//fflush(stdout);
							}

							if (dm->getDrivers()->front()->getStrategy(true) != NULL) {
								//update StmtEntry
								if (state->statement->type != stmt_t_union && state->statement->type != stmt_tl_union && state->statement->type != stmt_join) {
									state->statement->stmtEntry->lastWinQSize = bufSize;
								}
							}

							if (querySchdl::verbose) {
								//printf("1.9(%s) ", state->buf->name);
								//fflush(stdout);
							}
						}
					}
				}
			}

			//if (querySchdl::verbose) cout << "2 ";
			if (querySchdl::verbose) {
				//printf("2 ");
				//fflush(stdout);
			}

			win_cumul_total_buf_bytes += total_buf_bytes;
			win_cumul_total_source_buf_counts += total_source_buf_counts;

			if (win_max_total_buf_bytes.value < total_buf_bytes) {
				win_max_total_buf_bytes.value = total_buf_bytes;
			}

			if (querySchdl::verbose) {
				cout << "new win starts, total buf size calculated here is " << total_buf_bytes << endl;
				//printf("new win starts, total buf size calculated here is %d\n", total_buf_bytes);
				//fflush(stdout);
			}

			//if (querySchdl::verbose) cout << "3 ";

			//now go through send list, send performance data to buffers, which will then be sent to client
			bufferMngr *bm = bufferMngr::getInstance();
			if (!stat_to_send_bufs.empty()) {      
				for (set<perf_stat_info*, ltsspair>::iterator itr = stat_to_send_bufs.begin(); itr != stat_to_send_bufs.end(); ++itr) {
					//look up the performance data, send it
					perf_stat_info* info = *itr;

					buffer* buf = bm->lookup(info->buf_name);

					if (!buf) {
						//check to see if the buffer needs to be created, or the stmt is already invalid, if later delete it
						//set iterator should be fail-safe, so delete it this way
						if(info->isGlobal) cerr << "error, global performance buffer not found." << endl;
						else {
							//look to see whether the stmt is still not deleted, if deleted, also delete the invalid entry.
							//this relies on the fail-safe nature of set (on its iterators).
							if(!dm->stmtInUse(info->stmt_name.c_str())) {
								//delete this entry since it is no longer in use
								//(when stmt is removed from driver, this entry is not removed, for simplicity)
								stat_to_send_bufs.erase(itr);
							} else {
								//create this buffer from stat_to_send_bufs
								bm->create(info->buf_name, SHARED);
							}	       
						}
					}

					if (buf) {
						string msg = "s||";
						msg.append(info->buf_name);
						msg.append("||");
						char stat_value[50];

						if (info->isGlobal) {
							if (info->stat_name == TOTAL_OUTPUT_TUPLE) {
								sprintf(stat_value, "%d", win_total_output.value);
								//cout << "output value is " << win_total_output.value << endl;

								msg.append(stat_value);
							} else if (info->stat_name == AVG_LATENCY) {
								double win_avg_latency = 0;
								if (win_total_output.value > 0) {
									win_avg_latency = win_total_latency.value/(double)(win_total_output.value);
								}

								sprintf(stat_value, "%f", win_avg_latency);
								//cout << "output value is " << win_total_output.value << endl;

								msg.append(stat_value);
							} else if (info->stat_name == MAX_LATENCY) {
								sprintf(stat_value, "%f", win_max_latency.value);
								//cout << "output value is " << win_total_output.value << endl;

								msg.append(stat_value);
							} else if (info->stat_name == LAST_TOTAL_BUF_BYTES) {

								sprintf(stat_value, "%d", total_buf_bytes);
								//cout << "output value is " << win_total_output.value << endl;

								msg.append(stat_value);
							}
						} else {
							//see if the corresponding performance data exists for this statement, if not, activate it
							if (info->stat_name == TOTAL_OUTPUT_TUPLE) {

								sprintf(stat_value, "%d", win_total_output.value);
								//cout << "output value is " << win_total_output.value << endl;

								msg.append(stat_value);
							} else if (info->stat_name == AVG_LATENCY) {
								double win_avg_latency = 0;
								if (win_total_output.value > 0) {
									win_avg_latency = win_total_latency.value/(double)(win_total_output.value);
								}

								sprintf(stat_value, "%f", win_avg_latency);
								//cout << "output value is " << win_total_output.value << endl;

								msg.append(stat_value);
							} else if (info->stat_name == MAX_LATENCY) {
							} else if (info->stat_name == LAST_TOTAL_BUF_BYTES) {
								//this should not exist, does not make sense?
							}	    
						}

						int messageSize = MAX_ID_LEN*2;
						cDBT cdbt(messageSize);

						memset(cdbt.data, '\0', messageSize);
						msg.append("\n");
						strcpy((char*)cdbt.data, msg.c_str());

						//cout << "string sent is " << msg.c_str() << endl;

						buf->put(&cdbt);
					}
				}
			}

			//enter new window of time slice, output old stat first for experiments
			if (monitor_exp_mode) {
				if (!outNumsFile) {
					cerr << "can not open stat output file: " << monitor_exp_numloc << endl;
					//return (stmt_rc) rc;
				} else {
					//treat it as preferring using weighted strategy over unweighted ones.
					Driver* drv = dm->getDrivers()->front();
					string stratName;
					if (drv->getStrategy(true) != NULL) stratName = drv->getStrategy(true)->getName();
					else if (drv->getStrategy() != NULL) stratName = drv->getStrategy()->getName();

					double win_avg_latency = 0;
					if (win_total_output.value > 0) {
						win_avg_latency = win_total_latency.value/(double)(win_total_output.value);
					}


					//following for package experiments only. to remove later
					long tcp1_win_input = 0; //for packets exp only. to remove later
					long tcp2_win_input = 0; //for packets exp only. to remove later
					long tcp3_win_input = 0; //for packets exp only. to remove later
					long tcp4_win_input = 0; //for packets exp only. to remove later
					long tcp5_win_input = 0; //for packets exp only. to remove later
					long tcp6_win_input = 0; //for packets exp only. to remove later
					long tcp7_win_input = 0; //for packets exp only. to remove later
					long tcp8_win_input = 0; //for packets exp only. to remove later
					long tcp9_win_input = 0; //for packets exp only. to remove later
					long tcp10_win_input = 0; //for packets exp only. to remove later	
					double tcp1_win_tuple_wait_max = 0;
					double tcp2_win_tuple_wait_max = 0;
					double tcp3_win_tuple_wait_max = 0;
					double tcp4_win_tuple_wait_max = 0;
					double tcp5_win_tuple_wait_max = 0;
					double tcp6_win_tuple_wait_max = 0;
					double tcp7_win_tuple_wait_max = 0;
					double tcp8_win_tuple_wait_max = 0;
					double tcp9_win_tuple_wait_max = 0;
					double tcp10_win_tuple_wait_max = 0;	
					double tcp1_win_tuple_wait_avg = 0;
					double tcp2_win_tuple_wait_avg = 0;
					double tcp3_win_tuple_wait_avg = 0;
					double tcp4_win_tuple_wait_avg = 0;
					double tcp5_win_tuple_wait_avg = 0;
					double tcp6_win_tuple_wait_avg = 0;
					double tcp7_win_tuple_wait_avg = 0;
					double tcp8_win_tuple_wait_avg = 0;
					double tcp9_win_tuple_wait_avg = 0;
					double tcp10_win_tuple_wait_avg = 0;	

					float tcp1_win_in_buf_size_avg = 0;
					float tcp2_win_in_buf_size_avg = 0;
					float tcp3_win_in_buf_size_avg = 0;
					float tcp4_win_in_buf_size_avg = 0;
					float tcp5_win_in_buf_size_avg = 0;
					float tcp6_win_in_buf_size_avg = 0;
					float tcp7_win_in_buf_size_avg = 0;
					float tcp8_win_in_buf_size_avg = 0;
					float tcp9_win_in_buf_size_avg = 0;
					float tcp10_win_in_buf_size_avg = 0;

					for (InputStatMapType::iterator itr = inputStats->begin(); itr != inputStats->end(); ++itr) {
						//tcp buffer stats
						if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp1") == 0) {
							tcp1_win_input = itr->second->win_total_input.value;
							tcp1_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp1_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp1_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp2") == 0) {
							tcp2_win_input = itr->second->win_total_input.value;
							tcp2_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp2_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp2_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp3") == 0) {
							tcp3_win_input = itr->second->win_total_input.value;
							tcp3_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp3_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp3_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp4") == 0) {
							tcp4_win_input = itr->second->win_total_input.value;
							tcp4_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp4_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp4_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp5") == 0) {
							tcp5_win_input = itr->second->win_total_input.value;
							tcp5_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp5_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp5_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp6") == 0) {
							tcp6_win_input = itr->second->win_total_input.value;
							tcp6_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp6_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp6_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp7") == 0) {
							tcp7_win_input = itr->second->win_total_input.value;
							tcp7_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp7_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp7_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp8") == 0) {
							tcp8_win_input = itr->second->win_total_input.value;
							tcp8_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp8_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp8_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp9") == 0) {
							tcp9_win_input = itr->second->win_total_input.value;
							tcp9_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp9_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp9_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} else if (itr->second->win_total_input.on && strcmp(itr->first, "bai__tcp10") == 0) {
							tcp10_win_input = itr->second->win_total_input.value;
							tcp10_win_tuple_wait_max = itr->second->win_max_tuple_wait_time.value;
							tcp10_win_tuple_wait_avg = (itr->second->win_total_input.value == 0 ? 0 : itr->second->win_total_tuple_wait_time.value/(double)itr->second->win_total_input.value);
							tcp10_win_in_buf_size_avg = (float)itr->second->win_cumul_input_buf_size.value/((int)(itr->second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1);
						} 
					}	

					double tcp1_win_lat_max = 0;
					double tcp2_win_lat_max = 0;
					double tcp3_win_lat_max = 0;
					double tcp4_win_lat_max = 0;
					double tcp5_win_lat_max = 0;
					double tcp6_win_lat_max = 0;
					double tcp7_win_lat_max = 0;
					double tcp8_win_lat_max = 0;
					double tcp9_win_lat_max = 0;
					double tcp10_win_lat_max = 0;	
					double tcp1_win_lat_avg = 0;
					double tcp2_win_lat_avg = 0;
					double tcp3_win_lat_avg = 0;
					double tcp4_win_lat_avg = 0;
					double tcp5_win_lat_avg = 0;
					double tcp6_win_lat_avg = 0;
					double tcp7_win_lat_avg = 0;
					double tcp8_win_lat_avg = 0;
					double tcp9_win_lat_avg = 0;
					double tcp10_win_lat_avg = 0;	
					for (OutputStatMapType::iterator itr = outputStats->begin(); itr != outputStats->end(); ++itr) {
						//tcp buffer stats
						if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_605593") == 0 || strcmp(itr->first, "a131_179_64_67_543236") == 0)) {
							tcp1_win_lat_max = itr->second->win_max_latency.value;
							tcp1_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_89222") == 0 || strcmp(itr->first, "a131_179_64_67_992941") == 0)) {
							tcp2_win_lat_max = itr->second->win_max_latency.value;
							tcp2_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_536936") == 0 || strcmp(itr->first, "a131_179_64_67_544728") == 0)) {
							tcp3_win_lat_max = itr->second->win_max_latency.value;
							tcp3_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_982067") == 0 || strcmp(itr->first, "a131_179_64_67_994513") == 0)) {
							tcp4_win_lat_max = itr->second->win_max_latency.value;
							tcp4_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_633757") == 0 || strcmp(itr->first, "a131_179_64_67_544306") == 0)) {
							tcp5_win_lat_max = itr->second->win_max_latency.value;
							tcp5_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_387435") == 0 || strcmp(itr->first, "a131_179_64_67_992092") == 0)) {
							tcp6_win_lat_max = itr->second->win_max_latency.value;
							tcp6_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_852489") == 0 || strcmp(itr->first, "a131_179_64_67_541875") == 0)) {
							tcp7_win_lat_max = itr->second->win_max_latency.value;
							tcp7_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_504140") == 0 || strcmp(itr->first, "a131_179_64_67_989667") == 0)) {
							tcp8_win_lat_max = itr->second->win_max_latency.value;
							tcp8_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && (strcmp(itr->first, "a131_179_64_67_461796") == 0 || strcmp(itr->first, "a131_179_64_67_563454") == 0)) {
							tcp9_win_lat_max = itr->second->win_max_latency.value;
							tcp9_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} else if (itr->second->win_total_output.on && strcmp(itr->first, "a131_179_64_67_671681") == 0) {
							tcp10_win_lat_max = itr->second->win_max_latency.value;
							tcp10_win_lat_avg = (itr->second->win_total_output.value == 0 ? 0 : itr->second->win_total_latency.value/(double)itr->second->win_total_output.value);
						} 
					}

					outNumsFile << win_start_sec << '\t' << win_avg_latency << '\t' << win_max_latency.value << '\t' 
						<< win_max_stmt_wait_time.value << '\t' << win_max_tuple_wait_time.value << '\t' 
						<< (win_total_input.value == 0 ? 0 : (win_total_tuple_wait_time.value/(double)win_total_input.value)) 
						<< '\t' << tcp1_win_tuple_wait_max << '\t' << tcp1_win_tuple_wait_avg
						<< '\t' << tcp2_win_tuple_wait_max << '\t' << tcp2_win_tuple_wait_avg
						<< '\t' << tcp3_win_tuple_wait_max << '\t' << tcp3_win_tuple_wait_avg
						<< '\t' << tcp4_win_tuple_wait_max << '\t' << tcp4_win_tuple_wait_avg
						<< '\t' << tcp5_win_tuple_wait_max << '\t' << tcp5_win_tuple_wait_avg
						<< '\t' << tcp6_win_tuple_wait_max << '\t' << tcp6_win_tuple_wait_avg
						<< '\t' << tcp7_win_tuple_wait_max << '\t' << tcp7_win_tuple_wait_avg
						<< '\t' << tcp8_win_tuple_wait_max << '\t' << tcp8_win_tuple_wait_avg
						<< '\t' << tcp9_win_tuple_wait_max << '\t' << tcp9_win_tuple_wait_avg
						<< '\t' << tcp10_win_tuple_wait_max << '\t' << tcp10_win_tuple_wait_avg	  
						<< '\t' << tcp1_win_lat_max << '\t' << tcp1_win_lat_avg
						<< '\t' << tcp2_win_lat_max << '\t' << tcp2_win_lat_avg
						<< '\t' << tcp3_win_lat_max << '\t' << tcp3_win_lat_avg
						<< '\t' << tcp4_win_lat_max << '\t' << tcp4_win_lat_avg
						<< '\t' << tcp5_win_lat_max << '\t' << tcp5_win_lat_avg
						<< '\t' << tcp6_win_lat_max << '\t' << tcp6_win_lat_avg
						<< '\t' << tcp7_win_lat_max << '\t' << tcp7_win_lat_avg
						<< '\t' << tcp8_win_lat_max << '\t' << tcp8_win_lat_avg
						<< '\t' << tcp9_win_lat_max << '\t' << tcp9_win_lat_avg
						<< '\t' << tcp10_win_lat_max << '\t' << tcp10_win_lat_avg	  
						<< '\t' << win_total_input.value << '\t' << win_total_output.value 
						<< '\t' << total_scheduling_time.value << '\t' <<  total_processing_time.value
						<< '\t' << total_empty_buffer_hit.value
						<< '\t' << tcp1_win_input << '\t' << tcp2_win_input << '\t' << tcp3_win_input 
						<< '\t' << tcp4_win_input << '\t' << tcp5_win_input << '\t' << tcp6_win_input 
						<< '\t' << tcp7_win_input << '\t' << tcp8_win_input << '\t' << tcp9_win_input << '\t' << tcp10_win_input 
						<< '\t' << win_max_total_buf_bytes.value 
						<< '\t' << (win_buf_size_meas_cnt == 0 ? 0 : (win_cumul_total_buf_bytes/win_buf_size_meas_cnt)) 
						<< '\t' << (win_buf_size_meas_cnt == 0 ? 0 : (win_cumul_total_source_buf_counts/win_buf_size_meas_cnt))
						<< '\t' << win_total_p_time.value
						<< '\t' << stratName
						<< endl;
					outNumsFile.flush();      	

					//cout << win_start_sec << ": average input buffer size for source buffer stmts during the past " << WINDOW_SIZE_SEC << " seconds: " << tcp1_win_in_buf_size_avg << ", " << tcp2_win_in_buf_size_avg << ", " << tcp3_win_in_buf_size_avg << ", " << tcp4_win_in_buf_size_avg << ", " << tcp5_win_in_buf_size_avg << ", " << tcp6_win_in_buf_size_avg << ", " << tcp7_win_in_buf_size_avg << ", " << tcp8_win_in_buf_size_avg << ", " << tcp9_win_in_buf_size_avg << endl;

					//if (querySchdl::verbose) cout << "4 ";

					//for experiments, switch strategies every "interval" seconds. start from RR, and switch around.
					int interval = 240;
					int gracePeriod = mon_exp_start_sec + 480;

					if (win_start_sec == gracePeriod) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						ScheduleStrategy* weightedStrategy = drv->getStrategy(true);
						//drv->setStrategy((ScheduleStrategy*)NULL);
						drv->setStrategy(new OCSWRRDLStrategy(drv));

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							weightedStrategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy(true)->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						StrategyFactory::getInstance()->addToTrash(weightedStrategy);
						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OCSWRRStrategy" << endl;

					} else if (win_start_sec == gracePeriod + interval) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						ScheduleStrategy* weightedStrategy = drv->getStrategy(true);
						drv->setStrategy(new WeightOnlyWRRStrategy(drv));

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							weightedStrategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy(true)->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						StrategyFactory::getInstance()->addToTrash(weightedStrategy);
						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created WeightOnlyWRRStrategy" << endl;

					} else if (win_start_sec == gracePeriod + 2*interval) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						ScheduleStrategy* weightedStrategy = drv->getStrategy(true);
						drv->setStrategy(new OCSWRRStrategy(drv));

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							weightedStrategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy(true)->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						StrategyFactory::getInstance()->addToTrash(weightedStrategy);
						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OCSWRRStrategy" << endl;

					} else if (win_start_sec == gracePeriod + 3*interval) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						ScheduleStrategy* weightedStrategy = drv->getStrategy(true);
						drv->setStrategy(new OCSPriorityStrategy(drv));

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							weightedStrategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy(true)->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						StrategyFactory::getInstance()->addToTrash(weightedStrategy);
						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OCSPriorityStrategy" << endl;

					} else if (win_start_sec == gracePeriod + 4*interval) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						ScheduleStrategy* weightedStrategy = drv->getStrategy(true);
						drv->setStrategy(new OCSPriorityDLStrategy(drv));

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							weightedStrategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy(true)->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						StrategyFactory::getInstance()->addToTrash(weightedStrategy);
						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OCSPriorityStrategy" << endl;

					} else if (win_start_sec == (gracePeriod + 5*interval)) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						ScheduleStrategy* weightedStrategy = drv->getStrategy(true);
						drv->setStrategy((ScheduleStrategy*)NULL);
						drv->setStrategy(new OcsSegStrategy(drv));

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							weightedStrategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy()->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						StrategyFactory::getInstance()->addToTrash(weightedStrategy);
						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OcsSegStrategy" << endl;
					} else if (win_start_sec == (gracePeriod + 6*interval)) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						Strategy* strategy = drv->getStrategy();
						drv->setStrategy(new OcsQuotaStrategy(drv));
						//delete strategy;

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							strategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy()->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OcsQuotaStrategy" << endl;
					} else if (win_start_sec == (gracePeriod + 7*interval)) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						Strategy* strategy = drv->getStrategy();
						drv->setStrategy(new RRStrategy(drv, 50));
						//delete strategy;

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							strategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy()->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OcsQuotaStrategy" << endl;
					} else if (win_start_sec == (gracePeriod + 8*interval)) {
						if (querySchdl::verbose) {
							cout << "driver before:" << endl;
							drv->printStateTable();
						}

						Strategy* strategy = drv->getStrategy();
						drv->setStrategy(new OcsSegNoOptmztnStrategy(drv));
						//delete strategy;

						if (querySchdl::verbose) {
							cout << "old strategy" << endl;
							strategy->printStateTable();
							cout << "new strategy" << endl;
							drv->getStrategy()->printStateTable();

							cout << "driver after:" << endl;
							drv->printStateTable();	    
						}

						drv->stateTableChanged = true;
						if (querySchdl::verbose) cout << "created OcsSegNoOptmztnStrategy" << endl;

						mon_exp_start_sec = win_start_sec;
					} 
					//if (querySchdl::verbose) cout << "5 " << endl;
				}      
			}

			//reset everything, to start scratch on windowed stats
			win_start_sec = cur_sec;
			newWindow = true;  

			//global first
			if (win_total_output.on) win_total_input.value = 0;
			if (win_total_output.on) win_total_output.value = 0;
			if (win_total_latency.on) win_total_latency.value = 0;
			if (win_max_latency.on) win_max_latency.value = 0;
			if (win_max_stmt_wait_time.on) win_max_stmt_wait_time.value = 0;
			if (win_max_tuple_wait_time.on) win_max_tuple_wait_time.value = 0;
			if (win_total_tuple_wait_time.on) win_total_tuple_wait_time.value = 0;
			if (win_total_p_time.on) {
				last_win_total_p_time = win_total_p_time.value;
				win_total_p_time.value = 0;
			}

			if (win_max_total_buf_bytes.on) {
				win_max_total_buf_bytes.value = 0;
				win_cumul_total_buf_bytes = 0;
				win_cumul_total_source_buf_counts = 0;
				win_buf_size_meas_cnt = 0;
				win_input_processed_cnt = 0;
			}

			//then reset all window sensors
			for (StmtStatMapType::iterator itr = stmtStats->begin(); itr != stmtStats->end(); ++itr) {
				if(itr->second->win_total_input.on) {
					itr->second->win_total_input.value = 0;
				}
				if(itr->second->win_total_output.on) {
					itr->second->win_total_output.value = 0;
				}
				if(itr->second->win_total_p_time.on) {
					itr->second->win_total_p_time.value = 0;
				}      
				if(itr->second->win_cumul_input_buf_size.on) {
					itr->second->win_cumul_input_buf_size.value = 0;
				}      
			}

			for (InputStatMapType::iterator itr = inputStats->begin(); itr != inputStats->end(); ++itr) {
				if(itr->second->win_total_input.on) {
					//if (querySchdl::verbose) cout << "reset input entry for " << s->in->name << " at second " << cur_sec << endl;
					itr->second->win_total_input.value = 0;
				}
				if(itr->second->win_max_tuple_wait_time.on) {
					itr->second->win_max_tuple_wait_time.value = 0;
				}
				if(itr->second->win_total_tuple_wait_time.on) {
					itr->second->win_total_tuple_wait_time.value = 0;
				}
				if(itr->second->win_cumul_input_buf_size.on) {
					itr->second->win_cumul_input_buf_size.value = 0;
				}      
			}

			for (OutputStatMapType::iterator itr = outputStats->begin(); itr != outputStats->end(); ++itr) {
				if(itr->second->win_total_output.on) {
					itr->second->win_total_output.value = 0;
				}
				if(itr->second->win_total_latency.on) {
					itr->second->win_total_latency.value = 0;
				}
				if(itr->second->win_max_latency.on) {
					itr->second->win_max_latency.value = 0;
				}
			}

			struct timeval tv_exit;
			gettimeofday(&tv_exit, &tz);
			double diff_exit = timeval_subtract(tv_exit, tv_start);
			if (total_processing_time.on) {
				total_processing_time.value += diff_exit;
			}

			if (querySchdl::verbose) cout << "finish starting new window" << endl;
			if (querySchdl::verbose) {
				//printf("finish starting new window\n");
				//fflush(stdout);
			}
		}

		/*
		//also calculate max run interval of stmts (starvation)
		stmt* s2;
		double stmt_wait_time = 0;
		for (StmtStatMapType::iterator itr = stmtStats->begin(); itr != stmtStats->end(); ++itr) {
//for now make it only source stmts when calculate max run interval
s2 = dm->getStmtByName(itr->first);
if(win_max_stmt_wait_time.on && s2->monitor_src_stat 
&& itr->second->last_in_turn.on && itr->second->last_in_turn.value.tv_sec > 0) {
//stmt wait time
stmt_wait_time = timeval_subtract(tv, itr->second->last_in_turn.value);
}

if (win_max_stmt_wait_time.on && win_max_stmt_wait_time.value < stmt_wait_time) {    
win_max_stmt_wait_time.value = stmt_wait_time;
}      
}
		 */

//struct timeval tv_exit;
//gettimeofday(&tv_exit, &tz);
//double diff_exit = timeval_subtract(tv_exit, tv);
//if (total_processing_time.on) {
//  total_processing_time.value += diff_exit;
//}

return;
}

void Monitor::updateWakeup() {
		querySchdl::SMLOG(10, "Entering Monitor::updateWakeup");	
	if (querySchdl::verbose) {
		cout << "update monitor wakeup flags." << endl;
	}

	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	for (StmtStatMapType::iterator itr = stmtStats->begin(); itr != stmtStats->end(); ++itr) {
		//currently all three cumulative stats in stmt_stats are done together
		if (!(itr->second->total_input.on)) {
			if (itr->second->total_input.wakeup_time.tv_sec != 0 && itr->second->total_input.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
				itr->second->total_input.on = true;
				itr->second->total_output.on = true;
				itr->second->total_p_time.on = true;
				//itr->second->start_time.value.tv_sec = tv.tv_sec;
				//itr->second->start_time.value.tv_usec = tv.tv_usec;
			}
		}

		if (!(itr->second->win_total_input.on)) {
			if (itr->second->win_total_input.wakeup_time.tv_sec != 0 && itr->second->win_total_input.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
				itr->second->win_total_input.on = true;
			}
		}

		if (!(itr->second->win_total_output.on)) {
			if (itr->second->win_total_output.wakeup_time.tv_sec != 0 && itr->second->win_total_output.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
				itr->second->win_total_output.on = true;
			}
		}

		if (!(itr->second->win_total_p_time.on)) {
			if (itr->second->win_total_p_time.wakeup_time.tv_sec != 0 && itr->second->win_total_p_time.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
				itr->second->win_total_p_time.on = true;
			}
		}

		if (!(itr->second->win_cumul_input_buf_size.on)) {
			if (itr->second->win_cumul_input_buf_size.wakeup_time.tv_sec != 0 && itr->second->win_cumul_input_buf_size.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
				itr->second->win_cumul_input_buf_size.on = true;
			}
		}
	}

	/* these are not used now
	   for (InputStatMapType::iterator itr = inputStats->begin(); itr != inputStats->end(); ++itr) {
	   if (!(itr->second->total_input.on)) {
	   if (itr->second->total_input.wakeup_time.tv_sec != 0 && itr->second->total_input.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->total_input.on = true;
	   }
	   }    

	   if (!(itr->second->win_total_input.on)) {
	   if (itr->second->win_total_input.wakeup_time.tv_sec != 0 && itr->second->win_total_input.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->win_total_input.on = true;
	   }
	   }

	   if (!(itr->second->win_max_tuple_wait_time.on)) {
	   if (itr->second->win_max_tuple_wait_time.wakeup_time.tv_sec != 0 && itr->second->win_max_tuple_wait_time.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->win_max_tuple_wait_time.on = true;
	   }
	   }        

	   if (!(itr->second->win_total_tuple_wait_time.on)) {
	   if (itr->second->win_total_tuple_wait_time.wakeup_time.tv_sec != 0 && itr->second->win_total_tuple_wait_time.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->win_total_tuple_wait_time.on = true;
	   }
	   }        
	   }

	   for (OutputStatMapType::iterator itr = outputStats->begin(); itr != outputStats->end(); ++itr) {
	   if (!(itr->second->total_output.on)) {
	   if (itr->second->total_output.wakeup_time.tv_sec != 0 && itr->second->total_output.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->total_output.on = true;
	   }
	   }    
	   if (!(itr->second->total_latency.on)) {
	   if (itr->second->total_latency.wakeup_time.tv_sec != 0 && itr->second->total_latency.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->total_latency.on = true;
	   }
	   }    
	   if (!(itr->second->max_latency.on)) {
	   if (itr->second->max_latency.wakeup_time.tv_sec != 0 && itr->second->max_latency.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->max_latency.on = true;
	   }
	   }    

	   if (!(itr->second->win_total_output.on)) {
	   if (itr->second->win_total_output.wakeup_time.tv_sec != 0 && itr->second->win_total_output.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->win_total_output.on = true;
	   }
	   }    
	   if (!(itr->second->win_total_latency.on)) {
	   if (itr->second->win_total_latency.wakeup_time.tv_sec != 0 && itr->second->win_total_latency.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->win_total_latency.on = true;
	   }
	   }    
	   if (!(itr->second->win_max_latency.on)) {
	   if (itr->second->win_max_latency.wakeup_time.tv_sec != 0 && itr->second->win_max_latency.wakeup_time.tv_sec < (tv.tv_sec+1)) { //roughly is enough
	   itr->second->win_max_latency.on = true;
	   }
	   }    
	   }
	 */

	if (querySchdl::verbose) {
		cout << "end update monitor wakeup flags." << endl;
	}

}

//always rebuild from scratch, much simpler than incremental build. Since this happen relatively less frequently, tolerable.
//copy over existing data if there is any
void Monitor::updateMaps() {
		querySchdl::SMLOG(10, "Entering Monitor::updateMaps");	
	DrvMgr* dm = DrvMgr::getInstance();
	if (querySchdl::verbose) {
		cout << "update monitor after driver changes. There are total of " << dm->stmtToDrvMap->size() << " statements in the system." << endl;
		//cout << "Before update: " << endl;
		//printStateTables();
	}

	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	//everything is monitored, when the first statements are added to driver
	//if (stmtStats ->size() == 0 && dm->stmtToDrvMap->size() > 0) {
	//  stmts_last_monitored.tv_sec = tv.tv_sec;
	//  stmts_last_monitored.tv_usec = tv.tv_usec;
	//  inputs_last_monitored.tv_sec = tv.tv_sec;
	//  inputs_last_monitored.tv_usec = tv.tv_usec;
	//  outputs_last_monitored.tv_sec = tv.tv_sec;
	//  outputs_last_monitored.tv_usec = tv.tv_usec;
	//}

	StmtStatMapType* stmtStats2 = new StmtStatMapType();
	InputStatMapType* inputStats2 = new InputStatMapType();
	OutputStatMapType* outputStats2 = new OutputStatMapType();

	//analyze the driver graphs, and build the corresponding map
	for (StmtMapType::iterator itr = dm->stmtToDrvMap->begin(); itr != dm->stmtToDrvMap->end(); itr++) {

		stmt* s = dm->getStmtByName(itr->first);
		if (s == NULL || !(s->valid)) continue;

		Driver* drv = (*(dm->stmtToDrvMap))[s->name];
		char* s_name;

		//first reset all statement flags that were leftover.
		s->monitor_src_stat = false; 
		s->monitor_sink_stat = false;

		//the statement stat is always kept.
		//if (stmtStats2->find(s->name) == stmtStats2->end()) {
		stmt_entry* entry = new stmt_entry();
		if (stmtStats->find(s->name) == stmtStats->end()) {
			/*done by constructor now
			  entry->total_input.value = 0;
			  entry->total_input.last_updated.tv_sec = 0;
			  entry->total_input.last_updated.tv_usec = 0;
			  entry->total_input.wakeup_time.tv_sec = 0;
			  entry->total_input.wakeup_time.tv_usec = 0;

			  entry->total_output.value = 0;
			  entry->total_output.last_updated.tv_sec = 0;
			  entry->total_output.last_updated.tv_usec = 0;
			  entry->total_output.wakeup_time.tv_sec = 0;
			  entry->total_output.wakeup_time.tv_usec = 0;

			  entry->total_p_time.value.tv_sec = 0;
			  entry->total_p_time.value.tv_usec = 0;
			  entry->total_p_time.last_updated.tv_sec = 0;
			  entry->total_p_time.last_updated.tv_usec = 0;
			  entry->total_p_time.wakeup_time.tv_sec = 0;
			  entry->total_p_time.wakeup_time.tv_usec = 0;

			  entry->start_time.value.tv_sec = 0;
			  entry->start_time.value.tv_usec = 0;

			  entry->last_in_turn.value.tv_sec = 0;
			  entry->last_in_turn.value.tv_usec = 0;      
			 */
		} else {
			//copy over old data
			stmt_entry* entry2 = (*stmtStats)[s->name];

			entry->total_input.value = entry2->total_input.value;
			entry->total_input.last_updated.tv_sec = entry2->total_input.last_updated.tv_sec;
			entry->total_input.last_updated.tv_usec = entry2->total_input.last_updated.tv_usec;
			entry->total_input.wakeup_time.tv_sec = entry2->total_input.wakeup_time.tv_sec;
			entry->total_input.wakeup_time.tv_usec = entry2->total_input.wakeup_time.tv_usec;
			entry->total_input.on = entry2->total_input.on;

			entry->total_output.value = entry2->total_output.value;
			entry->total_output.last_updated.tv_sec = entry2->total_output.last_updated.tv_sec;
			entry->total_output.last_updated.tv_usec = entry2->total_output.last_updated.tv_usec;
			entry->total_output.wakeup_time.tv_sec = entry2->total_output.wakeup_time.tv_sec;
			entry->total_output.wakeup_time.tv_usec = entry2->total_output.wakeup_time.tv_usec;
			entry->total_output.on = entry2->total_output.on;

			entry->total_p_time.value = entry2->total_p_time.value;
			entry->total_p_time.last_updated.tv_sec = entry2->total_p_time.last_updated.tv_sec;
			entry->total_p_time.last_updated.tv_usec = entry2->total_p_time.last_updated.tv_usec;
			entry->total_p_time.wakeup_time.tv_sec = entry2->total_p_time.wakeup_time.tv_sec;
			entry->total_p_time.wakeup_time.tv_usec = entry2->total_p_time.wakeup_time.tv_usec;
			entry->total_p_time.on = entry2->total_p_time.on;

			entry->win_total_input.value = entry2->win_total_input.value;
			entry->win_total_input.last_updated.tv_sec = entry2->win_total_input.last_updated.tv_sec;
			entry->win_total_input.last_updated.tv_usec = entry2->win_total_input.last_updated.tv_usec;
			entry->win_total_input.wakeup_time.tv_sec = entry2->win_total_input.wakeup_time.tv_sec;
			entry->win_total_input.wakeup_time.tv_usec = entry2->win_total_input.wakeup_time.tv_usec;
			entry->win_total_input.on = entry2->win_total_input.on;

			entry->win_total_output.value = entry2->win_total_output.value;
			entry->win_total_output.last_updated.tv_sec = entry2->win_total_output.last_updated.tv_sec;
			entry->win_total_output.last_updated.tv_usec = entry2->win_total_output.last_updated.tv_usec;
			entry->win_total_output.wakeup_time.tv_sec = entry2->win_total_output.wakeup_time.tv_sec;
			entry->win_total_output.wakeup_time.tv_usec = entry2->win_total_output.wakeup_time.tv_usec;
			entry->win_total_output.on = entry2->win_total_output.on;

			entry->win_total_p_time.value = entry2->win_total_p_time.value;
			entry->win_total_p_time.last_updated.tv_sec = entry2->win_total_p_time.last_updated.tv_sec;
			entry->win_total_p_time.last_updated.tv_usec = entry2->win_total_p_time.last_updated.tv_usec;
			entry->win_total_p_time.wakeup_time.tv_sec = entry2->win_total_p_time.wakeup_time.tv_sec;
			entry->win_total_p_time.wakeup_time.tv_usec = entry2->win_total_p_time.wakeup_time.tv_usec;
			entry->win_total_p_time.on = entry2->win_total_p_time.on;

			entry->start_time.value.tv_sec = entry2->start_time.value.tv_sec;
			entry->start_time.value.tv_usec = entry2->start_time.value.tv_usec;      

			entry->last_in_turn.value.tv_sec = entry2->last_in_turn.value.tv_sec;
			entry->last_in_turn.value.tv_usec = entry2->last_in_turn.value.tv_usec;      

			entry->last_known_selectivity = entry2->last_known_selectivity;
			entry->last_known_process_rate = entry2->last_known_process_rate;
			entry->input_tuple_size = entry2->input_tuple_size;
			entry->output_tuple_size = entry2->output_tuple_size;

			entry->win_cumul_input_buf_size.value = entry2->win_cumul_input_buf_size.value;
			entry->win_cumul_input_buf_size.last_updated.tv_sec = entry2->win_cumul_input_buf_size.last_updated.tv_sec;
			entry->win_cumul_input_buf_size.last_updated.tv_usec = entry2->win_cumul_input_buf_size.last_updated.tv_usec;
			entry->win_cumul_input_buf_size.wakeup_time.tv_sec = entry2->win_cumul_input_buf_size.wakeup_time.tv_sec;
			entry->win_cumul_input_buf_size.wakeup_time.tv_usec = entry2->win_cumul_input_buf_size.wakeup_time.tv_usec;
			entry->win_cumul_input_buf_size.on = entry2->win_cumul_input_buf_size.on;

		}

		s_name = new char[strlen(s->name)+1];
		strcpy(s_name, s->name);
		(*stmtStats2)[s_name] = entry;
		//}

		//now, the sink specific map, need to check topology
		if(drv->getBufType(s->out) == t_sink || drv->getBufType(s->out) == t_merge_sink) {
			//if (outputStats2->find(s->name) == outputStats2->end()) {
			output_entry* entry = new output_entry();
			if (outputStats->find(s->name) == outputStats->end()) {
				/*
				   entry->total_output.value = 0;
				   entry->total_output.last_updated.tv_sec = 0;
				   entry->total_output.last_updated.tv_usec = 0;
				   entry->total_output.wakeup_time.tv_sec = 0;
				   entry->total_output.wakeup_time.tv_usec = 0;

				   entry->total_latency.value.tv_sec = 0;
				   entry->total_latency.value.tv_usec = 0;
				   entry->total_latency.last_updated.tv_sec = 0;
				   entry->total_latency.last_updated.tv_usec = 0;
				   entry->total_latency.wakeup_time.tv_sec = 0;
				   entry->total_latency.wakeup_time.tv_usec = 0;

				   entry->max_latency.value = 0;
				   entry->max_latency.last_updated.tv_sec = 0;
				   entry->max_latency.last_updated.tv_usec = 0;
				   entry->max_latency.wakeup_time.tv_sec = 0;
				   entry->max_latency.wakeup_time.tv_usec = 0;
				 */
			} else {
				//copy over old data
				output_entry* entry2 = (*outputStats)[s->name];

				entry->total_output.value = entry2->total_output.value;
				entry->total_output.last_updated.tv_sec = entry2->total_output.last_updated.tv_sec;
				entry->total_output.last_updated.tv_usec = entry2->total_output.last_updated.tv_usec;
				entry->total_output.wakeup_time.tv_sec = entry2->total_output.wakeup_time.tv_sec;
				entry->total_output.wakeup_time.tv_usec = entry2->total_output.wakeup_time.tv_usec;
				entry->total_output.on = entry2->total_output.on;

				entry->total_latency.value = entry2->total_latency.value;
				entry->total_latency.last_updated.tv_sec = entry2->total_latency.last_updated.tv_sec;
				entry->total_latency.last_updated.tv_usec = entry2->total_latency.last_updated.tv_usec;
				entry->total_latency.wakeup_time.tv_sec = entry2->total_latency.wakeup_time.tv_sec;
				entry->total_latency.wakeup_time.tv_usec = entry2->total_latency.wakeup_time.tv_usec;
				entry->total_latency.on = entry2->total_latency.on;

				entry->max_latency.value = entry2->max_latency.value;
				entry->max_latency.last_updated.tv_sec = entry2->max_latency.last_updated.tv_sec;
				entry->max_latency.last_updated.tv_usec = entry2->max_latency.last_updated.tv_usec;
				entry->max_latency.wakeup_time.tv_sec = entry2->max_latency.wakeup_time.tv_sec;
				entry->max_latency.wakeup_time.tv_usec = entry2->max_latency.wakeup_time.tv_usec;
				entry->max_latency.on = entry2->max_latency.on;

				entry->win_total_output.value = entry2->win_total_output.value;
				entry->win_total_output.last_updated.tv_sec = entry2->win_total_output.last_updated.tv_sec;
				entry->win_total_output.last_updated.tv_usec = entry2->win_total_output.last_updated.tv_usec;
				entry->win_total_output.wakeup_time.tv_sec = entry2->win_total_output.wakeup_time.tv_sec;
				entry->win_total_output.wakeup_time.tv_usec = entry2->win_total_output.wakeup_time.tv_usec;
				entry->win_total_output.on = entry2->win_total_output.on;

				entry->win_total_latency.value = entry2->win_total_latency.value;
				entry->win_total_latency.last_updated.tv_sec = entry2->win_total_latency.last_updated.tv_sec;
				entry->win_total_latency.last_updated.tv_usec = entry2->win_total_latency.last_updated.tv_usec;
				entry->win_total_latency.wakeup_time.tv_sec = entry2->win_total_latency.wakeup_time.tv_sec;
				entry->win_total_latency.wakeup_time.tv_usec = entry2->win_total_latency.wakeup_time.tv_usec;
				entry->win_total_latency.on = entry2->win_total_latency.on;

				entry->win_max_latency.value = entry2->win_max_latency.value;
				entry->win_max_latency.last_updated.tv_sec = entry2->win_max_latency.last_updated.tv_sec;
				entry->win_max_latency.last_updated.tv_usec = entry2->win_max_latency.last_updated.tv_usec;
				entry->win_max_latency.wakeup_time.tv_sec = entry2->win_max_latency.wakeup_time.tv_sec;
				entry->win_max_latency.wakeup_time.tv_usec = entry2->win_max_latency.wakeup_time.tv_usec;
				entry->win_max_latency.on = entry2->win_max_latency.on;
			}

			(*outputStats2)[s_name] = entry;
			s->monitor_sink_stat = true;
			//}
		}

		//source specific map depends on stmt type
		if(drv == NULL || drv == 0) {
			cerr << "internal error, stmt belongs to NULL driver" << endl;
		}

		if (s->type == stmt_normal) {
			if(drv->getBufType(s->in) == t_source || drv->getBufType(s->in) == t_source_fork) {
				if (inputStats2->find(s->in->name) == inputStats2->end()) {
					input_entry* entry = new input_entry();
					if (inputStats->find(s->in->name) == inputStats->end()) {
						/*
						   entry->total_input.value = 0;
						   entry->total_input.last_updated.tv_sec = 0;
						   entry->total_input.last_updated.tv_usec = 0;
						   entry->total_input.wakeup_time.tv_sec = 0;
						   entry->total_input.wakeup_time.tv_usec = 0;

						   entry->start_time.value.tv_sec = 0;
						   entry->start_time.value.tv_usec = 0;
						 */
					} else {
						//copy over old data
						input_entry* entry2 = (*inputStats)[s->in->name];

						entry->total_input.value = entry2->total_input.value;
						entry->total_input.last_updated.tv_sec = entry2->total_input.last_updated.tv_sec;
						entry->total_input.last_updated.tv_usec = entry2->total_input.last_updated.tv_usec;
						entry->total_input.wakeup_time.tv_sec = entry2->total_input.wakeup_time.tv_sec;
						entry->total_input.wakeup_time.tv_usec = entry2->total_input.wakeup_time.tv_usec;
						entry->total_input.on = entry2->total_input.on;

						entry->win_total_input.value = entry2->win_total_input.value;
						entry->win_total_input.last_updated.tv_sec = entry2->win_total_input.last_updated.tv_sec;
						entry->win_total_input.last_updated.tv_usec = entry2->win_total_input.last_updated.tv_usec;
						entry->win_total_input.wakeup_time.tv_sec = entry2->win_total_input.wakeup_time.tv_sec;
						entry->win_total_input.wakeup_time.tv_usec = entry2->win_total_input.wakeup_time.tv_usec;
						entry->win_total_input.on = entry2->win_total_input.on;

						entry->win_max_tuple_wait_time.value = entry2->win_max_tuple_wait_time.value;
						entry->win_max_tuple_wait_time.last_updated.tv_sec = entry2->win_max_tuple_wait_time.last_updated.tv_sec;
						entry->win_max_tuple_wait_time.last_updated.tv_usec = entry2->win_max_tuple_wait_time.last_updated.tv_usec;
						entry->win_max_tuple_wait_time.wakeup_time.tv_sec = entry2->win_max_tuple_wait_time.wakeup_time.tv_sec;
						entry->win_max_tuple_wait_time.wakeup_time.tv_usec = entry2->win_max_tuple_wait_time.wakeup_time.tv_usec;
						entry->win_max_tuple_wait_time.on = entry2->win_max_tuple_wait_time.on;

						entry->win_total_tuple_wait_time.value = entry2->win_total_tuple_wait_time.value;
						entry->win_total_tuple_wait_time.last_updated.tv_sec = entry2->win_total_tuple_wait_time.last_updated.tv_sec;
						entry->win_total_tuple_wait_time.last_updated.tv_usec = entry2->win_total_tuple_wait_time.last_updated.tv_usec;
						entry->win_total_tuple_wait_time.wakeup_time.tv_sec = entry2->win_total_tuple_wait_time.wakeup_time.tv_sec;
						entry->win_total_tuple_wait_time.wakeup_time.tv_usec = entry2->win_total_tuple_wait_time.wakeup_time.tv_usec;
						entry->win_total_tuple_wait_time.on = entry2->win_total_tuple_wait_time.on;

						entry->start_time.value.tv_sec = entry2->start_time.value.tv_sec;
						entry->start_time.value.tv_usec = entry2->start_time.value.tv_usec;

						entry->win_cumul_input_buf_size.value = entry2->win_cumul_input_buf_size.value;
						entry->win_cumul_input_buf_size.last_updated.tv_sec = entry2->win_cumul_input_buf_size.last_updated.tv_sec;
						entry->win_cumul_input_buf_size.last_updated.tv_usec = entry2->win_cumul_input_buf_size.last_updated.tv_usec;
						entry->win_cumul_input_buf_size.wakeup_time.tv_sec = entry2->win_cumul_input_buf_size.wakeup_time.tv_sec;
						entry->win_cumul_input_buf_size.wakeup_time.tv_usec = entry2->win_cumul_input_buf_size.wakeup_time.tv_usec;
						entry->win_cumul_input_buf_size.on = entry2->win_cumul_input_buf_size.on;

					}

					char* name = new char[strlen(s->in->name)+1];
					strcpy(name, s->in->name);
					(*inputStats2)[name] = entry;
					s->monitor_src_stat = true;
				}
			}
		} else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
			/* actually in current implementation this can never be true, omit it for now
			//I have lots of input buffers. test one by one
			uStmt* s1 = static_cast<uStmt*> (s);
			for (list<buffer*>::iterator itr2 = s1->union_bufs.begin(); itr2 != s1->union_bufs.end(); itr2++) {
			if(drv->getBufType((*itr2)) == t_source || drv->getBufType((*itr2)) == t_source_fork) {
			if (inputStats2->find((*itr2)->name) == inputStats2->end()) {
			input_entry* entry = new input_entry();
			entry->total_input = 0;
			entry->total_time = 0.0f;
			entry->last_in_turn.tv_sec = 0;
			entry->last_in_turn.tv_usec = 0;

			char* name = new char[strlen((*itr2)->name)+1];
			strcpy(name, (*itr2)->name);	    
			(*inputStats2)[name] = entry;
			s->monitor_src_stat = true;
			}
			}
			}
			 */
		} else if (s->type == stmt_join) {
			//need to test "in" buffer
			if(drv->getBufType(s->in) == t_source || drv->getBufType(s->in) == t_source_fork) {
				if (inputStats2->find(s->in->name) == inputStats2->end()) {
					input_entry* entry = new input_entry();
					if (inputStats->find(s->in->name) == inputStats->end()) {
						/*
						   entry->total_input.value = 0;
						   entry->total_input.last_updated.tv_sec = 0;
						   entry->total_input.last_updated.tv_usec = 0;
						   entry->total_input.wakeup_time.tv_sec = 0;
						   entry->total_input.wakeup_time.tv_usec = 0;

						   entry->start_time.value.tv_sec = 0;
						   entry->start_time.value.tv_usec = 0;
						 */
					} else {
						//copy over old data
						input_entry* entry2 = (*inputStats)[s->in->name];

						entry->total_input.value = entry2->total_input.value;
						entry->total_input.last_updated.tv_sec = entry2->total_input.last_updated.tv_sec;
						entry->total_input.last_updated.tv_usec = entry2->total_input.last_updated.tv_usec;
						entry->total_input.wakeup_time.tv_sec = entry2->total_input.wakeup_time.tv_sec;
						entry->total_input.wakeup_time.tv_usec = entry2->total_input.wakeup_time.tv_usec;
						entry->total_input.on = entry2->total_input.on;

						entry->win_total_input.value = entry2->win_total_input.value;
						entry->win_total_input.last_updated.tv_sec = entry2->win_total_input.last_updated.tv_sec;
						entry->win_total_input.last_updated.tv_usec = entry2->win_total_input.last_updated.tv_usec;
						entry->win_total_input.wakeup_time.tv_sec = entry2->win_total_input.wakeup_time.tv_sec;
						entry->win_total_input.wakeup_time.tv_usec = entry2->win_total_input.wakeup_time.tv_usec;
						entry->win_total_input.on = entry2->win_total_input.on;

						entry->win_max_tuple_wait_time.value = entry2->win_max_tuple_wait_time.value;
						entry->win_max_tuple_wait_time.last_updated.tv_sec = entry2->win_max_tuple_wait_time.last_updated.tv_sec;
						entry->win_max_tuple_wait_time.last_updated.tv_usec = entry2->win_max_tuple_wait_time.last_updated.tv_usec;
						entry->win_max_tuple_wait_time.wakeup_time.tv_sec = entry2->win_max_tuple_wait_time.wakeup_time.tv_sec;
						entry->win_max_tuple_wait_time.wakeup_time.tv_usec = entry2->win_max_tuple_wait_time.wakeup_time.tv_usec;
						entry->win_max_tuple_wait_time.on = entry2->win_max_tuple_wait_time.on;

						entry->win_total_tuple_wait_time.value = entry2->win_total_tuple_wait_time.value;
						entry->win_total_tuple_wait_time.last_updated.tv_sec = entry2->win_total_tuple_wait_time.last_updated.tv_sec;
						entry->win_total_tuple_wait_time.last_updated.tv_usec = entry2->win_total_tuple_wait_time.last_updated.tv_usec;
						entry->win_total_tuple_wait_time.wakeup_time.tv_sec = entry2->win_total_tuple_wait_time.wakeup_time.tv_sec;
						entry->win_total_tuple_wait_time.wakeup_time.tv_usec = entry2->win_total_tuple_wait_time.wakeup_time.tv_usec;
						entry->win_total_tuple_wait_time.on = entry2->win_total_tuple_wait_time.on;

						entry->start_time.value.tv_sec = entry2->start_time.value.tv_sec;
						entry->start_time.value.tv_usec = entry2->start_time.value.tv_usec;

						entry->win_cumul_input_buf_size.value = entry2->win_cumul_input_buf_size.value;
						entry->win_cumul_input_buf_size.last_updated.tv_sec = entry2->win_cumul_input_buf_size.last_updated.tv_sec;
						entry->win_cumul_input_buf_size.last_updated.tv_usec = entry2->win_cumul_input_buf_size.last_updated.tv_usec;
						entry->win_cumul_input_buf_size.wakeup_time.tv_sec = entry2->win_cumul_input_buf_size.wakeup_time.tv_sec;
						entry->win_cumul_input_buf_size.wakeup_time.tv_usec = entry2->win_cumul_input_buf_size.wakeup_time.tv_usec;
						entry->win_cumul_input_buf_size.on = entry2->win_cumul_input_buf_size.on;

					}

					char* name = new char[strlen(s->in->name)+1];
					strcpy(name, s->in->name);
					(*inputStats2)[s->in->name] = entry;
					s->monitor_src_stat = true;
				}
			}
			/* actually in current implementation this can never be true, omit it for now
			   jStmt* s1 = static_cast<jStmt*> (s);
			   for (list<buffer*>::iterator itr2 = s1->window_bufs.begin(); itr2 != s1->window_bufs.end(); itr2++) {
			   if(drv->getBufType((*itr2)) == t_source || drv->getBufType((*itr2)) == t_source_fork) {
			   if (inputStats2->find((*itr2)->name) == inputStats2->end()) {
			   input_entry* entry = new input_entry();
			   entry->total_input = 0;
			   entry->total_time = 0.0f;
			   entry->last_in_turn.tv_sec = 0;
			   entry->last_in_turn.tv_usec = 0;

			   char* name = new char[strlen((*itr2)->name)+1];
			   strcpy(name, (*itr2)->name);	    	    
			   (*inputStats2)[(*itr2)->name] = entry;
			   s->monitor_src_stat = true;
			   }
			   }
			   }
			 */
		} else {
			cerr << "unknown statement type for stmt name: " << s->name << endl;
			return;
		}
	}

	/*
	//now remove invalid map entries
	querySchdl* qs = querySchdl::getInstance();
	list<char*> toRemove;
	for (StmtStatMapType::iterator itr = stmtStats2->begin(); itr != stmtStats2->end(); itr++) {
	if(!(dm->stmtInUse(itr->first)) && !(qs->stmtInUse(itr->first))) {
	toRemove.push_back(itr->first);
	}
	}

	for (list<char*>::iterator itr = toRemove.begin(); itr != toRemove.end(); itr++) {
	stmt_entry* se = (*stmtStats2)[*itr];
	stmtStats2->erase(*itr);
	delete se;

	output_entry* oe = (*outputStats2)[*itr];
	outputStats2->erase(*itr);
	delete oe;

//stmt and out tables share the same key string
delete (*itr);
}

list<char*> toRemove2;
for (InputStatMapType::iterator itr = inputStats2->begin(); itr != inputStats2->end(); itr++) {
if(!(dm->bufferInUse(itr->first)) && !(qs->bufferInUse(itr->first))) {
toRemove2.push_back(itr->first);
}
}

for (list<char*>::iterator itr = toRemove2.begin(); itr != toRemove2.end(); itr++) {
input_entry* ie = (*inputStats2)[*itr];
inputStats2->erase(*itr);
delete ie;
delete (*itr);
}
	 */

for (StmtStatMapType::iterator itr = stmtStats->begin(); itr != stmtStats->end(); ++itr) {
	//delete itr->first; //shared with out map, delete once only, to be done later
	delete itr->second;
}
delete stmtStats;
stmtStats = stmtStats2;

for (InputStatMapType::iterator itr = inputStats->begin(); itr != inputStats->end();) {

	char* s = itr->first; 
	delete itr->second;
	itr++;
	delete s;
}
delete inputStats;
inputStats = inputStats2;

for (OutputStatMapType::iterator itr = outputStats->begin(); itr != outputStats->end();) {
	char* s = itr->first; 
	delete itr->second;
	itr++;
	delete s;
}
delete outputStats;
outputStats = outputStats2;

if (querySchdl::verbose) {
	cout << "After update: " << endl;
	printStateTables();  
}

return;
}

void Monitor::printStateTables()
{
	querySchdl::SMLOG(10, "Entering Monitor::printStateTables");	
	DrvMgr* dm = DrvMgr::getInstance();
	cout << "currently monitor has " << stmtStats->size() << " stmts" << endl;

	cout << "stmt name" << "\t\t";
	cout << "total input" << "\t";
	cout << "total output" << "\t";
	cout << "total ptime" << "\t";
	cout << "win input" << "\t";
	cout << "win output" << "\t";
	cout << "win ptime" << "\t";
	cout << "avg inbuf size" << "\t";
	cout << "last run";
	cout << endl;
	for (StmtStatMapType::iterator itr = stmtStats->begin(); itr != stmtStats->end(); itr++) {

		if (strlen((*itr).first) >= 16) {
			cout << (*itr).first << "\t";
		} else if (strlen((*itr).first) >= 8) {
			cout << (*itr).first << "\t\t";
		} else {
			cout << (*itr).first << "\t\t\t";
		}

		cout << (*itr).second->total_input.value << '(' << ((*itr).second->total_input.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->total_output.value << '(' << ((*itr).second->total_output.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->total_p_time.value << '(' << ((*itr).second->total_p_time.on ? "on":"off") << ')' << "\t\t";

		cout << (*itr).second->win_total_input.value << '(' << ((*itr).second->win_total_input.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->win_total_output.value << '(' << ((*itr).second->win_total_output.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->win_total_p_time.value << '(' << ((*itr).second->win_total_p_time.on ? "on":"off") << ')' << "\t\t";
		cout << (float)(*itr).second->win_cumul_input_buf_size.value/((int)((*itr).second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1) << '(' << ((*itr).second->win_cumul_input_buf_size.on ? "on":"off") << ')' << "\t\t";

		char ts[60];
		struct tm* tmp = localtime(&((*itr).second->last_in_turn.value.tv_sec));
		strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
		cout << ts << "." << (*itr).second->last_in_turn.value.tv_usec;

		cout << endl;
	}
	cout << endl;

	cout << "output stmt" << "\t\t";
	cout << "total output" << "\t";
	cout << "total latency" << "\t";
	cout << "max latency";
	cout << "win output" << "\t";
	cout << "win latency" << "\t";
	cout << "win max latency";
	cout << endl;
	for (OutputStatMapType::iterator itr = outputStats->begin(); itr != outputStats->end(); itr++) {    

		if (strlen((*itr).first) >= 16) {
			cout << (*itr).first << "\t";
		} else if (strlen((*itr).first) >= 8) {
			cout << (*itr).first << "\t\t";
		} else {
			cout << (*itr).first << "\t\t\t";
		}

		cout << (*itr).second->total_output.value << '(' << ((*itr).second->total_output.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->total_latency.value << '(' << ((*itr).second->total_latency.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->max_latency.value << '(' << ((*itr).second->max_latency.on ? "on":"off") << ')';

		cout << (*itr).second->win_total_output.value << '(' << ((*itr).second->win_total_output.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->win_total_latency.value << '(' << ((*itr).second->win_total_latency.on ? "on":"off") << ')' << "\t\t";
		cout << (*itr).second->win_max_latency.value << '(' << ((*itr).second->win_max_latency.on ? "on":"off") << ')';

		cout << endl;
	}
	cout << endl;

	cout << "input buffer" << "\t\t";
	cout << "total input" << "\t";
	cout << "win input" << "\t";
	cout << "win max tup wait" << "\t";
	cout << "win tot tup wait" << "\t";
	cout << "avrage size" << "\t";
	cout << "start time";
	cout << endl;
	for (InputStatMapType::iterator itr = inputStats->begin(); itr != inputStats->end(); itr++) {
		if (strlen((*itr).first) >= 16) {
			cout << (*itr).first << "\t";
		} else if (strlen((*itr).first) >= 8) {
			cout << (*itr).first << "\t\t";
		} else {
			cout << (*itr).first << "\t\t\t";
		}

		cout << (*itr).second->total_input.value << '(' << ((*itr).second->total_input.on ? "on":"off") << ')' << "\t\t";

		cout << (*itr).second->win_total_input.value << '(' << ((*itr).second->win_total_input.on ? "on":"off") << ')' << "\t\t";

		cout << (*itr).second->win_max_tuple_wait_time.value << '(' << ((*itr).second->win_max_tuple_wait_time.on ? "on":"off") << ')' << "\t\t";

		cout << (*itr).second->win_total_tuple_wait_time.value << '(' << ((*itr).second->win_total_tuple_wait_time.on ? "on":"off") << ')' << "\t\t";

		cout << (float)(*itr).second->win_cumul_input_buf_size.value/((int)((*itr).second->win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1) << '(' << ((*itr).second->win_cumul_input_buf_size.on ? "on":"off") << ')' << "\t\t";

		char ts[60];
		struct tm* tmp = localtime(&((*itr).second->start_time.value.tv_sec));
		strftime(ts, 60, TIMESTAMP_FORMAT, tmp);
		cout << ts << "." << (*itr).second->start_time.value.tv_usec;

		cout << endl;
	}
	cout << endl;

}

void Monitor::addSendStat(char* statName, char* stmt_name)
{
	querySchdl::SMLOG(10, "Entering Monitor::addSendStat");	
	perf_stat_info* pair = new perf_stat_info();

	if (stmt_name == NULL) {
		pair->isGlobal = true;
		pair->stat_name = statName;
		pair->buf_name = statName;
	} else {
		pair->isGlobal = false;
		pair->stat_name = statName;
		pair->stmt_name = stmt_name;
		pair->buf_name = pair->stmt_name + pair->stat_name;
	}
	stat_to_send_bufs.insert(pair);      
}

void Monitor::removeSendStat(char* statName, char* stmt_name)
{
	querySchdl::SMLOG(10, "Entering Monitor::removeSendStat");	
	perf_stat_info pair;

	if (stmt_name == NULL) {
		pair.isGlobal = true;
		pair.stat_name = statName;
		pair.buf_name = statName;
	} else {
		pair.isGlobal = false;
		pair.stat_name = statName;
		pair.stmt_name = stmt_name;
		pair.buf_name = pair.stmt_name + pair.stat_name;
	}

	set<perf_stat_info*, ltsspair>::iterator itr;
	if ((itr = stat_to_send_bufs.find(&pair)) != stat_to_send_bufs.end()) {
		stat_to_send_bufs.erase(itr);
	}
}

} // end of namespace ESL
