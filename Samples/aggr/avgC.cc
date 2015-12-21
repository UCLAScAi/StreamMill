#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <db.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <rtree.h>
extern "C"{
#include <im_db.h>
#include "swimlib.h"
#include "fptree.h"
#include "memmgr.h"
#include "timeval.h"
#include "histmgr.h"
#include <mcheck.h>
}
#include "adllib.h"
#include <ext/hash_map>
#include <winbuf.h>
#include <windowBuf.h>
using namespace ESL;

using namespace __gnu_cxx;

static hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables=new hash_map<const char*, void*, hash<const char*>, eqstrTab>;
extern int _ADL_NULL;
extern char* _ADL_NULL_STR;
#define MAX_STR_LEN 40960


int adldebug = 0;
int _adl_sqlcode, _adl_cursqlcode;
/**** Global Declarations ****/
IM_REL *traffic; 

struct avg_status { 
	double tsum; 
	int tcnt; 

	winbuf *win;
	int last_out;
	bool iterate;
	bool init;
	IM_REL *ret;
	IM_RELC *retc;
	int retc_first_entry;
};
extern "C" void avg_init(struct avg_status *status, 
		double next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
		hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
		vector<A_timeexp>* plist=NULL, int endSlide=0, 
		char* _modelId=NULL);
extern "C" void avg_iterate(struct avg_status *status, 
		double next, int _rec_id, bufferMngr* bm=NULL, 
		hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
		vector<A_timeexp>* plist=NULL, int endSlide=0, 
		char* _modelId=NULL);
extern "C" void avg_init(struct avg_status *status, double next,
		int _rec_id, int __is_init, bufferMngr* bm, 
		hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
		vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
	int rc;
	int _adl_sqlcode, _adl_cursqlcode;
	DBT key, data, windata;
	Rect r_key;
	char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];
	char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
	char _adl_dbname[80];
	IM_REL *window;
	int slide_out = 1;
	winbuf* rwinbuf = NULL;
	int rlast_out = 0;
	if (__is_init) {
		sprintf(_adl_dbname, "._%d_ret", status);
		(void)unlink(_adl_dbname);
		if ((rc = im_rel_create(&status->ret, NULL, IM_LINKEDLIST, 0)) != 0) {
			adlabort(rc, "im_rel_create()");
		}
		if ((rc = status->ret->open(status->ret, _adl_dbname, 0)) != 0) {
			adlabort(rc, "open()");
		}
		if ((rc = status->ret->cursor(status->ret, &status->retc, 0)) != 0) {
			adlabort(rc, "IM_REL->cursor()");
		}
	} /* end of __is_init */ 
	status->tsum = next; 
	status->tcnt = 1; 
	memset(&key, 0, sizeof(key)); 
	memset(&data, 0, sizeof(data)); 
	data.data = datadata; 
	key.data = keydata; 
	double ret = status->tsum/status->tcnt; 
	memcpy((char*)data.data+0, &ret, sizeof(double)); 
	data.size = 8; 
	key.size = 0; 
	if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) { 
		adlabort(rc, "IM_REL->put()"); 
	}
	printf ("salaam"); 

	status->retc_first_entry=1;
}
extern "C" void avg_iterate(struct avg_status *status, 
		double next, int _rec_id, bufferMngr* bm, 
		hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
		vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
	int rc;
	int _adl_sqlcode, _adl_cursqlcode;
	DBT key, data, windata;
	Rect r_key;
	char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
	char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
	char _adl_dbname[80];
	IM_REL *window;
	int slide_out = 1;
	winbuf* rwinbuf = NULL;
	int rlast_out = 0; 
	cout <<"in iterate"<< endl; 
	status->tsum = 0.9*status->tsum + 1.1*next; 
	status->tcnt = status->tcnt + 1; 
	memset(&key, 0, sizeof(key)); 
	memset(&data, 0, sizeof(data)); 
	data.data = datadata; 
	key.data = keydata; 
	double ret = status->tsum/status->tcnt; 
	memcpy((char*)data.data+0, &ret, sizeof(double)); 
	data.size = 8; 
	key.size = 0; 
	if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) { 
		adlabort(rc, "IM_REL->put()"); 
	} 

	status->retc_first_entry=1;
}
struct inWinType_avg_window {
	double next;
};
struct avg_window_status {
	winbuf *win;
	int last_out;
	bool iterate;
	bool init;
	IM_REL *ret;
	IM_RELC *retc;
	int retc_first_entry;
};
inWinType_avg_window* getLastTuple_avg_window(IM_REL* inwindow, inWinType_avg_window* tuple, bufferMngr* bm) {
	int rc;
	DBT key, data;
	char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];
	IM_RELC* temp;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	data.data = datadata;
	key.data = keydata;
	if ((rc = inwindow->cursor(inwindow, &temp, 0)) != 0) {
		adlabort(rc, "IM_REL->cursor()");
	}
	rc = temp->c_get(temp, &key, &data, DB_FIRST);
	if (rc == DB_NOTFOUND) {
		adlabort(rc, "IMREL->c_get() in oldest()");
	}
	memcpy(&((*tuple).next), (char*)data.data+0, sizeof(double));
	//printf("Retrieved (*tuple).next = %f\n", (*tuple).next);
	//fflush(stdout);
	return tuple;
}
extern "C" void avg_window_init(struct avg_window_status *status, 
		double next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
		hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
		vector<A_timeexp>* plist=NULL, int endSlide=0, 
		char* _modelId=NULL);
extern "C" void avg_window_init(struct avg_window_status *status, double next, 
		int _rec_id, int __is_init, bufferMngr* bm, 
		hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
		vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
	int rc;
	int _adl_sqlcode, _adl_cursqlcode;
	DBT key, data, windata;
	Rect r_key;
	char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];
	char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
	char _adl_dbname[80];
	IM_REL *window;
	int slide_out = 1;
	winbuf* rwinbuf = NULL;
	int rlast_out = 0;
	if(status && status->win)
		window = status->win->get_im_rel();
	struct inWinType_avg_window tuple;
	if (__is_init) {
		sprintf(_adl_dbname, "._%d_ret", status);
		(void)unlink(_adl_dbname);
		if ((rc = im_rel_create(&status->ret, NULL, IM_LINKEDLIST, 0)) != 0) {
			adlabort(rc, "im_rel_create()");
		}
		if ((rc = status->ret->open(status->ret, _adl_dbname, 0)) != 0) {
			adlabort(rc, "open()");
		}
		if ((rc = status->ret->cursor(status->ret, &status->retc, 0)) != 0) {
			adlabort(rc, "IM_REL->cursor()");
		}
	} /* end of __is_init */
	struct {
		double a_0;
		double a_0_expire;
		struct timeval atime;
	} insert_0;
	IM_RELC *w_6;
	int first_entry_5 = 1;
	int first_entry_6 = 1;
	int index_4 = 0;
	int terminating_4=0;
	struct gb_status_4 {
		struct avg_status *avg_0;
	};
	struct gb_status_4 *gbstatus_4 = (struct gb_status_4 *)0;

	int first_entry_4 = 1;
	int first_entry_2 = 1;
	rc = 0;
	_adl_cursqlcode = 1 /* ASSUME GUITY */;
	if ((rc = window->cursor(window, &w_6, 0)) != 0) {
		adlabort(rc, "WINDOW->cursor()");
	}
	while (rc==0) {
		struct {
			double a_0;
			double a_0_expire;
			struct timeval atime;
		} Q_0001_2_1;
next_2:
		struct {
			double a_0;
			double a_0_expire;
			struct timeval atime;
		} Q_0000_4_3;
next_4:
		while (index_4>=0 && index_4 < 2) {
			switch(index_4) {
				case 0:
					{
						if (terminating_4 == 0) {
							/* get source tuple from qun */
							struct {
								double next;
								double next_expire;
								struct timeval atime;
							} w_6_5;
next_6:
							memset(&key, 0, sizeof(key));
							memset(&data, 0, sizeof(data));
							rc = w_6->c_get(w_6, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
							if (rc==0) {
								first_entry_5 = 0;
								memcpy(&(w_6_5.next), (char*)data.data+0, sizeof(double));
								//printf("Retrieved w_6_5.next = %f\n", w_6_5.next);
								//fflush(stdout);
							} else if (rc == DB_NOTFOUND) {
								first_entry_5 = 1;
							} else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
							if (rc==0) {
								Q_0000_4_3.a_0 = w_6_5.next;
							} /* if (rc == 0) */
							if (rc==0) {
								first_entry_4 = 0;
								/* make assignments of non-aggr head expr */
								/* merge group-by columns into a key */
								strcpy(gbkey, "____");
								gbstatus_4 = (struct gb_status_4 *)0;
								rc = hash_get(4, _rec_id, gbkey, 4, (char**)&gbstatus_4);
								if (rc == DB_NOTFOUND) {//blah
									gbstatus_4 = (struct gb_status_4*)malloc(sizeof(*gbstatus_4));
									gbstatus_4->avg_0 = (struct avg_status*)malloc(sizeof(struct avg_status));
									gbstatus_4->avg_0->win = 0;
									setModelId("");
									avg_init(gbstatus_4->avg_0, Q_0000_4_3.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
									rc = hash_put(4, _rec_id, gbkey, 4, &gbstatus_4);
								} else if (rc == 0) {
									/* PHASE iterate */
									setModelId("");
									avg_iterate(gbstatus_4->avg_0, Q_0000_4_3.a_0, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
								} else adlabort(rc, "hash->get()");
							} else if (rc == DB_NOTFOUND) {
								terminating_4 = 1;
							}
						}
						if (terminating_4 == 1) {
							allkey = (char*)0;
							rc = hash_get(4, _rec_id, allkey, 4, (char**)&gbstatus_4);
							if (rc==0) {
							} else if(rc == DB_NOTFOUND) {
							} else adlabort(rc, "hash->get()");
						}
					}
					break;
				case 1:
					{
						memset(&key, 0, sizeof(key));
						memset(&data, 0, sizeof(data));
						rc = gbstatus_4->avg_0->retc->c_get(gbstatus_4->avg_0->retc, &key, &data, (gbstatus_4->avg_0->retc_first_entry)? DB_FIRST:DB_NEXT);
						if (rc == 0) {
							gbstatus_4->avg_0->retc_first_entry = 0;
							memcpy(&(Q_0001_2_1.a_0), (char*)data.data+0, sizeof(double));
							//printf("Retrieved Q_0001_2_1.a_0 = %f\n", Q_0001_2_1.a_0);
							//fflush(stdout);
							if ((rc = gbstatus_4->avg_0->retc->c_del(gbstatus_4->avg_0->retc, 0)) != 0) {
								adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
							}
						} else if (rc == DB_NOTFOUND) {
							gbstatus_4->avg_0->retc_first_entry = 1;
						} else adlabort(rc, "DBC->c_get()");
						first_entry_4 = 0;
					}
					break;
			} /*end of switch*/
			if (rc == 0) {
				index_4++;
			}
			if (rc == DB_NOTFOUND) {
				index_4--;
				if (terminating_4 == 1 && index_4 == 0) {
					if (gbstatus_4->avg_0->retc && (rc = gbstatus_4->avg_0->retc->c_close(gbstatus_4->avg_0->retc)) != 0) {
						adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
					}
					sprintf(_adl_dbname, "._%d_ret", gbstatus_4->avg_0);

					if (gbstatus_4->avg_0->ret && ((rc = gbstatus_4->avg_0->ret->close(gbstatus_4->avg_0->ret, 0)) != 0)) {
						adlabort(rc, "DB->close()");
					}
					gbstatus_4->avg_0->ret = NULL;
					(void)unlink(_adl_dbname);
					rc = DB_NOTFOUND;
				}
			}
		}/*end of while */
		if (rc == 0) index_4--;
		else 
		{
			int rc;		/* local rc */ 
			terminating_4 = 0;
			first_entry_4 = 1;
			index_4 = 0;
			/* free gbstatus */
			do {
				allkey = (char*)0;
				rc = hash_get(4, _rec_id, allkey, 4, (char**)&gbstatus_4);
				if (rc==0) {
					free(gbstatus_4->avg_0);
					//printf("freeing 4\n");
					free(gbstatus_4);
				}
			} while (rc==0);
			if (rc != DB_NOTFOUND) {
				adlabort(rc, "hash->get()");
			}
			/* release hash entry */
			hashgb_delete(4, _rec_id);
		}
		if (rc==0) {
			insert_0.a_0 = Q_0001_2_1.a_0;
		} /* if (rc == 0) */
		if (rc ==0 && slide_out == 1) {
			_adl_cursqlcode = 0; /* SUCCESS */ 
			/* INSERT STARTS */
			memset(&key, 0, sizeof(key));
			memset(&data, 0, sizeof(data));
			data.data = datadata;
			key.data = keydata;
			memcpy((char*)key.data+0, &(insert_0.a_0), sizeof(double));
			memcpy((char*)data.data+0, &(insert_0.a_0), sizeof(double));
			data.size = 8;
			key.size = 8;
			if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
				adlabort(rc, "IM_REL->put()");
			}
			/* INSERT ENDS */
		} else 
			slide_out = 1;
	} /* while (rc==0) */
	if (w_6 && (rc = w_6->c_close(w_6)) != 0) {
		adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
	}
	_adl_sqlcode = _adl_cursqlcode;
	status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_7()
{
	int rc = 0;
	DBT key, data, windata;
	struct timeval atime;
	Rect r_key;
	char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
	char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
	int _rec_id=0; /* recursive id */
	char _adl_dbname[MAX_STR_LEN];
	int slide_out = 1;
	winbuf* rwinbuf = NULL;
	int rlast_out = 0;
	static int last_out = 0;
	static bool iterate = false;
	static bool init = true;
	char _timeexpkey[MAX_STR_LEN];
	char *timeexpkey=_timeexpkey;
	rc = 0;
	_adl_cursqlcode = 1 /* ASSUME GUITY */;
	{
		FILE *_adl_load = fopen("./test", "rt");
		char _adl_load_buf[40960], *tok;
		char loadkeybuf[1], loaddatabuf[17];
		int _adl_line_no=0;
		char bPoint =0;
		if (!_adl_load) {
			printf("can not open file ./test.\n");
			exit(1);
		}
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));
		key.data = loadkeybuf;
		data.data = loaddatabuf;
		key.size = 0;
		data.size = 16;
		while (fgets(_adl_load_buf, 40959, _adl_load)) {
			_adl_line_no++;
			tok = csvtok(_adl_load_buf, ",\n");
			if (!tok) {
				printf("data format error at line %d\n", _adl_line_no);
				goto exit;
			}
			*(int*)((char*)data.data+0) = atoi(tok);
			tok = csvtok(NULL, ",\n");
			if (!tok) {
				printf("data format error at line %d\n", _adl_line_no);
				goto exit;
			}
			memset((char*)data.data+4, 0, 8);
			memcpy((char*)data.data+4, &getTimeval(tok), 8);
			if ((rc = traffic->put(traffic, &key, &data, DB_APPEND))!=0) {
				exit(rc);
			}
		} /* end of while */
		fclose(_adl_load);
	}
	_adl_sqlcode = _adl_cursqlcode;
exit:
	return rc;
};
int _adl_statement_15()
{
	int rc = 0;
	DBT key, data, windata;
	struct timeval atime;
	Rect r_key;
	char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
	char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
	int _rec_id=0; /* recursive id */
	char _adl_dbname[MAX_STR_LEN];
	int slide_out = 1;
	winbuf* rwinbuf = NULL;
	int rlast_out = 0;
	static int last_out = 0;
	static bool iterate = false;
	static bool init = true;
	char _timeexpkey[MAX_STR_LEN];
	char *timeexpkey=_timeexpkey;
	struct {
		int field_0;
		int field_0_expire;
		struct timeval atime;
	} insert_8;
	IM_RELC *traffic_14;
	int first_entry_13 = 1;
	int first_entry_14 = 1;
	int index_12 = 0;
	int terminating_12=0;
	struct gb_status_12 {
		int _baggr_0_value;
		int _baggr_0_first_entry;
		int sum_0_last_out;
		bool sum_0_iterate;
		bool sum_0_init;
		int _baggr_1_value;
		int _baggr_1_first_entry;
		int count_1_last_out;
		bool count_1_iterate;
		bool count_1_init;
	};
	struct gb_status_12 *gbstatus_12 = (struct gb_status_12 *)0;

	int first_entry_12 = 1;
	int first_entry_10 = 1;
	rc = 0;
	_adl_cursqlcode = 1 /* ASSUME GUITY */;
	if ((rc = traffic->cursor(traffic, &traffic_14, 0)) != 0) {
		adlabort(rc, "IM_REL->cursor()");
	}
	while (rc==0) {
		struct {
			int a_0;
			int a_1;
			int a_0_expire;
			int a_1_expire;
			struct timeval atime;
		} Q_0003_10_9;
next_10:
		struct {
			int a_0;
			int a_1;
			int a_0_expire;
			int a_1_expire;
			struct timeval atime;
		} Q_0002_12_11;
next_12:
		while (index_12>=0 && index_12 < 3) {
			switch(index_12) {
				case 0:
					{
						if (terminating_12 == 0) {
							/* get source tuple from qun */
							struct {
								int in1;
								struct timeval time1;
								int OID;
								int in1_expire;
								struct timeval time1_expire;
								int OID_expire;
								struct timeval atime;
							} traffic_14_13;
next_14:
							memset(&key, 0, sizeof(key));
							memset(&data, 0, sizeof(data));
							rc = traffic_14->c_get(traffic_14, &key, &data, (first_entry_13)? DB_FIRST:DB_NEXT);
							if (rc==0) {
								first_entry_13 = 0;
								memcpy(&(traffic_14_13.in1), (char*)data.data+0, sizeof(int));
								//printf("Retrieved traffic_14_13.in1 = %d\n", traffic_14_13.in1);
								//fflush(stdout);
								memcpy(&(traffic_14_13.time1), (char*)data.data+4, sizeof(struct timeval));
								memcpy(&(traffic_14_13.OID), (char*)data.data+12, sizeof(int));
								//printf("Retrieved traffic_14_13.OID = %d\n", traffic_14_13.OID);
								//fflush(stdout);
							} else if (rc == DB_NOTFOUND) {
								first_entry_13 = 1;
							} else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
							if (rc==0) {
								Q_0002_12_11.a_0 = traffic_14_13.in1;
								Q_0002_12_11.a_1 = 1;
							} /* if (rc == 0) */
							if (rc==0) {
								first_entry_12 = 0;
								/* make assignments of non-aggr head expr */
								/* merge group-by columns into a key */
								strcpy(gbkey, "____");
								gbstatus_12 = (struct gb_status_12 *)0;
								rc = hash_get(12, _rec_id, gbkey, 4, (char**)&gbstatus_12);
								if (rc == DB_NOTFOUND) {//blah
									gbstatus_12 = (struct gb_status_12*)malloc(sizeof(*gbstatus_12));
									gbstatus_12->_baggr_0_first_entry = 1;
									gbstatus_12->_baggr_0_value =  Q_0002_12_11.a_0;
									gbstatus_12->_baggr_1_first_entry = 1;
									gbstatus_12->_baggr_1_value = 1;
									rc = hash_put(12, _rec_id, gbkey, 4, &gbstatus_12);
								} else if (rc == 0) {
									/* PHASE iterate */
									gbstatus_12->_baggr_0_first_entry = 1;
									gbstatus_12->_baggr_0_value +=  Q_0002_12_11.a_0;
									gbstatus_12->_baggr_1_first_entry = 1;
									gbstatus_12->_baggr_1_value += 1;
								} else adlabort(rc, "hash->get()");
							} else if (rc == DB_NOTFOUND) {
								terminating_12 = 1;
							}
						}
						if (terminating_12 == 1) {
							if (first_entry_12 == 1) {
								rc = 0; /* fail on first entry, aggregate on empty set */
							} else {
								allkey = (char*)0;
								rc = hash_get(12, _rec_id, allkey, 4, (char**)&gbstatus_12);
								if (rc==0) {
								} else if(rc == DB_NOTFOUND) {
								} else adlabort(rc, "hash->get()");
							}
						}
					}
					break;
				case 1:
					{
						rc = DB_NOTFOUND;
						if (terminating_12 == 1) {
							if (gbstatus_12 == (struct gb_status_12 *)0) {
								if (first_entry_12) {
									rc = 0;
									Q_0003_10_9.a_0 = 0;
								}
							} else 
								if (gbstatus_12->_baggr_0_first_entry == 1) {
									Q_0003_10_9.a_0 = gbstatus_12->_baggr_0_value;
									gbstatus_12->_baggr_0_first_entry = 0;
									rc = 0;
								} else {
									gbstatus_12->_baggr_0_first_entry = 1;
								}
						}
					}
					break;
				case 2:
					{
						rc = DB_NOTFOUND;
						if (terminating_12 == 1) {
							if (gbstatus_12 == (struct gb_status_12 *)0) {
								if (first_entry_12) {
									rc = 0;
									Q_0003_10_9.a_1 = 0;
								}
							} else 
								if (gbstatus_12->_baggr_1_first_entry == 1) {
									Q_0003_10_9.a_1 = gbstatus_12->_baggr_1_value;
									gbstatus_12->_baggr_1_first_entry = 0;
									rc = 0;
								} else {
									gbstatus_12->_baggr_1_first_entry = 1;
								}
						}
						first_entry_12 = 0;
					}
					break;
			} /*end of switch*/
			if (rc == 0) {
				index_12++;
			}
			if (rc == DB_NOTFOUND) {
				index_12--;
				if (terminating_12 == 1 && index_12 == 0) {
					rc = DB_NOTFOUND;
				}
			}
		}/*end of while */
		if (rc == 0) index_12--;
		else 
		{
			int rc;		/* local rc */ 
			terminating_12 = 0;
			first_entry_12 = 1;
			index_12 = 0;
			/* free gbstatus */
			do {
				allkey = (char*)0;
				rc = hash_get(12, _rec_id, allkey, 4, (char**)&gbstatus_12);
				if (rc==0) {
					//printf("freeing 12\n");
					free(gbstatus_12);
				}
			} while (rc==0);
			if (rc != DB_NOTFOUND) {
				adlabort(rc, "hash->get()");
			}
			/* release hash entry */
			hashgb_delete(12, _rec_id);
		}
		if (rc==0) {
			insert_8.field_0 = ((Q_0003_10_9.a_0) / Q_0003_10_9.a_1);
		} /* if (rc == 0) */
		if (rc ==0 && slide_out == 1) {
			_adl_cursqlcode = 0; /* SUCCESS */ 
			/* INSERT STARTS */
			printf("%10d ", insert_8.field_0);
			printf("\n");
			/* INSERT ENDS */
		} else 
			slide_out = 1;
	} /* while (rc==0) */
	if (traffic_14 && (rc = traffic_14->c_close(traffic_14)) != 0) {
		adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
	}
	_adl_sqlcode = _adl_cursqlcode;
exit:
	return rc;
};
int main()
{
	int rc;
	tempdb_init();
	hashgb_init();
	_adl_dlm_init();
	// Initialization of Declarations
	if ((rc = im_rel_create(&traffic, NULL, IM_LINKEDLIST, 0)) != 0) {
		adlabort(rc, "im_rel_create()");
	}
	if ((rc = traffic->open(traffic, "_adl_db_traffic", 0)) != 0) {
		adlabort(rc, "open()");
	}
	if (inMemTables->count("traffic") == 0) {
		inMemTables->operator[](strdup("traffic")) = traffic;
	}
	_adl_statement_7();
	_adl_statement_15();
exit:
	tempdb_delete();
	_adl_dlm_delete();

	if ((rc = traffic->close(traffic, 0)) != 0) {
		adlabort(rc, "IM_REL->close()");
	}
	return(rc);
};
