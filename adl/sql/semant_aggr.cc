#include <sql/adl_sys.h>
#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/symbol.h>
#include <sql/trans2C.h>
#include <sql/io.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "util.h"
#include "aggr_info.h"
#include <math.h>

#include <vector>
using namespace std;

extern "C" {
#include <dbug/dbug.h>
}

extern system_t *ntsys;
/* 
   Generate code for calling
   aggr_init, aggr_iterate, aggr_expire, aggr_terminate
   routines.
 */
err_t transCallBuiltInAggr(A_sqlopr a, /* gb node */
		Aggr_info_t aggr_info, int i, int routine, char *buf) {
	err_t rc = ERR_NONE;
	char acall[MAX_STR_LEN];
	char value[80];
	A_exp aggr = aggr_info->aggr;
	A_win win = aggr->u.call.win;
	char fname[256];

	int winsize;
	A_slide slide;
	int slidesize = 1;

	if (win != (A_win) 0) {
		winsize = win->range->size;
		slide = win->slide;
		if (slide)
			slidesize = slide->size;
	}

	sprintf(fname, "%s", S_name(aggr->u.call.func));

	if (routine == AGGR_TERMINATE) {
		// builtin aggrs do not have terminate routines
		goto exit;
	}
	//This is supported now
	//if (win && win->range && win->range->size != -1){
	//EM_error(ERR_TO_BE_IMPLEMENTED, __LINE__,__FILE__,"built-in aggr on windows");
	//goto exit;
	//}
	strcpy(value, aggr_info->argbuf + 1);

	sprintf(acall, "\ngbstatus_%d->_baggr_%d_first_entry = 1;", UID(a), i);
	strcat(buf, acall);

	if (routine == AGGR_INIT || routine == AGGR_ITERATE) {
		if (win && win->range && win->range->size != -1 && win->range->type
				== COUNT_RANGE) {
			sprintf(acall, "\ngbstatus_%d->%s_%d_win->updateTupleID();", UID(a),
					fname, i);
			strcat(buf, acall);
		} else if (win && win->range && win->range->size != -1) {
			sprintf(acall, "\ngbstatus_%d->%s_%d_win->updateTupleID(&atime);",
					UID(a), fname, i);
			strcat(buf, acall);
		}
		if (win && win->range && win->range->size != -1) {
			strcat(buf, aggr_info->windata_code);
			strcat(buf, aggr_info->winkey_code);
			sprintf(acall, "\ngbstatus_%d->%s_%d_win->put(&windata);", UID(a), fname,
					i);
			strcat(buf, acall);
		}
	}

	if (routine == AGGR_INIT) {

		switch (aggr_info->builtin) {
			case AGGR_BUILTIN_AVG:
			case AGGR_BUILTIN_MAX:
			case AGGR_BUILTIN_MIN:
			case AGGR_BUILTIN_MAXR:
			case AGGR_BUILTIN_MINR:
			case AGGR_BUILTIN_SUMR:
			case AGGR_BUILTIN_SUM:
				sprintf(acall, "\ngbstatus_%d->_baggr_%d_value = %s;", UID(a), i, value);
				break;
			case AGGR_BUILTIN_COUNT:
			case AGGR_BUILTIN_COUNTR:
				sprintf(acall, "\ngbstatus_%d->_baggr_%d_value = 1;", UID(a), i);
				break;
			case AGGR_BUILTIN_XA: // XMLAgg
				sprintf(acall, "\nstrcpy(gbstatus_%d->_baggr_%d_value, %s);", UID(a), i,
						value);
				break;
			case AGGR_BUILTIN_VAR:
				sprintf(acall, "\ngbstatus_%d->_baggr_%d_value = 0;"
						"\ngbstatus_%d->_baggr_%d_value_sum = %s;"
						"\ngbstatus_%d->_baggr_%d_value_cnt = 1;"
						"\ngbstatus_%d->_baggr_%d_value_avg = %s;", UID(a), i, UID(a), i,
						value, UID(a), i, UID(a), i, value);
				break;
		} // end switch
		strcat(buf, acall);

	} else if (routine == AGGR_ITERATE) {
		if (win && win->range && win->range->size != -1) {
			sprintf(acall, "\nwhile (gbstatus_%d->%s_%d_win->hasExpired()) {"
					"\ngbstatus_%d->%s_%d_win->getExpired(&windata);"
					"%s", UID(a), fname, i, UID(a), fname, i, aggr_info->expire_code);
			strcat(buf, acall);

			switch (aggr_info->builtin) {
				case AGGR_BUILTIN_SUM:
				case AGGR_BUILTIN_SUMR:
					sprintf(acall, "\ngbstatus_%d->_baggr_%d_value -= %s_expire;", UID(a),
							i, value);//ae->exp);
					strcat(buf, acall);
					break;
				case AGGR_BUILTIN_COUNT:
				case AGGR_BUILTIN_COUNTR:
					sprintf(acall, "\ngbstatus_%d->_baggr_%d_value -= 1;", UID(a), i);
					strcat(buf, acall);
					break;
					/*smart window management, thus nothing to do for expire, just pop*/
				case AGGR_BUILTIN_MINR:
				case AGGR_BUILTIN_MIN:
					break;
				case AGGR_BUILTIN_MAXR:
				case AGGR_BUILTIN_MAX:
					break;
				case AGGR_BUILTIN_AVG:
					rc = ERR_NTSQL_INTERNAL;
					EM_error(0, rc, __LINE__, __FILE__, "aggregate avg encounted");
					goto exit;
				case AGGR_BUILTIN_VAR:
					sprintf(
							acall,
							"\nif (gbstatus_%d->_baggr_%d_value_cnt > 1) {"
							"\n\tgbstatus_%d->_baggr_%d_value -= pow(%s_expire - gbstatus_%d->_baggr_%d_value_avg, 2) * gbstatus_%d->_baggr_%d_value_cnt / (gbstatus_%d->_baggr_%d_value_cnt - 1);"
							"\n\tgbstatus_%d->_baggr_%d_value_sum -= %s_expire;"
							"\n\tgbstatus_%d->_baggr_%d_value_cnt -= 1;"
							"\n\tgbstatus_%d->_baggr_%d_value_avg = gbstatus_%d->_baggr_%d_value_sum / gbstatus_%d->_baggr_%d_value_cnt;"
							"\n} else {"
							"\n\tgbstatus_%d->_baggr_%d_value = 0;"
							"\n\tgbstatus_%d->_baggr_%d_value_sum = 0;"
							"\n\tgbstatus_%d->_baggr_%d_value_cnt = 0;"
							"\n\tgbstatus_%d->_baggr_%d_value_avg = 0;"
							"\n}", UID(a), i, UID(a), i, value, UID(a), i, UID(a), i, UID(a),
							i, UID(a), i, value, UID(a), i, UID(a), i, UID(a), i, UID(a), i,
							UID(a), i, UID(a), i, UID(a), i, UID(a), i);
					strcat(buf, acall);
					break;
			} // end switch

			sprintf(acall, "\ngbstatus_%d->%s_%d_win->pop();"
					"\n}", UID(a), fname, i);
			strcat(buf, acall);
		}

		switch (aggr_info->builtin) {
			case AGGR_BUILTIN_SUM:
			case AGGR_BUILTIN_SUMR:
				sprintf(acall, "\ngbstatus_%d->_baggr_%d_value += %s;", UID(a), i, value);//ae->exp);

				break;
			case AGGR_BUILTIN_COUNT:
			case AGGR_BUILTIN_COUNTR:
				sprintf(acall, "\ngbstatus_%d->_baggr_%d_value += 1;", UID(a), i);
				break;
			case AGGR_BUILTIN_MINR:
				/* here if we have window, we go over the window and delete larger */
				/* we return last of the window */
				if (win && win->range && win->range->size != -1) {
					sprintf(
							acall,
							"\ngbstatus_%d->%s_%d_win->deleteGreaterReal(%s);"
							"\ngbstatus_%d->_baggr_%d_value = gbstatus_%d->%s_%d_win->getHeadReal();",
							UID(a), fname, i, value, UID(a), i, UID(a), fname, i);
				} else {
					sprintf(acall, "\nif (gbstatus_%d->_baggr_%d_value > %s) {"
							"\ngbstatus_%d->_baggr_%d_value = %s;"
							"\n}", UID(a), i, value, UID(a), i, value);
				}
				break;
			case AGGR_BUILTIN_MIN:
				/* here if we have window, we go over the window and delete larger */
				/* we return last of the window */
				if (win && win->range && win->range->size != -1) {
					sprintf(
							acall,
							"\ngbstatus_%d->%s_%d_win->deleteGreaterInt(%s);"
							"\ngbstatus_%d->_baggr_%d_value = gbstatus_%d->%s_%d_win->getHeadInt();",
							UID(a), fname, i, value, UID(a), i, UID(a), fname, i);
				} else {
					sprintf(acall, "\nif (gbstatus_%d->_baggr_%d_value > %s) {"
							"\ngbstatus_%d->_baggr_%d_value = %s;"
							"\n}", UID(a), i, value, UID(a), i, value);
				}
				break;
			case AGGR_BUILTIN_MAXR:
				/* here if we have window, we go over the window and delete lesser */
				/* we return last of the window */
				if (win && win->range && win->range->size != -1) {
					sprintf(
							acall,
							"\ngbstatus_%d->%s_%d_win->deleteLesserReal(%s);"
							"\ngbstatus_%d->_baggr_%d_value = gbstatus_%d->%s_%d_win->getHeadReal();",
							UID(a), fname, i, value, UID(a), i, UID(a), fname, i);
				} else {
					sprintf(acall, "\nif (gbstatus_%d->_baggr_%d_value < %s) {"
							"\ngbstatus_%d->_baggr_%d_value = %s;"
							"\n}", UID(a), i, value, UID(a), i, value);
				}
				break;
			case AGGR_BUILTIN_MAX:
				/* here if we have window, we go over the window and delete lesser */
				/* we return last of the window */
				if (win && win->range && win->range->size != -1) {
					sprintf(
							acall,
							"\ngbstatus_%d->%s_%d_win->deleteLesserInt(%s);"
							"\ngbstatus_%d->_baggr_%d_value = gbstatus_%d->%s_%d_win->getHeadInt();",
							UID(a), fname, i, value, UID(a), i, UID(a), fname, i);
				} else {
					sprintf(acall, "\nif (gbstatus_%d->_baggr_%d_value < %s) {"
							"\ngbstatus_%d->_baggr_%d_value = %s;"
							"\n}", UID(a), i, value, UID(a), i, value);
				}
				break;
			case AGGR_BUILTIN_AVG:
				rc = ERR_NTSQL_INTERNAL;
				EM_error(0, rc, __LINE__, __FILE__, "aggregate avg encounted");
				goto exit;
			case AGGR_BUILTIN_XA: // XMLAgg
				sprintf(acall, "\nstrcat(gbstatus_%d->_baggr_%d_value,%s);", UID(a), i,
						value);//ae->exp);

				break;
			case AGGR_BUILTIN_VAR:
				sprintf(
						acall,
						"\ngbstatus_%d->_baggr_%d_value += pow(%s - gbstatus_%d->_baggr_%d_value_avg, 2) * gbstatus_%d->_baggr_%d_value_cnt / (gbstatus_%d->_baggr_%d_value_cnt + 1);"
						"\ngbstatus_%d->_baggr_%d_value_sum += %s;"
						"\ngbstatus_%d->_baggr_%d_value_cnt += 1;"
						"\ngbstatus_%d->_baggr_%d_value_avg = gbstatus_%d->_baggr_%d_value_sum / gbstatus_%d->_baggr_%d_value_cnt;",
						UID(a), i, value, UID(a), i, UID(a), i, UID(a), i, UID(a), i, value,
						UID(a), i, UID(a), i, UID(a), i, UID(a), i);
				break;
		} // end switch
		strcat(buf, acall);

		// TODO(nlaptev): Verify that the heartbeat modification needs to only be 
		// implemented below and not in another place where similar logic is in place.
		if (win && win->range && win->range->size != -1) {
			// If we specified a time=slide to also produce heartbeats, then
			// we enable the heartbeat functionality.
			char heartbeat_check[300];
			if (isESL() && win->slide->type == HEARTBEAT_TIME_SLIDE) { 
				sprintf(heartbeat_check, "\nFLAGS_enable_heartbeat = true;"
						"\nif (FLAGS_enable_heartbeat && (int)gbstatus_%d->%s_%d_win->getTupleID() >= gbstatus_%d->%s_%d_last_out + %d) {"
						"\ngbstatus_%d->%s_%d_last_out = gbstatus_%d->%s_%d_last_out + %d;"
						"\nslide_out = 1;"
						"\n} else {"
						"\nslide_out = 0;"
						"\n}", UID(a), fname, i, UID(a), fname, i, slidesize, UID(a), fname, i, UID(a), fname, i, slidesize);	
			} else {
				sprintf(heartbeat_check, "slide_out = 0;");
			}
			sprintf(
					acall,
					"\nif((!(%d <= 1 || ((gbstatus_%d->%s_%d_win->getTupleID() == 0 && gbstatus_%d->%s_%d_last_out != 0) "
					"\n||((((int)gbstatus_%d->%s_%d_win->getTupleID()) >= gbstatus_%d->%s_%d_last_out + %d)))))) {"
					"\n%s"
					"\n//printf(\"Here no output %%d %d %%d\\n\", gbstatus_%d->%s_%d_last_out, gbstatus_%d->%s_%d_win->getTupleID());fflush(stdout);"
					"\n} else {"
					"\nslide_out = 1;"
					"\n//printf(\"Here YES output %%d %d %%d\\n\", gbstatus_%d->%s_%d_last_out, gbstatus_%d->%s_%d_win->getTupleID());fflush(stdout);"
					"\ngbstatus_%d->%s_%d_last_out = gbstatus_%d->%s_%d_last_out + %d;"
					"\nwhile(gbstatus_%d->%s_%d_last_out < (gbstatus_%d->%s_%d_win->getTupleID() - %d) && gbstatus_%d->%s_%d_win->getTupleID() > 0) {"
					"\nif(%d == 1) {"
					"\ngbstatus_%d->%s_%d_last_out = gbstatus_%d->%s_%d_win->getTupleID();"
					"\n}"
					"\nelse {"
					"\ngbstatus_%d->%s_%d_last_out = gbstatus_%d->%s_%d_last_out + %d;"
					"\n}"
					"\n}"
					"\n}", slidesize, UID(a), fname, i, UID(a), fname, i, UID(a),
					fname, i, UID(a), fname, i, slidesize,
					heartbeat_check,
					slidesize, UID(a), fname, i,
					UID(a), fname, i, slidesize, UID(a), fname, i, UID(a), fname, i,
					UID(a), fname, i, UID(a), fname, i, slidesize, UID(a), fname, i,
					UID(a), fname, i, slidesize, UID(a), fname, i, slidesize, UID(a),
					fname, i, UID(a), fname, i, UID(a), fname, i, UID(a), fname, i,
					slidesize);
			strcat(buf, acall);
		}
	} // end else if ITERATE
	else if (routine == AGGR_EXPIRE) {
	}
exit: return rc;
}

vector<void*> S_endScopeval(S_table);
int aggrDefinedInsideAggr(char* fname, S_table venv) {
	int ret = 0;

	vector<void*> first = S_endScopeval(venv);
	vector<void*> second = S_endScopeval(venv);
	vector<void*> third = S_endScopeval(venv);
	vector<void*> localVars = S_endScopeval(venv);
	int size = localVars.size();
	S_beginScope(venv);
	for (int i = 0; i < size; i++) {
		E_enventry e = (E_enventry) localVars.operator[](i);
		S_enter(venv, e->key, e);
		if (e->kind == E_aggrEntry) {
			if (strcmp(S_name(e->key), fname) == 0)
				ret = 1;
		}
	}

	int thirdsize = third.size();
	S_beginScope(venv);
	for (int i = 0; i < thirdsize; i++) {
		E_enventry e = (E_enventry) third.operator[](i);
		S_enter(venv, e->key, e);
	}

	int secondsize = second.size();
	S_beginScope(venv);
	for (int i = 0; i < secondsize; i++) {
		E_enventry e = (E_enventry) second.operator[](i);
		S_enter(venv, e->key, e);
	}

	int firstsize = first.size();
	S_beginScope(venv);
	for (int i = 0; i < firstsize; i++) {
		E_enventry e = (E_enventry) first.operator[](i);
		S_enter(venv, e->key, e);
	}

	return ret;
}

char* getOuterAggrsArgs(char* fname, S_table venv, vector<void*> aggregates) {
	char ret[500];
	int size = aggregates.size();
	ret[0] = (char) NULL;
	for (int i = 0; i < size; i++) {
		S_symbol name = (S_symbol) aggregates.operator[](i);
		strcat(ret, S_name(name));
		strcat(ret, "_Inst, ");
	}
	if (aggrDefinedInsideAggr(fname, venv) == 1)
		strcat(ret, "status, ");
	return strdup(ret);
}

char* getArgTypes(S_table venv, char* fname, vector<void*> aggregates) {
	char ret[500];
	ret[0] = (char) NULL;
	E_enventry e = (E_enventry) S_look(venv, S_Symbol(fname));
	A_list argsTys = e->u.fun.formals;
	int size = argsTys->length;

	strcat(ret, "void*, ");

	for (int i = 0; i < size; i++) {
		Ty_field fi = (Ty_field) getNthElementList(argsTys, i);

		switch (fi->ty->kind) {
			case Ty_int:
				strcat(ret, "int");
				break;
			case Ty_real:
				strcat(ret, "double");
				break;
			case Ty_string:
				strcat(ret, "char*");
				break;
			case Ty_timestamp:
				strcat(ret, "struct timeval");
				break;
			case Ty_iext:
				strcat(ret, "struct iExt_");
				break;
			case Ty_rext:
				strcat(ret, "struct rExt_");
				break;
			case Ty_cext:
				strcat(ret, "struct cExt_");
				break;
			case Ty_text:
				strcat(ret, "struct tExt_");
				break;
		}

		if (i < size - 1)
			strcat(ret, ", ");
		else
			strcat(ret, ", int");
	}

	/*int aggrSize = aggregates.size();
	  for(int i = 0; i < aggrSize; i++)
	  {
	  S_symbol name = (S_symbol)aggregates.operator[](i);
	  strcat(ret, ", ");
	  strcat(ret, S_name(name));
	  }
	  if(aggrDefinedInsideAggr(fname, venv) == 1)
	  {
	  strcat(ret, ", ");
	  strcat();
	  }*/

	return strdup(ret);
}

err_t transCallUDA(A_sqlopr a, /* gb node */
		Sql_sem sql, Aggr_info_t aggr_info, int i, int routine, int defined_routines,
		char *buf, vector<void*> aggregates, S_table venv, int keysize) {

	DBUG_ENTER("transCallUDA");
	err_t rc = ERR_NONE;
	char fname[80], acall[MAX_STR_LEN];
	char dlOpen[MAX_STR_LEN];
	char dlClose[MAX_STR_LEN];
	char *routine_name[] = { "", "init", "iterate", "", "terminate" };
	char *outerAggrs;
	char *argTypes;
	char modelId[200];
	char fetchModelId[200];
	sprintf(fetchModelId, "getModelId()");
	dlOpen[0] = '\0';

	A_exp aggr = aggr_info->aggr;
	A_win win = aggr->u.call.win;
	int winsize;
	A_slide slide;
	int slidesize = 1;

	if (win != (A_win) 0) {
		winsize = win->range->size;
		slide = win->slide;
		if (slide)
			slidesize = slide->size;
	}

	char *argbuf;
	bool ptlist = false;
	char ptlistname[256];
	ptlistname[0] = '\0';
	sprintf(ptlistname, ", NULL");

	strcpy(fname, S_name(aggr->u.call.func));

	//printf("Calling %s %d\n", fname, win);

	argbuf = aggr_info->argbuf;

	/*
	 * The special case of terminate routine: Normally, the
	 * terminate routine should *not* see the data that were
	 * aggregated upon -- because the actual aggregation is done,
	 * and the task of the terminate routine is to return final
	 * results. However, parameters not from the table undergoing
	 * aggregation should be visible to the terminate routine. As a
	 * matter of fact, they are constants in terms of the
	 * aggregation. Right now, we don't make the distinction, and
	 * leave it to users' discretion (He should not use table column
	 * variables in the terminate routine).
	 *  */
#ifdef TERMINATE_NO_ARGS
	if (routine == AGGR_TERMINATE) {
		argbuf = '\0';
	}
#endif
	E_enventry e = (E_enventry) S_look(venv, S_Symbol(fname));
	if (e && e->u.fun.inaggr == 1) {
		outerAggrs = getOuterAggrsArgs(fname, venv, aggregates);
		if (outerAggrs[0] == 0) {
			strcat(outerAggrs, "status, ");
		}
	} else {
		outerAggrs = "";
	}

	if (isESL() || isAdHoc()) {
		argTypes = getArgTypes(venv, fname, aggregates);
	}

	//the modelName should always be correct, so we set all the time
	if (e && e->u.fun.inaggr != 1) {
		sprintf(modelId, "\nsetModelId(\"%s\");", getModelName());
		strcat(buf, modelId);
	} else if (e && e->u.fun.inaggr == 1) {
		sprintf(fetchModelId, "_modelId");
	}

	// here we check if it is a windowed call, slidesize is not 1
	// slide size != winsize, winsize is a multiple of slidesize
	// there exists a base version of the uda
	// in this case, we must write two statements,
	// one inwhich base aggregate is called for every slide size tuples and
	//   result is fed to a temp buf -- pretty hard to do
	// another in which window aggregate is called over this new buf
	// with winsize equal to winsize/slidesize -- easier, but still hard
	//both of these statements must be put in the driver and executed properly
	//question is do we put these statements in separate files or
	//same file, if we put in same file the query scheduler must know the
	// location somehow. We will also have to register both statements
	// with queryScheduler/driver. This can probably be done with a
	// special statement that simply has list of nStmts under it.

	if ((isESL() || isAdHoc()) && e->u.fun.inaggr != 1 && e->u.fun.default_win
			&& routine == AGGR_INIT) {

		//strip window from aggr name, because it is in base file
		char* base_fName = strdup(fname);
		char* under_win = strstr(fname, "_window");
		base_fName[under_win - fname] = '\0';
		sprintf(dlOpen,
				"\n static void *handle_%d=NULL;"
				//static for performance reasons
				"\nif(!handle_%d) {"
				"\nhandle_%d= dlopen(\"../exe/%s.so\", RTLD_NOW);"
				"\nif(!handle_%d) {"
				"\nchar errorMsg[1024];"
				"\nsprintf(errorMsg, \"dl error at open: %%s\\n\", dlerror());"
				"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
				"\nreturn s_failure;\n}\n}", UID(a), UID(a), UID(a), base_fName,
				UID(a), getUserName());
		sql->predec = expTy_Seq(sql->predec, dlOpen);

		sprintf(dlClose, "\n//dlclose(handle_%d);", UID(a));
		sql->afterpreclean = expTy_Seq(sql->afterpreclean, dlClose);

		//strcat(buf, dlOpen);
	}
	//don't see the use of this else if
	else if (((isESL() || isAdHoc()) && routine == AGGR_INIT)) {
		char ffname[80];
		sprintf(ffname, fname);
		ffname[strlen(ffname) - strlen("_window")] = '\0';
		E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

		/*if (slide && slide->size >= winsize && winsize != -1 &&
		  base && base->kind == E_aggrEntry) {
		  sprintf(dlOpen, "\n static void *handle_%d=NULL;"
		//static for performance reasons
		"\nif(!handle_%d) {"
		"\nhandle_%d= dlopen(\"../exe/%s.so\", RTLD_NOW);"
		"\nif(!handle_%d) {"
		"\nchar errorMsg[1024];"
		"\nsprintf(errorMsg, \"dl error at open: %%s\\n\", dlerror());"
		"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
		"\nreturn s_failure;\n}\n}"
		, UID(a), UID(a), UID(a), ffname, UID(a), getUserName());
		}
		else {*/
		sprintf(dlOpen,
				"\n static void *handle_%d=NULL;"
				//static for performance reasons
				"\nif(!handle_%d) {"
				"\nhandle_%d= dlopen(\"../exe/%s.so\", RTLD_NOW);"
				"\nif(!handle_%d) {"
				"\nchar errorMsg[1024];"
				"\nsprintf(errorMsg, \"dl error at open: %%s\\n\", dlerror());"
				"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
				"\nreturn s_failure;\n}\n}", UID(a), UID(a), UID(a), fname, UID(a),
				getUserName());
		//}
		sql->predec = expTy_Seq(sql->predec, dlOpen);

		sprintf(dlClose, "\n//dlclose(handle_%d);", UID(a));
		sql->afterpreclean = expTy_Seq(sql->afterpreclean, dlClose);
		//strcat(buf, dlOpen);
	}

	if (win != (A_win) 0) {
		char ffname[80];
		sprintf(ffname, fname);
		ffname[strlen(ffname) - strlen("_window")] = '\0';
		// window aggregate
		A_slide slide = win->slide;

		ptlist = (win->range->type == TIME_RANGE && win->partition_list
				!= (A_list) 0 && !A_ListEmpty(win->partition_list));

		//lookup base version
		E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

		// put the data to window
		strcat(buf, aggr_info->windata_code);
		strcat(buf, aggr_info->winkey_code);
		if (win->range->type == COUNT_RANGE) { // count-based
			if (slide && slide->size >= winsize && winsize != -1 && base
					&& base->kind == E_aggrEntry) {
				sprintf(acall, "\ngbstatus_%d->%s_%d->win->updateTupleID();", UID(a),
						ffname, i);
			} else {
				sprintf(acall, "\ngbstatus_%d->%s_%d->win->updateTupleID();", UID(a),
						fname, i);
			}
			strcat(buf, acall);
		} else if (winsize != -1) { // timebase and not unlimited (signified by -1)
			if (slide && slide->size >= winsize && winsize != -1 && base
					&& base->kind == E_aggrEntry) {
				sprintf(acall, "\ngbstatus_%d->%s_%d->win->updateTupleID(&atime);",
						UID(a), ffname, i);
			} else {
				sprintf(acall, "\ngbstatus_%d->%s_%d->win->updateTupleID(&atime);",
						UID(a), fname, i);
			}
			strcat(buf, acall);
		}

		if (slide && slide->size >= winsize && winsize != -1 && base && base->kind
				== E_aggrEntry) {
			sprintf(acall, "\nslide_out = 1;");
			strcat(buf, acall);

			sprintf(
					acall,
					"\nif(gbstatus_%d->%s_%d->win->getTupleID() > gbstatus_%d->%s_%d->last_out + %d - %d) {"
					"\ngbstatus_%d->%s_%d->win->put(&windata);"
					"\n}", UID(a), ffname, i, UID(a), ffname, i, slide->size, winsize,
					UID(a), ffname, i);
			strcat(buf, acall);
		}

		//printf("here %d %d %d %d %s %s %d\n", slide, winsize, base, base?base->kind:-1, ffname, getQueryName(), slide?slide->size:-1);
		if (slide && slide->size >= winsize && winsize != -1 && base && base->kind
				== E_aggrEntry) {
			ptlist = 0;
			//change the call to base version
			//set defined_routines accordingly
			defined_routines = base->u.fun.aggr_routines;
			sprintf(
					acall,
					"\nif(gbstatus_%d->%s_%d->win->getTupleID() <= gbstatus_%d->%s_%d->last_out + %d - %d) {"
					"\n//do nothing"
					"\n//printf(\"doing nothing %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
					"\n//index_%d++;"
					"\nslide_out = 0;"
					"\ngbstatus_%d->%s_%d->init = true;"
					"\n}"
					"\nelse if(gbstatus_%d->%s_%d->win->getTupleID() >= gbstatus_%d->%s_%d->last_out + %d && !gbstatus_%d->%s_%d->init) {"
					"\ngbstatus_%d->%s_%d->last_out = gbstatus_%d->%s_%d->last_out + %d;"
					"\n//while(gbstatus_%d->%s_%d->last_out < (gbstatus_%d->%s_%d->win->getTupleID()-%d)) {"
					"\n//gbstatus_%d->%s_%d->last_out = gbstatus_%d->%s_%d->last_out + %d;"
					"\n//}"
					"\ngbstatus_%d->%s_%d->iterate = false;"
					"\ngbstatus_%d->%s_%d->init = true;", UID(a), ffname, i, UID(a),
					ffname, i, slide->size, winsize, UID(a), ffname, i, UID(a), ffname,
					i, UID(a), UID(a), ffname, i, UID(a), ffname, i, UID(a), ffname, i,
					slide->size, UID(a), ffname, i, UID(a), ffname, i, UID(a), ffname, i,
					slide->size, UID(a), ffname, i, UID(a), ffname, i, slide->size,
					UID(a), ffname, i, UID(a), ffname, i, slide->size, UID(a), ffname, i,
					UID(a), ffname, i);
			strcat(buf, acall);

			//here we should see if isESL or not and do accordingly
			if (defined_routines & AGGR_INIT_ITERATE) {
				if (isESL() || isAdHoc()) {
					sprintf(
							acall,
							"\nvoid (*%s_init) (%s, int, bufferMngr*,"
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* ,"
							"\n\tvector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_init\"));"
							"\nif(!%s_init) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\n//printf(\"iterating1 %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
							"\n(*(%s_init))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0, bm, inMemTables%s, slide_out, %s);",
							ffname, argTypes, argTypes, UID(a), ffname, ffname,
							getUserName(), UID(a), ffname, i, UID(a), ffname, i, ffname,
							outerAggrs, UID(a), ffname, i, argbuf, ptlist ? ptlistname
							: ", NULL", fetchModelId);
				} else {
					sprintf(acall, "\n%s_init(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0",
							ffname, outerAggrs, UID(a), ffname, i, argbuf);

					if (isESLAggr()) {
						strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					} else {
						strcat(acall, ", NULL, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					}
				}
			} else {
				if (isESL() || isAdHoc()) {
					sprintf(
							acall,
							"\nvoid (*%s_iterate) (%s, bufferMngr*, "
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* , "
							"\n\tvector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_iterate\"));"
							"\nif(!%s_iterate) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\n//printf(\"iterating1 %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
							//"\nprintf(\"before calling iterate\\n\"); fflush(stdout);"
							"\n(*(%s_iterate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, bm, inMemTables%s, slide_out, %s);"
							//"\nprintf(\"before calling iterate\\n\"); fflush(stdout);"
							, ffname, argTypes, argTypes, UID(a), ffname, ffname,
							getUserName(), UID(a), ffname, i, UID(a), ffname, i, ffname,
							outerAggrs, UID(a), ffname, i, argbuf, ptlist ? ptlistname
							: ", NULL", fetchModelId);
				} else {
					sprintf(acall, "\n%s_iterate(%sgbstatus_%d->%s_%d%s, _rec_id+1",
							ffname, outerAggrs, UID(a), ffname, i, argbuf);

					if (isESLAggr()) {
						strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					} else {
						strcat(acall, ", NULL, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					}
				}
			}
			strcat(buf, acall);

			if (defined_routines & AGGR_TERMINATE) {
				if (isESL() || isAdHoc()) {
					sprintf(
							acall,
							"\nvoid (*%s_terminate) (%s, int, bufferMngr*,"
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_terminate\"));"
							"\nif(!%s_terminate) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\nslide_out = 1;"
							"\n//printf(\"terminating %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
							"\n(*(%s_terminate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);"
							"\n}", ffname, argTypes, argTypes, UID(a), ffname, ffname,
							getUserName(), UID(a), ffname, i, UID(a), ffname, i, ffname,
							outerAggrs, UID(a), ffname, i, argbuf, ptlist ? ptlistname
							: ", NULL", fetchModelId);
				} else {
					sprintf(acall, "\n%s_terminate(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1",
							ffname, outerAggrs, UID(a), ffname, i, argbuf);

					if (isESLAggr()) {
						strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");\n}");
					} else {
						strcat(acall, ", NULL, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");\n}");
					}
				}
			} else {
				sprintf(acall, "\n}");
			}
			strcat(buf, acall);

			sprintf(
					acall,
					"\nelse if(gbstatus_%d->%s_%d->win->getTupleID() >= gbstatus_%d->%s_%d->last_out + %d - %d && !gbstatus_%d->%s_%d->iterate && gbstatus_%d->%s_%d->init) {"
					"\ngbstatus_%d->%s_%d->iterate = true;"
					"\ngbstatus_%d->%s_%d->init = false;"
					"\nslide_out = 0;", UID(a), ffname, i, UID(a), ffname, i,
					slide->size, winsize, UID(a), ffname, i, UID(a), ffname, i, UID(a),
					ffname, i, UID(a), ffname, i);
			strcat(buf, acall);

			sprintf(
					acall,
					"\nrwinbuf = gbstatus_%d->%s_%d->win;"
					"\nrlast_out = gbstatus_%d->%s_%d->last_out;"
					"\nfree(gbstatus_%d->%s_%d);"
					"\nfree(gbstatus_%d);"
					"\nhashgb_delete(%d, _rec_id); //deallocate -- should deallocate before, but that gets complicated"
					"\ngbstatus_%d = (struct gb_status_%d*)malloc(sizeof(*gbstatus_%d)); //allocate"
					"\ngbstatus_%d->%s_%d = (struct %s_status*)malloc(sizeof(struct %s_status));"
					"\ngbstatus_%d->%s_%d->win = 0;"
					"\ngbstatus_%d->%s_%d->last_out = 0;"
					"\ngbstatus_%d->%s_%d->iterate = true;"
					"\ngbstatus_%d->%s_%d->init = false;", UID(a), ffname, i, UID(a),
					ffname, i, UID(a), ffname, i, UID(a), UID(a), UID(a), UID(a), UID(a),
					UID(a), ffname, i, ffname, ffname, UID(a), ffname, i, UID(a), ffname,
					i, UID(a), ffname, i, UID(a), ffname, i);
			strcat(buf, acall);

			sprintf(acall, "\ngbstatus_%d->%s_%d->win = rwinbuf;"
					"\ngbstatus_%d->%s_%d->last_out = rlast_out;", UID(a), ffname, i,
					UID(a), ffname, i);
			strcat(buf, acall);

			//sprintf(acall, "\n//call initialize");
			if (isESL() || isAdHoc()) {
				sprintf(
						acall,
						"\nvoid (*%s_init) (%s, int, bufferMngr*,"
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
						"\n\tvector<A_timeexp>*, int, char*) = "
						"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_init\"));"
						"\nif(!%s_init) {"
						"\nchar errorMsg[1024];"
						"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
						"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
						"\nreturn s_failure;"
						"\n}"
						"\n//printf(\"initializing %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
						"\n(*(%s_init))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
						ffname, argTypes, argTypes, UID(a), ffname, ffname, getUserName(),
						UID(a), ffname, i, UID(a), ffname, i, ffname, outerAggrs, UID(a),
						ffname, i, argbuf, ptlist ? ptlistname : ", NULL", fetchModelId);
			} else {
				sprintf(acall, "\n%s_init(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1",
						ffname, outerAggrs, UID(a), ffname, i, argbuf);

				if (isESLAggr()) {
					strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
					strcat(acall, fetchModelId);
					strcat(acall, ");");
				} else {
					strcat(acall, ", NULL, inMemTables, NULL, slide_out, ");
					strcat(acall, fetchModelId);
					strcat(acall, ");");
				}
			}
			strcat(buf, acall);

			if (routine == AGGR_ITERATE) {
				sprintf(acall,
						"\nrc = hash_put(%d, _rec_id, gbkey, %d, &gbstatus_%d);", UID(a),
						keysize, UID(a));
				strcat(buf, acall);
			}

			sprintf(
					acall,
					"\n}"
					"\nelse if(gbstatus_%d->%s_%d->win->getTupleID() >= gbstatus_%d->%s_%d->last_out + %d - %d && gbstatus_%d->%s_%d->iterate) {"
					"\nslide_out = 0;", UID(a), ffname, i, UID(a), ffname, i,
					slide->size, winsize, UID(a), ffname, i);
			strcat(buf, acall);

			//sprintf(acall, "\n//call iterate");
			if (defined_routines & AGGR_INIT_ITERATE) {
				if (isESL() || isAdHoc()) {
					sprintf(
							acall,
							"\nvoid (*%s_init) (%s, int, bufferMngr*,"
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* , "
							"\n\tvector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_init\"));"
							"\nif(!%s_init) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\n//printf(\"iterating %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
							"\n(*(%s_init))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0, bm, inMemTables%s, slide_out, %s);",
							ffname, argTypes, argTypes, UID(a), ffname, ffname,
							getUserName(), UID(a), ffname, i, UID(a), ffname, i, ffname,
							outerAggrs, UID(a), ffname, i, argbuf, ptlist ? ptlistname
							: ", NULL", fetchModelId);
				} else {
					sprintf(acall, "\n%s_init(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0",
							ffname, outerAggrs, UID(a), ffname, i, argbuf);

					if (isESLAggr()) {
						strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					} else {
						strcat(acall, ", NULL, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					}
				}
			} else {
				if (isESL() || isAdHoc()) {
					sprintf(
							acall,
							"\nvoid (*%s_iterate) (%s, bufferMngr*, "
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* , "
							"\n\tvector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_iterate\"));"
							"\nif(!%s_iterate) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\n//printf(\"iterating %%d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
							//"\nprintf(\"before calling iterate\\n\"); fflush(stdout);"
							"\n(*(%s_iterate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, bm, inMemTables%s, slide_out, %s);"
							//"\nprintf(\"before calling iterate\\n\"); fflush(stdout);"
							, ffname, argTypes, argTypes, UID(a), ffname, ffname,
							getUserName(), UID(a), ffname, i, UID(a), ffname, i, ffname,
							outerAggrs, UID(a), ffname, i, argbuf, ptlist ? ptlistname
							: ", NULL", fetchModelId);
				} else {
					sprintf(acall, "\n%s_iterate(%sgbstatus_%d->%s_%d%s, _rec_id+1",
							ffname, outerAggrs, UID(a), ffname, i, argbuf);

					if (isESLAggr()) {
						strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					} else {
						strcat(acall, ", NULL, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					}
				}
			}
			strcat(buf, acall);
			strcat(buf, "\n}");
		} else {
			if (e->u.fun.implicit == 1) {
				if (win->range->type == COUNT_RANGE) { // count-based
					sprintf(acall, "\ngbstatus_%d->%s_%d->win->put(&windata);", UID(a),
							fname, i);
					strcat(buf, acall);
				} else if (ptlist) {
					sprintf(ptlistname, ", list%s_%d_%d", S_name(
								aggr_info->aggr->u.call.func), i, UID(a));
					sprintf(
							acall,
							"\ngbstatus_%d->%s_%d->win->put(&windata, gbkey, list%s_%d_%d->size());"
							"\nlist%s_%d_%d->push_back(A_Timeexp(gbstatus_%d->%s_%d->win));",
							UID(a), fname, i, S_name(aggr_info->aggr->u.call.func), i,
							UID(a), S_name(aggr_info->aggr->u.call.func), i, UID(a), UID(a),
							fname, i);
					strcat(buf, acall);
				} else if (win->range->type == TIME_RANGE) {
					sprintf(acall, "\ngbstatus_%d->%s_%d->win->put(&windata);", UID(a),
							fname, i);
					strcat(buf, acall);
				}
			}

			if (winsize != -1) {

				char heartbeat_check[400];
				if (isESL() && win->slide->type == HEARTBEAT_TIME_SLIDE) { 
					sprintf(heartbeat_check, "\nFLAGS_enable_heartbeat = true;"
							"\nif (FLAGS_enable_heartbeat && (int)gbstatus_%d->%s_%d->win->getTupleID() >= gbstatus_%d->%s_%d->last_out + %d) {"
							"\ngbstatus_%d->%s_%d->last_out = gbstatus_%d->%s_%d->last_out + %d;"
							"\nslide_out = 1;"
							"\n} else {"
							"\nslide_out = 0;"
							"\n}", UID(a), fname, i, UID(a), fname, i, slidesize, UID(a), fname, i, UID(a), fname, i, slidesize);	
				} else {
					sprintf(heartbeat_check, "slide_out = 0;");
				}


				sprintf(
						acall,
						"\nif((!(%d <= 1 || ((gbstatus_%d->%s_%d->win->getTupleID() == 0 && gbstatus_%d->%s_%d->last_out != 0) "
						"\n||((((int)gbstatus_%d->%s_%d->win->getTupleID()) >= gbstatus_%d->%s_%d->last_out + %d)))))) {"
						"\n%s"
						"\n//printf(\"Here no output %%d %d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
						"\n} else {"
						"\nslide_out = 1;"
						"\n//printf(\"Here YES output %%d %d %%d\\n\", gbstatus_%d->%s_%d->last_out, gbstatus_%d->%s_%d->win->getTupleID());fflush(stdout);"
						"\ngbstatus_%d->%s_%d->last_out = gbstatus_%d->%s_%d->last_out + %d;"
						"\nwhile(gbstatus_%d->%s_%d->last_out < (gbstatus_%d->%s_%d->win->getTupleID() - %d) && gbstatus_%d->%s_%d->win->getTupleID() > 0) {"
						"\nif(%d == 1) {"
						"\ngbstatus_%d->%s_%d->last_out = gbstatus_%d->%s_%d->win->getTupleID();"
						"\n}"
						"\nelse {"
						"\ngbstatus_%d->%s_%d->last_out = gbstatus_%d->%s_%d->last_out + %d;"
						"\n}"
						"\n}"
						"\n}", slidesize, UID(a), fname, i, UID(a), fname, i, UID(a),
						fname, i, UID(a), fname, i, slidesize, heartbeat_check, slidesize, UID(a), fname, i,
						UID(a), fname, i, slidesize, UID(a), fname, i, UID(a), fname, i,
						UID(a), fname, i, UID(a), fname, i, slidesize, UID(a), fname, i,
						UID(a), fname, i, slidesize, UID(a), fname, i, slidesize, UID(a),
						fname, i, UID(a), fname, i, UID(a), fname, i, UID(a), fname, i,
						slidesize);
				strcat(buf, acall);
			}

			// call uda routines
			switch (routine) {
				case AGGR_INIT:
					if ((isESL() || isAdHoc())) {
						sprintf(
								acall,
								"\nvoid (*%s_init) (%s, int, bufferMngr*,"
								"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
								"\n\tvector<A_timeexp>*, int, char*) = "
								"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_init\"));"
								"\nif(!%s_init) {"
								"\nchar errorMsg[1024];"
								"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
								"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
								"\nreturn s_failure;"
								"\n}"
								"\n(*(%s_init))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
								fname, argTypes, argTypes, UID(a), fname, fname, getUserName(),
								fname, outerAggrs, UID(a), fname, i, argbuf, ptlist ? ptlistname
								: ", NULL", fetchModelId);
						//in case of unlimited preceding calling terminate
						if (winsize == -1 && (defined_routines & AGGR_TERMINATE)) {
							strcat(buf, acall);
							sprintf(
									acall,
									"\nvoid (*%s_terminate) (%s, int, bufferMngr*,"
									"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
									"\n\tvector<A_timeexp>*, int, char*) = "
									"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_terminate\"));"
									"\nif(!%s_terminate) {"
									"\nchar errorMsg[1024];"
									"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
									"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
									"\nreturn s_failure;"
									"\n}"
									"\n(*(%s_terminate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
									fname, argTypes, argTypes, UID(a), fname, fname, getUserName(),
									fname, outerAggrs, UID(a), fname, i, argbuf,
									ptlist ? ptlistname : ", NULL", fetchModelId);
						}
					} else {
						sprintf(acall, "\n%s_init(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1",
								fname, outerAggrs, UID(a), fname, i, argbuf);
						if (isESLAggr()) {
							strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
							strcat(acall, fetchModelId);
							strcat(acall, ");");
						} else {
							strcat(acall, ", NULL, inMemTables, NULL, 0, ");
							strcat(acall, fetchModelId);
							strcat(acall, ");");
						}
						//in case of unlimited preceding calling terminate
						if (winsize == -1 && (defined_routines & AGGR_TERMINATE)) {
							strcat(buf, acall);
							sprintf(acall,
									"\n%s_terminate(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1", fname,
									outerAggrs, UID(a), fname, i, argbuf);
							if (isESLAggr()) {
								strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							} else {
								strcat(acall, ", NULL, inMemTables, NULL, 0, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							}
						}
					}
					break;
				case AGGR_ITERATE:
					//printf("in case iterate %d\n", (defined_routines & AGGR_EXPIRE));
					// call expire routine if defined
					//only if not unlimited preceding (signified by -1) and expire defined
					if (winsize != -1) {
						if ((isESL() || isAdHoc()) && defined_routines & AGGR_EXPIRE) {
							char temp[MAX_STR_LEN];
							if (win->range->type == TIME_RANGE && win->partition_list
									!= (A_list) 0 && !A_ListEmpty(win->partition_list)) {
								//here expire eagerly in TIME_RANGE with partition case
								//S_name(aggr_info->aggr->u.call.func), i, UID(a)
								acall[0] = '\0';
								sprintf(
										temp,
										"\nwinbuf* wbuf%s_%d_%d = "
										"\n  (index%s_%d_%d==list%s_%d_%d->size())?NULL:(*list%s_%d_%d)[index%s_%d_%d]->wbuf;"
										"\nwhile(wbuf%s_%d_%d && (*list%s_%d_%d)[index%s_%d_%d]->deleted==true) {"
										"\nindex%s_%d_%d++;"
										"\nwbuf%s_%d_%d = (index%s_%d_%d==list%s_%d_%d->size())?NULL:(*list%s_%d_%d)[index%s_%d_%d]->wbuf;"
										"\n}"
										"\nif(wbuf%s_%d_%d)"
										"\n  wbuf%s_%d_%d->updateTupleID(&atime);", S_name(
											aggr_info->aggr->u.call.func), i, UID(a), S_name(
												aggr_info->aggr->u.call.func), i, UID(a), S_name(
													aggr_info->aggr->u.call.func), i, UID(a), S_name(
														aggr_info->aggr->u.call.func), i, UID(a), S_name(
															aggr_info->aggr->u.call.func), i, UID(a), S_name(
																aggr_info->aggr->u.call.func), i, UID(a), S_name(
																	aggr_info->aggr->u.call.func), i, UID(a), S_name(
																		aggr_info->aggr->u.call.func), i, UID(a), S_name(
																			aggr_info->aggr->u.call.func), i, UID(a), S_name(
																				aggr_info->aggr->u.call.func), i, UID(a), S_name(
																					aggr_info->aggr->u.call.func), i, UID(a), S_name(
																						aggr_info->aggr->u.call.func), i, UID(a), S_name(
																							aggr_info->aggr->u.call.func), i, UID(a), S_name(
																								aggr_info->aggr->u.call.func), i, UID(a), S_name(
																									aggr_info->aggr->u.call.func), i, UID(a), S_name(
																										aggr_info->aggr->u.call.func), i, UID(a));
								strcat(acall, temp);
								sprintf(
										temp,
										"\nwhile(wbuf%s_%d_%d && wbuf%s_%d_%d->hasExpired()) {"
										"\nwbuf%s_%d_%d->getExpired(&windata);"
										"%s"
										"\nstruct gb_status_%d *tmpstatus = 0;"
										//need to write over gbkey, because it is not the right thing
										"\nmemcpy(_timeexpkey, ((char*)windata.data)+wbuf%s_%d_%d->keybegin, %d);"
										"\nrc = hash_get(%d, _rec_id, timeexpkey, %d, (char**)&tmpstatus);"
										"\nif(rc != 0) {"
										"\nchar errorMsg[1024];"
										"\nsprintf(errorMsg, \"Expire error aggregate: %s\\n\");"
										"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
										"\nreturn s_failure;"
										"\n}", S_name(aggr_info->aggr->u.call.func), i, UID(a),
										S_name(aggr_info->aggr->u.call.func), i, UID(a), S_name(
											aggr_info->aggr->u.call.func), i, UID(a),
										aggr_info->expire_code, UID(a), S_name(
											aggr_info->aggr->u.call.func), i, UID(a), keysize,
										UID(a), keysize, S_name(aggr_info->aggr->u.call.func),
										getUserName());
								strcat(acall, temp);

								//need to fetch the right gbstatus structure
								//the call the expire with that struct -- got the keysize now
								//close the while loop opened above
								sprintf(
										temp,
										"\nvoid (*%s_expire) (%s, bufferMngr*, "
										"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
										"\n\tvector<A_timeexp>*, int, char*) = "
										"((void(*)(%s, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_expire\"));"
										"\nif(!%s_expire) {"
										"\nchar errorMsg[1024];"
										"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
										"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
										"\nreturn s_failure;"
										"\n}"
										"\n(*(%s_expire))(%stmpstatus->%s_%d%s, _rec_id+1, bm, inMemTables%s, slide_out, %s);"
										"%s"
										"\nwbuf%s_%d_%d->pop();"
										"\n(*list%s_%d_%d)[index%s_%d_%d]->deleted = true;"
										"\nindex%s_%d_%d++;"
										"\nwbuf%s_%d_%d = "
										"\n  (index%s_%d_%d==list%s_%d_%d->size())?NULL:(*list%s_%d_%d)[index%s_%d_%d]->wbuf;"
										"\nwhile(wbuf%s_%d_%d && (*list%s_%d_%d)[index%s_%d_%d]->deleted==true) {"
										"\nindex%s_%d_%d++;"
										"\nwbuf%s_%d_%d = (index%s_%d_%d==list%s_%d_%d->size())?NULL:(*list%s_%d_%d)[index%s_%d_%d]->wbuf;"
										"\n}"
										"\nif(wbuf%s_%d_%d)"
										"\n  wbuf%s_%d_%d->updateTupleID(&atime);"
										"\n}", //End of while above
									fname, argTypes, argTypes, UID(a), fname, fname,
									getUserName(), fname, outerAggrs, fname, i,
									aggr_info->expire_argbuf, ptlist ? ptlistname : ", NULL",
									fetchModelId, aggr_info->extDeallocBuf_expire, S_name(
											aggr_info->aggr->u.call.func), i, UID(a), S_name(
												aggr_info->aggr->u.call.func), i, UID(a), S_name(
													aggr_info->aggr->u.call.func), i, UID(a), S_name(
														aggr_info->aggr->u.call.func), i, UID(a), S_name(
															aggr_info->aggr->u.call.func), i, UID(a), S_name(
																aggr_info->aggr->u.call.func), i, UID(a), S_name(
																	aggr_info->aggr->u.call.func), i, UID(a), S_name(
																		aggr_info->aggr->u.call.func), i, UID(a), S_name(
																			aggr_info->aggr->u.call.func), i, UID(a), S_name(
																				aggr_info->aggr->u.call.func), i, UID(a), S_name(
																					aggr_info->aggr->u.call.func), i, UID(a), S_name(
																						aggr_info->aggr->u.call.func), i, UID(a), S_name(
																							aggr_info->aggr->u.call.func), i, UID(a), S_name(
																								aggr_info->aggr->u.call.func), i, UID(a), S_name(
																									aggr_info->aggr->u.call.func), i, UID(a), S_name(
																										aggr_info->aggr->u.call.func), i, UID(a), S_name(
																											aggr_info->aggr->u.call.func), i, UID(a), S_name(
																												aggr_info->aggr->u.call.func), i, UID(a), S_name(
																													aggr_info->aggr->u.call.func), i, UID(a), S_name(
																														aggr_info->aggr->u.call.func), i, UID(a)
																													);
								strcat(acall, temp);
							} else {
								sprintf(
										acall,
										"\nwhile (gbstatus_%d->%s_%d->win->hasExpired()) {"
										"\ngbstatus_%d->%s_%d->win->getExpired(&windata);"
										"%s"
										"\nvoid (*%s_expire) (%s, bufferMngr*, "
										"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
										"\n\tvector<A_timeexp>*, int, char*) = "
										"((void(*)(%s, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_expire\"));"
										"\nif(!%s_expire) {"
										"\nchar errorMsg[1024];"
										"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
										"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
										"\nreturn s_failure;"
										"\n}"
										"\n(*(%s_expire))(%sgbstatus_%d->%s_%d%s, _rec_id+1, bm, inMemTables%s, slide_out, %s);"
										"%s"
										"\ngbstatus_%d->%s_%d->win->pop();"
										"\n}", UID(a), fname, i, UID(a), fname, i,
										aggr_info->expire_code, fname, argTypes, argTypes, UID(a),
										fname, fname, getUserName(), fname, outerAggrs, UID(a),
										fname, i, aggr_info->expire_argbuf, ptlist ? ptlistname
											: ", NULL", fetchModelId,
										aggr_info->extDeallocBuf_expire, UID(a), fname, i);
							}
						} else if (defined_routines & AGGR_EXPIRE) {
							char temp[500];
							sprintf(acall, "\nwhile (gbstatus_%d->%s_%d->win->hasExpired()){"
									"\ngbstatus_%d->%s_%d->win->getExpired(&windata);"
									"%s"
									"\n%s_expire(%sgbstatus_%d->%s_%d%s, _rec_id+1", UID(a), fname,
									i, UID(a), fname, i, aggr_info->expire_code, fname, outerAggrs,
									UID(a), fname, i, aggr_info->expire_argbuf);

							if (isESLAggr()) {
								strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							} else {
								strcat(acall, ", NULL, inMemTables, NULL, 0, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							}

							sprintf(temp, "\ngbstatus_%d->%s_%d->win->pop();"
									"\n}", UID(a), fname, i);
							strcat(acall, temp);
							sprintf(temp, aggr_info->extDeallocBuf_expire);
							strcat(acall, temp);
						} else {
							char temp[500];
							sprintf(acall, "\nwhile (gbstatus_%d->%s_%d->win->hasExpired()){"
									"\ngbstatus_%d->%s_%d->win->getExpired(&windata);"
									"\ngbstatus_%d->%s_%d->win->pop();"
									"\n}", UID(a), fname, i, UID(a), fname, i, UID(a), fname, i);
							sprintf(temp, aggr_info->extDeallocBuf_expire);
							strcat(acall, temp);
						}
						strcat(buf, acall);
					} // end call expire
					if (defined_routines & AGGR_INIT_ITERATE) {
						if ((isESL() || isAdHoc())) {

							sprintf(
									acall,
									"\nvoid (*%s_init) (%s, int, bufferMngr*,"
									"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
									"\n\tvector<A_timeexp>*, int, char*) = "
									"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_init\"));"
									"\nif(!%s_init) {"
									"\nchar errorMsg[1024];"
									"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
									"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
									"\nreturn s_failure;"
									"\n}"
									"\n(*(%s_init))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0, bm, inMemTables%s, slide_out, %s);",
									fname, argTypes, argTypes, UID(a), fname, fname, getUserName(),
									fname, outerAggrs, UID(a), fname, i, argbuf,
									ptlist ? ptlistname : ", NULL", fetchModelId);

							//in case of unlimited preceding calling terminate
							if (winsize == -1 && (defined_routines & AGGR_TERMINATE)) {
								strcat(buf, acall);
								sprintf(
										acall,
										"\nvoid (*%s_terminate) (%s, int, bufferMngr*, "
										"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
										"\n\tvector<A_timeexp>*, int, char*) = "
										"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_terminate\"));"
										"\nif(!%s_terminate) {"
										"\nchar errorMsg[1024];"
										"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
										"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
										"\nreturn s_failure;"
										"\n}"
										"\n(*(%s_terminate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
										fname, argTypes, argTypes, UID(a), fname, fname,
										getUserName(), fname, outerAggrs, UID(a), fname, i, argbuf,
										ptlist ? ptlistname : ", NULL", fetchModelId);
							}
							//strcat(acall, "\nprintf(\"calling dlc\\n\");fflush(stdout);\ndlclose(handle);\nprintf(\"backfrm dlc\\n\");fflush(stdout);");
						} else {
							sprintf(acall, "\n%s_init(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0",
									fname, outerAggrs, UID(a), fname, i, argbuf);

							if (isESLAggr()) {
								strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							} else {
								strcat(acall, ", NULL, inMemTables, NULL, 0, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							}

							//in case of unlimited preceding calling terminate
							if (winsize == -1 && (defined_routines & AGGR_TERMINATE)) {
								strcat(buf, acall);
								sprintf(acall,
										"\n%s_terminate(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1", fname,
										outerAggrs, UID(a), fname, i, argbuf);

								if (isESLAggr()) {
									strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
									strcat(acall, fetchModelId);
									strcat(acall, ");");
								} else {
									strcat(acall, ", NULL, inMemTables, NULL, 0, ");
									strcat(acall, fetchModelId);
									strcat(acall, ");");
								}
							}
						}
					} else {
						if ((isESL() || isAdHoc())) {
							sprintf(
									acall,
									"\nvoid (*%s_iterate) (%s, bufferMngr*, "
									"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
									"\n\tvector<A_timeexp>*, int, char*) = "
									"((void(*)(%s, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_iterate\"));"
									"\nif(!%s_iterate) {"
									"\nchar errorMsg[1024];"
									"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
									"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
									"\nreturn s_failure;"
									"\n}"
									//"\nprintf(\"before calling iterate\\n\"); fflush(stdout);"
									"\n(*(%s_iterate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, bm, inMemTables%s, slide_out, %s);"
									//"\nprintf(\"before calling iterate\\n\"); fflush(stdout);"
									, fname, argTypes, argTypes, UID(a), fname, fname,
									getUserName(), fname, outerAggrs, UID(a), fname, i, argbuf,
									ptlist ? ptlistname : ", NULL", fetchModelId);
							//in case of unlimited preceding calling terminate
							if (winsize == -1 && (defined_routines & AGGR_TERMINATE)) {
								strcat(buf, acall);
								sprintf(
										acall,
										"\nvoid (*%s_terminate) (%s, int, bufferMngr*, "
										"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
										"\n\tvector<A_timeexp>*, int, char*) = "
										"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_terminate\"));"
										"\nif(!%s_terminate) {"
										"\nchar errorMsg[1024];"
										"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
										"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
										"\nreturn s_failure;"
										"\n}"
										"\n(*(%s_terminate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
										fname, argTypes, argTypes, UID(a), fname, fname,
										getUserName(), fname, outerAggrs, UID(a), fname, i, argbuf,
										ptlist ? ptlistname : ", NULL", fetchModelId);
							}
						} else {
							sprintf(acall, "\n%s_iterate(%sgbstatus_%d->%s_%d%s, _rec_id+1",
									fname, outerAggrs, UID(a), fname, i, argbuf);

							if (isESLAggr()) {
								strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							} else {
								strcat(acall, ", NULL, inMemTables, NULL, 0, ");
								strcat(acall, fetchModelId);
								strcat(acall, ");");
							}
							//in case of unlimited preceding calling terminate
							if (winsize == -1 && (defined_routines & AGGR_TERMINATE)) {
								strcat(buf, acall);
								sprintf(acall,
										"\n%s_terminate(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, %s);",
										fname, outerAggrs, UID(a), fname, i, argbuf, fetchModelId);
							}
						}
					}
					break;
				case AGGR_TERMINATE:
					if ((isESL() || isAdHoc())) {
						sprintf(
								acall,
								"\nvoid (*%s_terminate) (%s, int, bufferMngr*, "
								"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
								"\n\tvector<A_timeexp>*, int, char*) = "
								"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_terminate\"));"
								"\nif(!%s_terminate) {"
								"\nchar errorMsg[1024];"
								"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
								"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
								"\nreturn s_failure;"
								"\n}"
								"\n(*(%s_terminate))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
								fname, argTypes, argTypes, UID(a), fname, fname, getUserName(),
								fname, outerAggrs, UID(a), fname, i, argbuf, ptlist ? ptlistname
								: ", NULL", fetchModelId);
					} else {
						sprintf(acall, "\n%s_terminate(%sgbstatus_%d->%s_%d%s, _rec_id+1",
								fname, outerAggrs, UID(a), fname, i, argbuf);

						if (isESLAggr()) {
							strcat(acall, ", 1, bm, inMemTables, NULL, slide_out, ");
							strcat(acall, fetchModelId);
							strcat(acall, ");");
						} else {
							strcat(acall, ", 1, NULL, inMemTables, NULL, 0, ");
							strcat(acall, fetchModelId);
							strcat(acall, ");");
						}
					}
					break;
			}
			strcat(buf, acall);

			/* Slide is always the same, if winsize is smaller then we may
			   save some computation, by not processing some tuples (that are
			   don't effect the computation, this remains to be done -- Hetal
			   if(winsize != -1 && winsize < slidesize) {
			   sprintf(acall, "\nif((gbstatus_%d->%s_%d->win->getTupleID() %% (%d)) != %d) {"
			   "\nslide_out = 0;"
			   "\n} else"
			   "\nslide_out = 1;",
			   UID(a), fname, i, slidesize, winsize);
			   }
			   else if(winsize != -1) {
			   sprintf(acall, "\nif(!(((gbstatus_%d->%s_%d->win->getTupleID() - 1) == 0) "
			   "\n||(((gbstatus_%d->%s_%d->win->getTupleID() - 1) %% %d) == 0))) {"
			   "\nslide_out = 0;"
			   "\n} else"
			   "\nslide_out = 1;",
			   UID(a), fname, i, UID(a), fname, i, slidesize);
			   }*/

			//sprintf(acall, "printf(\"tid %%d, winsize %d, slidesize %d, slide_out %%d\\n\", gbstatus_%d->%s_%d->win->getTupleID(), slide_out);",
			//	    winsize, slidesize, UID(a), fname, i);
			//strcat(buf, acall);
		}
	} else {
		// Traditional (non-aggr) aggregate

		// check if INIT/ITERATE are shared
		if (routine == AGGR_ITERATE && (defined_routines & AGGR_INIT_ITERATE)) {
			//routine = AGGR_INIT;
			if ((isESL() || isAdHoc())) {
				//this call is for iterate, therefore is_init flag is defaulted to 0
				sprintf(
						acall,
						"\nvoid (*%s_init) (%s, int, bufferMngr*, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* , "
						"\n\tvector<A_timeexp>*, int, char*) = "
						"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_init\"));"
						"\nif(!%s_init) {"
						"\nchar errorMsg[1024];"
						"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
						"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
						"\nreturn s_failure;"
						"\n}"
						"\n(*(%s_init))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0, bm, inMemTables%s, slide_out, %s);",
						fname, argTypes, argTypes, UID(a), fname, fname, getUserName(),
						fname, outerAggrs, UID(a), fname, i, argbuf, ptlist ? ptlistname
						: ", NULL", fetchModelId);
			} else {
				sprintf(acall, "\n%s_init(%sgbstatus_%d->%s_%d%s, _rec_id+1, 0", fname,
						outerAggrs, UID(a), fname, i, argbuf);

				if (isESLAggr()) {
					strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
					strcat(acall, fetchModelId);
					strcat(acall, ");");
				} else {
					strcat(acall, ", NULL, inMemTables, NULL, 0, ");
					strcat(acall, fetchModelId);
					strcat(acall, ");");
				}
			}
		} else {
			if ((isESL() || isAdHoc())) {
				if (routine == AGGR_INIT) {
					sprintf(
							acall,
							"\nvoid (*%s_%s) (%s, int, bufferMngr*, "
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
							"\n\tvector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, int, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_%s\"));"
							"\nif(!%s_%s) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\n(*(%s_%s))(%sgbstatus_%d->%s_%d%s, _rec_id+1, 1, bm, inMemTables%s, slide_out, %s);",
							fname, routine_name[routine], argTypes, argTypes, UID(a), fname,
							routine_name[routine], fname, routine_name[routine],
							getUserName(), fname, routine_name[routine], outerAggrs, UID(a),
							fname, i, argbuf, ptlist ? ptlistname : ", NULL", fetchModelId);
					//strcat(acall, "\nprintf(\"calling dlc\\n\");fflush(stdout);\ndlclose(handle);\nprintf(\"backfrm dlc\\n\");fflush(stdout);");
				} else {

					sprintf(
							acall,
							"\nvoid (*%s_%s) (%s, bufferMngr*, "
							"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>*, "
							"\n\tvector<A_timeexp>*, int, char*) = "
							"((void(*)(%s, bufferMngr*, hash_map<const char*, void*, hash<const char*>, eqstrTab>*, vector<A_timeexp>*, int, char*))dlsym(handle_%d, \"%s_%s\"));"
							"\nif(!%s_%s) {"
							"\nchar errorMsg[1024];"
							"\nsprintf(errorMsg, \"dl error at sym lookup: %%s\\n\", dlerror());"
							"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, errorMsg);"
							"\nreturn s_failure;"
							"\n}"
							"\n(*(%s_%s))(%sgbstatus_%d->%s_%d%s, _rec_id+1, bm, inMemTables%s, slide_out, %s);",
							fname, routine_name[routine], argTypes, argTypes, UID(a), fname,
							routine_name[routine], fname, routine_name[routine],
							getUserName(), fname, routine_name[routine], outerAggrs, UID(a),
							fname, i, argbuf, ptlist ? ptlistname : ", NULL", fetchModelId);
					//strcat(acall, "\nprintf(\"calling dlc\\n\");fflush(stdout);\ndlclose(handle);\nprintf(\"backfrm dlc\\n\");fflush(stdout);");
				}
			} else {
				sprintf(acall, "\n%s_%s(%sgbstatus_%d->%s_%d%s, _rec_id+1", fname,
						routine_name[routine], outerAggrs, UID(a), fname, i, argbuf);
				if (isESLAggr()) {
					if (routine == AGGR_INIT || routine == AGGR_TERMINATE) {
						//INIT case - need 1 for the __is_init
						//TERMINATE case - need 1 for not_delete
						strcat(acall, ", 1 , bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");// bit of cheating here where is the plist");
					} else {
						strcat(acall, ", bm, inMemTables, NULL, slide_out, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");//bit of cheating here where is the plist");
					}
				} else {
					if (routine == AGGR_INIT || routine == AGGR_TERMINATE) {
						strcat(acall, ", 1, NULL, inMemTables, NULL, 0, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					} else {
						strcat(acall, ", NULL, inMemTables, NULL, 0, ");
						strcat(acall, fetchModelId);
						strcat(acall, ");");
					}
				}
			}
		}
		strcat(buf, acall);
	}

	/* note we dealloc all iExts here, because we are assuming they are only
	   generated whiling calling the aggr, but if they could happen in a table
	   we have to remove this default deallocation */
	if (!win && (routine == AGGR_INIT || routine == AGGR_ITERATE)) {
		sprintf(acall, aggr_info->extDeallocBuf);
		strcat(buf, acall);
	}

	//for deallocation we need this but some how it is giving a seg fault
	/*
	   if((isESL() || isAdHoc())) {
	   sprintf(acall, "\nif(handle != NULL) {"
	   "\ndlclose(handle);"
	   "\n}");
	   strcat(buf, acall);
	   }*/
exit: DBUG_RETURN(rc);
      return rc;
}

err_t transCallAggr(S_table venv, S_table tenv, A_sqlopr a, /* gb node */
		Sql_sem sql, int routine, /* init, iterate, terminate */
		A_list aggr_list, /* list of UDAs */
		char *buf, /* OUT */
		vector<void*> aggregates, int keysize) {
	DBUG_ENTER("transCallAggr");
	err_t rc = ERR_NONE;
	E_enventry x;

	*buf = '\0';

	//printf("entering the loop\n");
	for (int i = 0; i < aggr_list->length; i++) {
		Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
		//printf("aggr info1 %d\n", aggr_info->builtin);

		if (aggr_info->builtin) {
			//printf("transBuiltin case\n");
			rc = transCallBuiltInAggr(a, aggr_info, i, routine, buf);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCallAggr",
						"transCallBuiltInAggr");
				goto exit;
			}
		} else {
			// check if the aggr is defined
			//printf("non transBuiltin case\n");
			x = (E_enventry) S_look(venv, aggr_info->aggr->u.call.func);
			if (!x) {
				rc = ERR_UNDEFINED_FUNCTION;
				EM_error(aggr_info->aggr->pos, rc, __LINE__, __FILE__, S_name(
							aggr_info->aggr->u.call.func));
				goto exit;
			}

			// check if the aggr has the routine (INIT/ITERATE/TERMINATE) defined
			if ((routine & x->u.fun.aggr_routines) != 0) {
				rc = transCallUDA(a, sql, aggr_info, i, routine,
						x->u.fun.aggr_routines, buf, aggregates, venv, keysize);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCallAggr",
							"transCallUDA");
					goto exit;
				}
			}
		}

	}

exit: DBUG_RETURN(rc);
      return rc;
}

/*
 * transGenGBKey() combines all the group-by columns into a single
 * string, which is to be used as the key of the groupby table. 
 */
err_t transGenGBKey(S_table venv, S_table tenv, A_sqlopr a, /* gb node */
		A_list gb_list, Sql_sem sql, char *name, /* target */
		char *encodebuf, /* OUT: C code */
		char *decodebuf, /* OUT: C code */
		int &keysize, vector<void*> aggregates) {
	err_t rc = ERR_NONE;
	int i, j, found_hxp;
	int offset[16];
	char line[MAX_STR_LEN], field[80];
	A_exp gb;
	A_selectitem hxp;
	T_expty dummy, grps[16];

	keysize = 0;
	*encodebuf = *decodebuf = '\0';

	/*
	   if (!a->prd_list || a->prd_list->length==0) {
	   strcpy(encodebuf,
	   "\nstrcpy(gbkey, \"____\");"
	   );
	   keysize = 4;
	   goto exit;
	   }
	 */

	/* process group-by column one by one */
	for (i = 0; i < gb_list->length; i++) {
		gb = (A_exp) getNthElementList(gb_list, i);

		/* compile group-by column */
		rc = transExp(venv, tenv, gb, sql, dummy, grps[i], aggregates);
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGenGBKey", "transExp");
			goto exit;
		}

		offset[i] = keysize;

		/* Look for head expr that represents the group-by column */
		found_hxp = 0;
		for (j = 0; j < a->hxp_list->length && found_hxp == 0; j++) {
			hxp = (A_selectitem) getNthElementList(a->hxp_list, j);

			if (hxp->kind == SIMPLE_ITEM && hxp->u.s.exp->kind != A_callExp
					&& equalExp(hxp->u.s.exp, gb)) {
				found_hxp = 1;
				if (hxp->u.s.alias == (S_symbol) 0) {
					sprintf(field, "field_%d", j);
				} else {
					strcpy(field, S_name(hxp->u.s.alias));
				}
			}
		}

		switch (grps[i]->ty->kind) {
			case Ty_int:
			case Ty_ref:
				/* encode */
				sprintf(line, "\nmemcpy(gbkey+%d, &(%s), sizeof(int));", offset[i],
						grps[i]->exp);
				strcat(encodebuf, line);
				/* decode */
				if (found_hxp) {
					sprintf(line, "\nmemcpy(&%s.%s, allkey+%d, sizeof(int));", name, field,
							offset[i]);
					strcat(decodebuf, line);
				}
				keysize += 4;
				break;
			case Ty_string:
				/* encode */
				//      sprintf(line, "\nmemcpy(gbkey+%d, %s, 20);",
				sprintf(line, "\nmemcpy(gbkey+%d, %s, %d);", offset[i], grps[i]->exp,
						grps[i]->size);
				strcat(encodebuf, line);
				/* decode */
				if (found_hxp) {
					//	sprintf(line, "\nmemcpy(%s.%s, allkey+%d, 20);",
					sprintf(line, "\nmemcpy(%s.%s, allkey+%d, %d);", name, field,
							offset[i], grps[i]->size);
					strcat(decodebuf, line);
				}
				keysize += grps[i]->size;
				break;
			case Ty_timestamp:
				/* encode */
				//      sprintf(line, "\nmemcpy(gbkey+%d, %s, 20);",
				sprintf(line, "\nmemcpy(gbkey+%d, &%s, %d);", offset[i], grps[i]->exp,
						sizeof(struct timeval));
				strcat(encodebuf, line);
				/* decode */
				if (found_hxp) {
					//	sprintf(line, "\nmemcpy(%s.%s, allkey+%d, 20);",
					sprintf(line, "\nmemcpy(&%s.%s, allkey+%d, %d);", name, field,
							offset[i], sizeof(struct timeval));
					strcat(decodebuf, line);
				}
				keysize += sizeof(struct timeval);
				break;
			default:
				rc = ERR_DATATYPE;
				EM_error(0, rc, __LINE__, __FILE__, "transGenGBKey()");
		}
	}

exit: return rc;
}

/*
 * Function/Aggregate returns a tuple type. 
 *
 * an invocation f()->name returns a member field in the rescord
 *
 * getCallExpRetField() decides if "callexp" references a field that
 * exists in the tuple type, and if it does, return the field and its 
 * offset in the tuple type.
 */
err_t getCallExpRetField(S_table venv, S_table tenv, A_exp callexp, // calling function, which references a field
		Ty_field &f, // OUT: type of the field
		int *off // OUT: offset of the field in the record
		) {
	err_t rc = ERR_NONE;

	E_enventry x = (E_enventry) S_look(venv, callexp->u.call.func);
	A_list ret_record = x->u.fun.result->u.record;
	int offset[2] = { 0, 0 };
	int i;

	if (callexp->u.call.member == (S_symbol) 0) {
		if (ret_record->length > 1) {
			rc = ERR_COMPLEX_DATA;
			EM_error(callexp->pos, rc, __LINE__, __FILE__, S_name(
						callexp->u.call.func));
			goto exit;
		}
		f = (Ty_field) getNthElementList(ret_record, 0);
		*off = 0;
	} else {
		for (i = 0; i < ret_record->length; i++) {
			f = (Ty_field) getNthElementList(ret_record, i);
			if (f->name == callexp->u.call.member) {
				if (f->iskey)
					*off = offset[1];
				*off = offset[0];
				break;
			}

			if (f->ty == Ty_String()) {
				if (f->iskey)
					offset[1] += f->size;
				offset[0] += f->size;
			} else {
				if (f->iskey)
					offset[1] += getStorageSize(f->ty);
				offset[0] += getStorageSize(f->ty);
			}
			//        if (f->ty == Ty_Int()) offset[f->iskey] += sizeof(int);
			//        if (f->ty == Ty_Real()) offset[f->iskey] += sizeof(double);
		}
		/* not found */
		if (i >= ret_record->length) {
			rc = ERR_UNDEFINED_FIELD;
			EM_error(callexp->pos, rc, __LINE__, __FILE__, S_name(
						callexp->u.call.member));
			goto exit;
		}
	}

exit: return rc;
}

// remove time-based window
err_t transWindowRemove(A_sqlopr a, A_list aggr_list, T_expty &exe) {
	DBUG_ENTER("transWindowRemove");
	err_t rc = ERR_NONE;
	char buf[MAX_STR_LEN];

	for (int i = 0; i < aggr_list->length; i++) {
		Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
		A_win win = aggr_info->aggr->u.call.win;

		// We only create time-based window.
		// For row-based windows, we create them in gbstatus structures
		// i.e. We have only one time-based window, but multiple row-based windows for each GB values
		// remember we don't create a window if unlimited (signified by size = -1)
		if (win != (A_win) 0 && win->range->type == TIME_RANGE && win->range->size
				!= -1) {
			sprintf(buf, "\ndelete(_adl_win_%s_%d_%d);", S_name(
						aggr_info->aggr->u.call.func), i, UID(a) // differentiate same aggr on different windows
			       );
			exe = expTy_Seq(exe, buf);
			//       sprintf(buf,
			// 	      "\nchar _adl_winkey[%d];"
			// 	      "\nchar _adl_windata[%d];",
			// 	      aggr_info->winkey_size, aggr_info->winkey_size);
			//       exe = expTy_Seq(exe, buf);
		}
	}

exit: DBUG_RETURN(rc);
      return rc;
}

err_t transWindow(A_sqlopr a, A_list aggr_list, T_expty &exe) {
	err_t rc = ERR_NONE;
	char buf[MAX_STR_LEN];

	for (int i = 0; i < aggr_list->length; i++) {
		Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
		A_win win = aggr_info->aggr->u.call.win;

		// We only create time-based window.
		// For row-based windows, we create them in gbstatus structures
		// i.e. We have only one time-based window, but multiple row-based windows for each GB values
		// not creating this if unlimited (signified by size = -1)
		if (win != (A_win) 0 && win->range->type == TIME_RANGE && win->range->size
				!= -1) {
			sprintf(buf,
					"\nwinbuf *_adl_win_%s_%d_%d = new winbuf(%d, %d, _ADL_WIN_TIME);",
					S_name(aggr_info->aggr->u.call.func), i, UID(a), // differentiate same aggr on different windows
					win->range->size, aggr_info->windata_size
					//(win->range->type == COUNT_RANGE) ? "_ADL_WIN_ROW": "_ADL_WIN_TIME",
					//aggr_info->winkey_size,
					//aggr_info->windata_size
			       );
			exe = expTy_Seq(exe, buf);
			//       sprintf(buf,
			// 	      "\nchar _adl_winkey[%d];"
			// 	      "\nchar _adl_windata[%d];",
			// 	      aggr_info->winkey_size, aggr_info->winkey_size);
			//       exe = expTy_Seq(exe, buf);
		}
	}

exit: return rc;
}

err_t transTimeExpList(A_sqlopr a, A_list aggr_list, T_expty &exe) {
	err_t rc = ERR_NONE;
	char buf[MAX_STR_LEN];

	for (int i = 0; i < aggr_list->length; i++) {
		Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
		A_win win = aggr_info->aggr->u.call.win;

		// We only create time-based window.
		// For row-based windows, we create them in gbstatus structures
		// i.e. We have only one time-based window, but multiple row-based windows for each GB values
		// not creating this if unlimited (signified by size = -1)
		if (win != (A_win) 0 && win->range->type == TIME_RANGE && win->range->size
				!= -1 && win->partition_list != (A_list) 0 && !A_ListEmpty(
					win->partition_list)) {
			sprintf(buf,
					"\nstatic vector<A_timeexp> *list%s_%d_%d = new vector<A_timeexp>;"
					"\nstatic int index%s_%d_%d=0;", S_name(
						aggr_info->aggr->u.call.func), i, UID(a), // differentiate same aggr on different windows
					S_name(aggr_info->aggr->u.call.func), i, UID(a)
			       );
			exe = expTy_Seq(exe, buf);
		}
	}

exit: return rc;
}

err_t transTimeExpListRemove(A_sqlopr a, A_list aggr_list, T_expty &exe) {
	err_t rc = ERR_NONE;
	char buf[MAX_STR_LEN];

	for (int i = 0; i < aggr_list->length; i++) {
		Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
		A_win win = aggr_info->aggr->u.call.win;

		// We only create time-based window.
		// For row-based windows, we create them in gbstatus structures
		// i.e. We have only one time-based window, but multiple row-based windows for each GB values
		// not creating this if unlimited (signified by size = -1)
		if (win != (A_win) 0 && win->range->type == TIME_RANGE && win->range->size
				!= -1 && win->partition_list != (A_list) 0 && !A_ListEmpty(
					win->partition_list)) {
			sprintf(buf, "\nlist%s_%d_%d->clear();"
					"\nindex%s_%d_%d=0;", S_name(aggr_info->aggr->u.call.func), i, UID(a), // differentiate same aggr on different windows
					S_name(aggr_info->aggr->u.call.func), i, UID(a)
			       );
			exe = expTy_Seq(exe, buf);
		}
	}

exit: return rc;
}

err_t getBuiltInFinalResult(Aggr_info_t aggr_info, int i, A_sqlopr a,
		A_list t_fields, int count_only_aggr_p, char *name, T_expty &exe) {
	err_t rc = ERR_NONE;
	char buf[MAX_STR_LEN];
	int j;
	Ty_field curfield;
	A_exp aggr = aggr_info->aggr;

	if (!isESL() && aggr->u.call.win == (A_win) 0) {
		sprintf(buf, "\nrc = DB_NOTFOUND;"
				"\nif (terminating_%d == 1) {", UID(a));
	} else {
		sprintf(buf, "\nrc = DB_NOTFOUND;");
	}
	exe = expTy_Seq(exe, buf);

	if (count_only_aggr_p) {
		sprintf(buf, "\nif (gbstatus_%d == (struct gb_status_%d *)0) {"
				"\nif (first_entry_%d) {"
				"\nrc = 0;", UID(a), UID(a), UID(a));
		exe = expTy_Seq(exe, buf);

		/* We return results of aggregation on empty sets */
		for (j = 0; j < a->hxp_list->length; j++) {
			A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, j);

			if (arg->kind == SIMPLE_ITEM && aggr == arg->u.s.exp) {

				//	E_enventry x = (E_enventry)S_look(venv, aggr->u.call.func);
				char field[80], *fieldp;

				if (arg->u.s.alias == (S_symbol) 0) {
					sprintf(field, "field_%d", i);
					fieldp = field;
				} else {
					fieldp = S_name(arg->u.s.alias);
				}
				if (aggr_info->builtin == AGGR_BUILTIN_COUNT || aggr_info->builtin
						== AGGR_BUILTIN_SUM || aggr_info->builtin == AGGR_BUILTIN_COUNTR
						|| aggr_info->builtin == AGGR_BUILTIN_SUMR || aggr_info->builtin
						== AGGR_BUILTIN_VAR) {
					sprintf(buf, "\n%s.%s = 0;", name, fieldp);
				} else {
					sprintf(buf, "\n%s.%s = _ADL_NULL;", name, fieldp);
				}

				exe = expTy_Seq(exe, buf);
				break;
			}
		}
		exe = expTy_Seq(exe, "\n}\n} else ");
	}

	/* We return results of aggregation */
	sprintf(buf, "\nif (gbstatus_%d->_baggr_%d_first_entry == 1) {", UID(a), i);
	exe = expTy_Seq(exe, buf);

	for (j = 0; j < a->hxp_list->length; j++) {
		A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, j);

		if (arg->kind == SIMPLE_ITEM && aggr == arg->u.s.exp) {
			if (arg->u.s.alias == (S_symbol) 0) {
				char field[20];
				sprintf(field, "field_%d", i);

				switch (aggr_info->builtin_type) {
					case Ty_int:
						curfield = Ty_Field(new_Symbol(field), Ty_Int(), sizeof(int));
						break;
					case Ty_real:
						curfield = Ty_Field(new_Symbol(field), Ty_Real(), sizeof(double));
						break;
					case Ty_string:
						curfield = Ty_Field(new_Symbol(field), Ty_String(), MAX_STR_LEN);
						break;
					case Ty_timestamp:
						curfield = Ty_Field(new_Symbol(field), Ty_Timestamp(),
								sizeof(struct timeval));
						break;
					case Ty_iext:
						curfield = Ty_Field(new_Symbol(field), Ty_IExt(),
								sizeof(struct iExt_));
						break;
					case Ty_rext:
						curfield = Ty_Field(new_Symbol(field), Ty_RExt(),
								sizeof(struct rExt_));
						break;
					case Ty_cext:
						curfield = Ty_Field(new_Symbol(field), Ty_CExt(),
								sizeof(struct cExt_));
						break;
					case Ty_text:
						curfield = Ty_Field(new_Symbol(field), Ty_TExt(),
								sizeof(struct tExt_));
						break;
				} // end switch
				if (aggr_info->builtin_type == Ty_string)
					sprintf(buf, "\nstrcpy(%s.%s, gbstatus_%d->_baggr_%d_value);", name,
							field, UID(a), i);
				else if (aggr_info->builtin_type == Ty_timestamp) {
					sprintf(buf, "\nmemcpy(&(%s.%s), gbstatus_%d->_baggr_%d_value, %d);",
							name, field, UID(a), i, sizeof(struct timeval));
				} else if (aggr_info->builtin_type == Ty_iext) {
					sprintf(buf, "\nmemcpy(&(%s.%s), gbstatus_%d->_baggr_%d_value, %d);",
							name, field, UID(a), i, sizeof(struct iExt_));
				} else if (aggr_info->builtin_type == Ty_rext) {
					sprintf(buf, "\nmemcpy(&(%s.%s), gbstatus_%d->_baggr_%d_value, %d);",
							name, field, UID(a), i, sizeof(struct rExt_));
				} else if (aggr_info->builtin_type == Ty_cext) {
					sprintf(buf, "\nmemcpy(&(%s.%s), gbstatus_%d->_baggr_%d_value, %d);",
							name, field, UID(a), i, sizeof(struct cExt_));
				} else if (aggr_info->builtin_type == Ty_text) {
					sprintf(buf, "\nmemcpy(&(%s.%s), gbstatus_%d->_baggr_%d_value, %d);",
							name, field, UID(a), i, sizeof(struct tExt_));
				} else if (aggr_info->builtin == AGGR_BUILTIN_VAR) // variance
				{
					sprintf(
							buf,
							"\nif (gbstatus_%d->_baggr_%d_value_cnt == 0) {"
							"\n\t%s.%s = 0;"
							"\n} else {"
							"\n\t%s.%s = sqrt(gbstatus_%d->_baggr_%d_value / gbstatus_%d->_baggr_%d_value_cnt);"
							"\n}", UID(a), i, name, field, name, field, UID(a), i, UID(a),
							i);
				} else
					// int or real
					sprintf(buf, "\n%s.%s = gbstatus_%d->_baggr_%d_value;", name, field,
							UID(a), i);

			} else {
				switch (aggr_info->builtin_type) {
					case Ty_int:
						curfield = Ty_Field(arg->u.s.alias, Ty_Int(), sizeof(int));
						break;
					case Ty_real:
						curfield = Ty_Field(arg->u.s.alias, Ty_Real(), sizeof(double));
						break;
					case Ty_timestamp:
						curfield = Ty_Field(arg->u.s.alias, Ty_Timestamp(),
								sizeof(struct timeval));
						break;
					case Ty_iext:
						curfield = Ty_Field(arg->u.s.alias, Ty_IExt(), sizeof(struct iExt_));
						break;
					case Ty_rext:
						curfield = Ty_Field(arg->u.s.alias, Ty_RExt(), sizeof(struct rExt_));
						break;
					case Ty_cext:
						curfield = Ty_Field(arg->u.s.alias, Ty_CExt(), sizeof(struct cExt_));
						break;
					case Ty_text:
						curfield = Ty_Field(arg->u.s.alias, Ty_TExt(), sizeof(struct tExt_));
						break;
					case Ty_string:
						curfield = Ty_Field(arg->u.s.alias, Ty_String(), MAX_STR_LEN);
						break;
				} // end switch
				if (aggr_info->builtin_type == Ty_string)
					sprintf(buf, "\nstrcpy(%s.%s,gbstatus_%d->_baggr_%d_value);", name,
							S_name(arg->u.s.alias), UID(a), i);
				else if (aggr_info->builtin == AGGR_BUILTIN_VAR) // variance
				{
					sprintf(
							buf,
							"\nif (gbstatus_%d->_baggr_%d_value_cnt == 0) {"
							"\n\t%s.%s = 0;"
							"\n} else {"
							"\n\t%s.%s = sqrt(gbstatus_%d->_baggr_%d_value / gbstatus_%d->_baggr_%d_value_cnt);"
							"\n}", UID(a), i, name, S_name(arg->u.s.alias), name, S_name(
								arg->u.s.alias), UID(a), i, UID(a), i);
				} else
					sprintf(buf, "\n%s.%s = gbstatus_%d->_baggr_%d_value;", name, S_name(
								arg->u.s.alias), UID(a), i);
			}
			exe = expTy_Seq(exe, buf);

			appendElementList(t_fields, (nt_obj_t*) curfield);
			break;
		}
	} // end of for

	if (!isESL() && aggr->u.call.win == (A_win) 0) {
		sprintf(buf, "\ngbstatus_%d->_baggr_%d_first_entry = 0;"
				"\nrc = 0;"
				"\n} else {"
				"\ngbstatus_%d->_baggr_%d_first_entry = 1;"
				"\n}\n}"
				//	    "\n}\nbreak;"
				, UID(a), i, UID(a), i);
	} else {
		sprintf(buf, "\ngbstatus_%d->_baggr_%d_first_entry = 0;"
				"\nrc = 0;"
				"\n} else {"
				"\ngbstatus_%d->_baggr_%d_first_entry = 1;"
				"\n}"
				//	    "\n}\nbreak;"
				, UID(a), i, UID(a), i);
	}
	exe = expTy_Seq(exe, buf);

exit: return rc;
}

err_t getUDAFinalResult(S_table venv, S_table tenv, Aggr_info_t aggr_info,
		int i, A_sqlopr a, A_list t_fields, char *name, T_expty &exe) {
	err_t rc = ERR_NONE;
	char rettable[80], buf[MAX_STR_LEN], fname[80];
	char first_entry_name[80];
	char cursor_name[80];
	int j;
	Ty_field curfield;
	A_exp aggr = aggr_info->aggr;

	strcpy(fname, S_name(aggr->u.call.func));
	A_win win = aggr->u.call.win;
	int winsize;
	A_slide slide;
	int slidesize = 1;

	if (win != (A_win) 0) {
		winsize = win->range->size;
		slide = win->slide;
		if (slide)
			slidesize = slide->size;
	}

	char ffname[80];
	sprintf(ffname, fname);
	ffname[strlen(ffname) - strlen("_window")] = '\0';

	//lookup base version
	E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

	//BaseOpt: check slide > window size here as well
	if (win && slide && slide->size >= winsize && winsize != -1 && base
			&& base->kind == E_aggrEntry) {
		sprintf(first_entry_name, "gbstatus_%d->%s_%d->retc_first_entry", UID(a),
				ffname, i);
		sprintf(cursor_name, "gbstatus_%d->%s_%d->retc", UID(a), ffname, i);
	} else {
		/* We return results in the "return" Table. */
		sprintf(first_entry_name, "gbstatus_%d->%s_%d->retc_first_entry", UID(a),
				fname, i);
		sprintf(cursor_name, "gbstatus_%d->%s_%d->retc", UID(a), fname, i);
	}
	/*
	 * Since we are dealing explicitly with return table
	 * we can pass in the "TAB_MEMORY" flag here
	 */
	rc = transCursorGet2C(venv, first_entry_name, cursor_name, (E_enventry) 0,
			(Tr_exp*) 0, // binding
			(Tr_exp*) 0, // binding upper bound
			(Tr_exp*) 0, // binding lower bound
			BOUND_NONE, // no binding
			buf);

	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transCursorGet2C");
		goto exit;
	}

	exe = expTy_Seq(exe, buf);

	if (!isESL())
		sprintf(buf, "\nif (rc == 0) {"
				"\n%s = 0;", first_entry_name);
	else
		sprintf(buf, "\nif (rc == 0) {"
				"\n//output_count++;"
				"\n%s = 0;", first_entry_name);

	exe = expTy_Seq(exe, buf);

	// make assignments of aggr head expressions
	for (j = 0; j < a->hxp_list->length; j++) {
		A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, j);
		//    E_enventry x = (E_enventry)S_look(venv, aggr->u.call.func);

		/* make assignments for both original and shared aggregate */
		if (arg->kind == SIMPLE_ITEM && equalExp(aggr, arg->u.s.exp)) {

			Ty_field tyf;
			int offset;
			int current_offset;
			rc = getCallExpRetField(venv, tenv, arg->u.s.exp, tyf, &offset);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
						"getCallExpRetField");
				goto exit;
			}

			current_offset = offset;

			if (arg->u.s.alias == (S_symbol) 0) {
				char field[20];

				sprintf(field, "field_%d", i);
				rc = assignField2C(tyf, name, field, &offset, &offset, buf);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
							"assignField2C");
					goto exit;
				}
				curfield
					= Ty_Field(new_Symbol(field), tyf->ty, offset - current_offset);
			} else {
				rc = assignField2C(tyf, name, S_name(arg->u.s.alias), &offset, &offset,
						buf);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
							"assignField2C");
					goto exit;
				}
				curfield = Ty_Field(arg->u.s.alias, tyf->ty, offset - current_offset);
			}

			exe = expTy_Seq(exe, buf);

			appendElementList(t_fields, (nt_obj_t*) curfield);
		}
	}

	/* delete current tuple in the "return" table */
	rc = transCursorDelete2C(cursor_name, buf);

	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transCursorDelete2C");
		goto exit;
	}
	exe = expTy_Seq(exe, buf);

	// reset cursor
	sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
			"\n%s = 1;", first_entry_name, UID(a));
	exe = expTy_Seq(exe, buf);
	if (isESL()) {
		sprintf(
				buf,
				"\n} else {"
				"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DBC->c_get()\");"
				"\nreturn s_failure;"
				"\n}", getUserName(), getQueryName());
	} else if (isESLAggr()) {
		sprintf(
				buf,
				"\n} else {"
				"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DBC->c_get()\");"
				"\nreturn;"
				"\n}", getUserName(), getAggrName());
	} else {
		sprintf(buf, "\n} else adlabort(rc, \"DBC->c_get()\");");
	}
	exe = expTy_Seq(exe, buf);

exit: return rc;
}

err_t transGBOpr(S_table venv, S_table tenv, A_sqlopr a, Sql_sem sql,
		T_expty &dec, T_expty &exe, char *name, vector<void*> aggregates, vector<
		string> &srcs) {
	DBUG_ENTER("transGBOpr");
	err_t rc = ERR_NONE;
	int i, j;
	A_list t_fields;
	A_list aggr_list = A_List();
	int has_aggr = 0;
	int count_only_aggr_p = 1;

	T_expty subdec, subexe;
	char buf[MAX_STR_LEN], decodebuf[MAX_STR_LEN];
	char linebuf[MAX_STR_LEN];
	int keysize;
	char fname[80], qunname[80];
	Ty_field curfield;
	A_qun qun;
	A_list final_prd_list = NULL;
	bool sgtw = false;
	int aggrCnt = 0;
	A_exp aggrExp;
	int paneBasedOpt = 0;
	cStmt* cstmt = new cStmt("temp");

	dec = exe = (T_expty) 0;

	S_beginScope(venv);
	S_beginScope(tenv);

	sql->cur_sqlopr = a;

	/* collect all aggrs */

	//during this loop determine if any aexp->u.call.win->patition_list is specified
	//if so record it, if more than one specified then make sure they are consistent
	//otherwise throw error
	//After the loop check to see if a->prd_list is non null, if "a" has a group by and
	//partition by used, throw error
	//if only partition by is used and it is consistent if multiple of them, then set
	// a->prd_list equal to partition by
	//if only prd_list specified then no change
	// CONCERN: from the adl_yacc it seems that hxp_list is always empty, need to check
	//          because Rewrite may take care of this
	// here is where the list is made YYYY

	//we also count the number of aggregate expressions
	//if the number is 1 and we are using window, then we save the aggregate
	//expression. Then we can look at teh expression to determine if it
	//can be optimized with pane based optimization.

	for (i = 0; i < a->hxp_list->length; i++) {
		A_selectitem hxp = (A_selectitem) getNthElementList(a->hxp_list, i);
		A_exp aexp;
		E_enventry x;

		if (hxp->kind != SIMPLE_ITEM) {
			continue;
		}

		aexp = hxp->u.s.exp;

		if (aexp->kind == A_callExp) {
			aggrCnt++;
			aggrExp = aexp;
			// check if the aggr is defined
			x = (E_enventry) S_look(venv, aexp->u.call.func);
			if (!x) {
				rc = ERR_UNDEFINED_FUNCTION;
				EM_error(aexp->pos, rc, __LINE__, __FILE__, S_name(aexp->u.call.func));
				goto exit;
			}

			if (x->kind == E_aggrEntry) {
				has_aggr = 1;
				if (x->u.fun.builtin != AGGR_BUILTIN_COUNT && x->u.fun.builtin
						!= AGGR_BUILTIN_SUM && x->u.fun.builtin != AGGR_BUILTIN_COUNTR
						&& x->u.fun.builtin != AGGR_BUILTIN_SUMR && x->u.fun.builtin
						!= AGGR_BUILTIN_SUMR)
					count_only_aggr_p = 0;

				if (aexp->u.call.shared != SHARED_INVOCATION) {
					Aggr_info_t aggr_info = AggrInfo(aexp);
					//printf("aggr info %d\n", aggr_info->builtin);
					appendElementList(aggr_list, (nt_obj_t*) aggr_info);
				}
				if (x->u.fun.window && // UDA that requires window
						!aexp->u.call.win) // no window provided
				{
					rc = ERR_WINDOW_UDA_REQUIRED;
					EM_error(aexp->pos, rc, __LINE__, __FILE__, S_name(aexp->u.call.func));
					goto exit;
				}

				if (aexp->u.call.win && aexp->u.call.win->partition_list) {
					if (final_prd_list == NULL || final_prd_list
							== aexp->u.call.win->partition_list) {
						//printf("assigning prd list %s\n", S_name(aexp->u.call.func));
						final_prd_list = aexp->u.call.win->partition_list;
					} else {
						//here verify that they are the same
						//for now we can disallow this, i.e. throw error
						//printf("found another prd list %s\n", S_name(aexp->u.call.func));
						rc = ERR_INVALID_PARTITION_USE;
						EM_error(
								aexp->pos,
								rc,
								__LINE__,
								__FILE__,
								"You are using partition by in two different aggregates, which is currently not supported, even if they are the same.");
						goto exit;
					}
				}

				// 	if (x->u.fun.builtin != 0) {
				// 	  /* builtin aggregate */
				// 	  builtin_type[builtin_list->length] = Ty_int; // default type of built in aggregates;
				// 	  appendElementList(builtin_list, (nt_obj_t*)aexp);
				// 	} else {
				// 	  /* user defined aggregate */

				// 	  if (aexp->u.call.shared != SHARED_INVOCATION) {
				// 	    /* shared aggregates are not put into aggr_list */
				// 	    appendElementList(aggr_list, (nt_obj_t*)aexp);
				// 	  }
				// 	}
			}
		}
	}

	if (a->prd_list && final_prd_list) {
		//throw error
		rc = ERR_INVALID_PARTITION_USE;
		EM_error(
				a->pos,
				rc,
				__LINE__,
				__FILE__,
				"You are using both partition by and group by, which is not allowed, even if they are the same.");
		goto exit;
	} else if (final_prd_list) {
		a->prd_list = final_prd_list;
	}

	if (has_aggr == 0)
		count_only_aggr_p = 0;

	//this tumble detection is done elsewhere,
	// so let's wait for now
	if (aggrCnt == 1 && aggrExp->u.call.win != NULL && aggrExp->u.call.win->slide
			!= NULL) {
		A_win w = aggrExp->u.call.win;
		double wsize = w->range->size;
		if (wsize == w->slide->size) {
			//pane based optimization
			//in this case we should init and then iter till end of window then
			//terminate and then again init...
			paneBasedOpt = 1;
		}
	}
	/*
	   if(paneBasedOpt == 1) {
	   char wname[80];
	   sprintf(wname, S_name(aggrExp->u.call.func));
	   wname[strlen(wname) - strlen("_window")] = '\0';


//lookup base version
E_enventry base = (E_enventry)S_look(venv, S_Symbol(wname));
if(!base) paneBasedOpt = 0;

//if a non-windowed version exists, then we can optimize
}
	 */
if (paneBasedOpt == 1) {
	printf("we have detected a tumble %s\n", S_name(aggrExp->u.call.func));
}

/* there's only one QUN under GB node */
qun = (A_qun) getNthElementList(a->jtl_list, 0);
qun->ppnode = a;

getQunName(qun, qunname);
/*printf("--------Calling qun again\n");
  displayQun(qun);
  printf("\n--------done\n");*/
rc = transQun(venv, tenv, qun, sql, subdec, subexe, qunname, (Tr_exp*) 0,
		(Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, aggregates, srcs, NULL, cstmt);
//printf("--------back from the again qun\n");
if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr", "transQun");
	goto exit;
}

/* declare qun under the current node */
declareQun2C(qunname, subexe->ty, buf, DCL_EXPIRE);
exe = expTy_Seq(exe, buf);

//appendElementList(sql->index_list, (nt_obj_t*)a);
addSqlSemIndexDec(sql, (void*) a);

// compile each aggr
for (i = 0; i < aggr_list->length; i++) {
	char target[80];
	Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);

	sprintf(target, "_databuf", UID(a), S_name(aggr_info->aggr->u.call.func), i);
	rc = transAggrArgs(venv, tenv, sql, aggr_info, target, aggregates);

	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transAggrArgs");
		goto exit;
	}
}

if (!isESL() || sql->in_func == 1) {
	sprintf(buf, "\nnext_%d:"
			"\nwhile (index_%d>=0 && index_%d < %d) {", UID(a), UID(a), UID(a),
			aggr_list->length + 1);
} else if (isESL()) {
	sprintf(buf, "\nnext_%d:"
			"\nwhile (index_%d>=0 && index_%d < %d && !freeVars) {"
			/*"\n// nlaptev(TODO): Verify that this fixes the time-based window slide output.
			// The problem was that even when slide was defined to be X seconds/minutes, the output
			// was still produced every time an input was received. slide_out = 1;"*/,
			UID(a), UID(a), UID(a), aggr_list->length + 1);
}

exe = expTy_Seq(exe, buf);

/* switch structure
   case 0: Get a tuple from the underlying qun, and call
   aggr_init, aggr_iterate, aggr_terminate. Return values
   are put into the "return" relation of the aggr routine.
   case 1:
   ...
   case n: For each of the 1..n aggr in the GB box, get a tuple
   `             from its "return" relation.
 */
sprintf(buf, "\nswitch(index_%d) {"
		"\ncase 0:\n{", UID(a));
exe = expTy_Seq(exe, buf);

sprintf(buf, "\nif (terminating_%d == 0) {", UID(a));
exe = expTy_Seq(exe, buf);

/* get source tuple from qun */
exe = expTy_Seq(exe, "\n/* get source tuple from qun */");
exe = expTy_Seq(exe, subexe);

/* make assignments of non-aggr head expr */
if (!isESL())
	sprintf(buf, "\nif (rc==0) {"
			"\nfirst_entry_%d = 0;"
			"\n/* make assignments of non-aggr head expr */", UID(a));
	else
	sprintf(buf, "\nif (rc==0) {"
			"\n/* make assignments of non-aggr head expr */");
	//  	  "\ngbstatus_%d = (struct gb_status_%d *)0;"
	//  	  , UID(a), UID(a));
	exe = expTy_Seq(exe, buf);

	t_fields = A_List();

	//printf("doing select items\n");
	for (j = 0; j < a->hxp_list->length; j++) {
		A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, j);
		//    Ty_field curfield;

		if (arg->kind == SIMPLE_ITEM && arg->u.s.exp->kind != A_callExp) {
			T_expty d, e;
			rc = transExp(venv, tenv, arg->u.s.exp, sql, d, e, aggregates);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr", "transExp");
				goto exit;
			}

			// make assignments
			char *pField;
			if (arg->u.s.alias == (S_symbol) 0) {
				char field[20];
				sprintf(field, "field_%d", i);
				curfield = Ty_Field(new_Symbol(field), e->ty, e->size);
				pField = field;
				//	sprintf(buf, "\n%s.%s = %s;", name, field, e->exp);
			} else {
				curfield = Ty_Field(arg->u.s.alias, e->ty, e->size);
				pField = S_name(arg->u.s.alias);
				//	sprintf(buf, "\n%s.%s = %s;", name, S_name(arg->u.s.alias), e->exp);
			}
			switch (e->ty->kind) {
				case Ty_int:
				case Ty_ref:
					sprintf(buf, "\n%s.%s = %s;", name, pField, e->exp);
					break;
				case Ty_string:
					sprintf(buf, "\nmemcpy(%s.%s, %s, %d);"
							"\n%s.%s[%d]=0;", name, pField, e->exp, e->size, name, pField,
							e->size);//string length
					break;
				case Ty_timestamp:
					sprintf(buf, "\nmemcpy(&(%s.%s), &%s, %d);", name, pField, e->exp,
							sizeof(struct timeval));
					break;
				default:
					rc = ERR_DATATYPE;
					EM_error(0, rc, __LINE__, __FILE__, "transGBOpr()");
			}
			appendElementList(t_fields, (nt_obj_t*) curfield);
			exe = expTy_Seq(exe, buf);
		}
	}

//printf("done with select items\n");
/* merge group-by columns into a key */
exe = expTy_Seq(exe, "\n/* merge group-by columns into a key */");

if (!A_ListEmpty(a->prd_list)) {
	rc = transGenGBKey(venv, tenv, a, a->prd_list, sql, name, buf, // code for merge group-by columns
			decodebuf, // code for decompose group-by columns
			keysize, aggregates);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transGenGBKey");
		goto exit;
	}
} else {
	/* TODO: if there is no group-by column, we shall not use hashtable!! */
	strcpy(buf, "\nstrcpy(gbkey, \"____\");");
	*decodebuf = '\0';
	keysize = 4;
}
exe = expTy_Seq(exe, buf);

/* retrieve gbstatus via key */
sprintf(buf, "\ngbstatus_%d = (struct gb_status_%d *)0;"
		"\nrc = hash_get(%d, _rec_id, gbkey, %d, (char**)&gbstatus_%d);", UID(a),
		UID(a), UID(a), keysize, UID(a));
exe = expTy_Seq(exe, buf);

/* if new group found, create new gb_status */
sprintf(buf, "\nif (rc == DB_NOTFOUND) {//blah"
		"\ngbstatus_%d = (struct gb_status_%d*)malloc(sizeof(*gbstatus_%d));",
		UID(a), UID(a), UID(a));
exe = expTy_Seq(exe, buf);

//printf("initializing UDA states\n");
/* initialize states for each UDA */
for (i = 0; i < aggr_list->length; i++) {
	Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);

	if (!aggr_info->builtin) {
		A_exp aggr = aggr_info->aggr;
		strcpy(fname, S_name(aggr->u.call.func));
		A_win win = aggr->u.call.win;
		int winsize;
		A_slide slide;
		int slidesize = 1;

		if (aggr->u.call.init != (char*) 0) {
			exe = expTy_Seq(exe, "\n//he");
			exe = expTy_Seq(exe, aggr->u.call.init);
			exe = expTy_Seq(exe, "\n//he|");
		}

		if (win != (A_win) 0) {
			winsize = win->range->size;
			slide = win->slide;
			if (slide)
				slidesize = slide->size;
		}

		char ffname[80];
		sprintf(ffname, fname);
		ffname[strlen(ffname) - strlen("_window")] = '\0';

		//lookup base version
		E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

		//BaseOpt: check slide > window size here as well
		if (win && slide && slide->size >= winsize && winsize != -1 && base
				&& base->kind == E_aggrEntry) {
			sgtw = true;
			sprintf(buf,
					"\ngbstatus_%d->%s_%d = (struct %s_status*)malloc(sizeof(struct %s_status));"
					"\ngbstatus_%d->%s_%d->win = 0;", UID(a), ffname, i, ffname,
					ffname, UID(a), ffname, i);
		} else {
			/* create status for each aggregate */
			sprintf(buf,
					"\ngbstatus_%d->%s_%d = (struct %s_status*)malloc(sizeof(struct %s_status));"
					"\ngbstatus_%d->%s_%d->win = 0;", UID(a), fname, i, fname, fname,
					UID(a), fname, i);
		}

		if (win) {
			//we use separate buffer for each partition by, for range
			//windows the tuples are still expired eagerly using an external linked list
			if (win->range->type == COUNT_RANGE || (win->range->type == TIME_RANGE
						&& win->range->size != -1)) {
				if (slide && slide->size >= winsize && winsize != -1 && base
						&& base->kind == E_aggrEntry) {
					sprintf(linebuf,
							"\ngbstatus_%d->%s_%d->win = new winbuf(%d, %d, %d, %s);",
							UID(a), ffname, i, win->range->size, aggr_info->windata_size,
							keysize, (win->range->type == COUNT_RANGE) ? "_ADL_WIN_ROW"
							: "_ADL_WIN_TIME");
					strcat(buf, linebuf);
					sprintf(linebuf, "\ngbstatus_%d->%s_%d->last_out = 0;"
							"\ngbstatus_%d->%s_%d->iterate = false;"
							"\ngbstatus_%d->%s_%d->init = true;", UID(a), ffname, i, UID(a),
							ffname, i, UID(a), ffname, i);
				} else {
					sprintf(linebuf,
							"\ngbstatus_%d->%s_%d->win = new winbuf(%d, %d, %d, %s);",
							UID(a), fname, i, win->range->size, aggr_info->windata_size,
							keysize, (win->range->type == COUNT_RANGE) ? "_ADL_WIN_ROW"
							: "_ADL_WIN_TIME");
					strcat(buf, linebuf);
					sprintf(linebuf, "\ngbstatus_%d->%s_%d->last_out = 0;"
							"\ngbstatus_%d->%s_%d->iterate = false;"
							"\ngbstatus_%d->%s_%d->init = true;", UID(a), fname, i, UID(a),
							fname, i, UID(a), fname, i);
				}
				strcat(buf, linebuf);
			}
			/*
			   else if(win->range->size != -1) { // time-based and not unlimited (signified by -1)
			   sprintf(linebuf, "\ngbstatus_%d->%s_%d->win = _adl_win_%s_%d_%d;",
			   UID(a), fname, i,
			   fname, i, UID(a));
			   strcat(buf, linebuf);

			   }
			 */
		} // end if win
		exe = expTy_Seq(exe, buf);
	} else {
		/* initialize states for each builtin Aggregate */
		// nothing to be initialized
		A_exp aggr = aggr_info->aggr;
		strcpy(fname, S_name(aggr->u.call.func));
		A_win win = aggr->u.call.win;
		int winsize;
		A_slide slide;
		int slidesize = 1;

		if (win != (A_win) 0) {
			winsize = win->range->size;
			slide = win->slide;
			if (slide)
				slidesize = slide->size;
		}
		E_enventry base = (E_enventry) S_look(venv, S_Symbol(fname));
		if (win && win->range) {
			if (win->range->type == COUNT_RANGE || (win->range->type == TIME_RANGE
						&& win->range->size != -1)) {
				sprintf(buf, "\ngbstatus_%d->%s_%d_win = new winbuf(%d, %d, %d, %s);"
						"\ngbstatus_%d->%s_%d_last_out=0;"
						"\ngbstatus_%d->%s_%d_iterate=false;"
						"\ngbstatus_%d->%s_%d_init=true;", UID(a), fname, i,
						win->range->size, aggr_info->windata_size, keysize,
						(win->range->type == COUNT_RANGE) ? "_ADL_WIN_ROW"
						: "_ADL_WIN_TIME", UID(a), fname, i, UID(a), fname, i,
						UID(a), fname, i);
				exe = expTy_Seq(exe, buf);
				// TODO(nlaptev): Verify that below is the fix for outputting the correct number of tuples when
				// sliding window is used.
				// add buf to exe.
				sprintf(
						buf,
						"\nif((!(%d <= 1 || ((gbstatus_%d->%s_%d_win->getTupleID() == 0 && gbstatus_%d->%s_%d_last_out != 0) "
						"\n||((((int)gbstatus_%d->%s_%d_win->getTupleID()) >= gbstatus_%d->%s_%d_last_out + %d)))))) {"
						"\nslide_out = 0;"
						"\n} else {"
						"\nslide_out = 1;"
						"\n}", slidesize, UID(a), fname, i, UID(a), fname, i, UID(a),
						fname, i, UID(a), fname, i, slidesize);

				exe = expTy_Seq(exe, buf);
			}
		}
	}
}

if (has_aggr) {
	/* PHASE init */
	//    rc = transCallAggr(venv, tenv, a, sql, AGGR_INIT, aggr_list, builtin_list, builtin_type, buf);
	//TODO: ideally we can put the partable initialization here
	rc = transCallAggr(venv, tenv, a, sql, AGGR_INIT, aggr_list, buf,
			aggregates, keysize);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transCallAggr");
		goto exit;
	}
	exe = expTy_Seq(exe, buf);
}
//printf("done initializing UDA states, except iterate, terminate\n");

/* put updated gbstatus into hash table */
sprintf(buf, "\nrc = hash_put(%d, _rec_id, gbkey, %d, &gbstatus_%d);",
		UID(a), keysize, UID(a));
exe = expTy_Seq(exe, buf);

/* PHASE iterate */
if (!has_aggr) {
	/* do nothing for the iterate phase*/
	if (isESL()) {
		sprintf(
				buf,
				"\n} else if(rc != 0) {"
				"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: hash->get()\");"
				"\nreturn s_failure;"
				"\n}", getUserName(), getQueryName());
	} else if (isESLAggr()) {
		sprintf(
				buf,
				"\n} else if(rc != 0) {"
				"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: hash->get()\");"
				"\nreturn;"
				"\n}", getUserName(), getAggrName());
	} else {
		sprintf(buf, "\n} else if(rc != 0) adlabort(rc, \"hash->get()\");");
	}
	exe = expTy_Seq(exe, buf);
} else {
	exe = expTy_Seq(exe, "\n} else if (rc == 0) {"
			"\n/* PHASE iterate */");
	/*
	   for (i=0; i<aggr_list->length; i++) {
	   A_exp aggr = (A_exp)getNthElementList(aggr_list, i);

	   sprintf(fname, "dbstatus_%d->%s_%d->ret",
	   UID(a),
	   S_name(aggr->u.call.func), i);

	   rc = transTabRemove2C(fname, buf);
	   if (rc) {
	   EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr", "transTabRemove2C");
	   goto exit;
	   }
	   exe = expTy_Seq(exe, buf);
	   }*/

	/* call aggr_iterate */
	rc = transCallAggr(venv, tenv, a, sql, AGGR_ITERATE, aggr_list, buf,
			aggregates, keysize);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transCallAggr");
		goto exit;
	}
	exe = expTy_Seq(exe, buf);

	if (isESL()) {
		sprintf(
				buf,
				"\n} else {"
				"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: hash->get()\");"
				"\nreturn s_failure;"
				"\n}", getUserName(), getQueryName());
	} else if (isESLAggr()) {
		sprintf(
				buf,
				"\n} else {"
				"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: hash->get()\");"
				"\nreturn;"
				"\n}", getUserName(), getAggrName());
	} else {
		sprintf(buf, "\n} else adlabort(rc, \"hash->get()\");");
	}
	exe = expTy_Seq(exe, buf);
}

//here is where we put "last" tuple in win (before if terminating)
sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
		"\nterminating_%d = 1;"
		"\n}"
		"\n}"
		"\nif (terminating_%d == 1) {", UID(a), UID(a));
exe = expTy_Seq(exe, buf);

if (count_only_aggr_p) {
	sprintf(buf, "\nif (first_entry_%d == 1) {"
			"\nrc = 0; /* fail on first entry, aggregate on empty set */"
			"\n} else {", UID(a));
	exe = expTy_Seq(exe, buf);
}

/* PHASE terminate */
sprintf(buf, "\nallkey = (char*)0;"
		"\nrc = hash_get(%d, _rec_id, allkey, %d, (char**)&gbstatus_%d);"
		"\nif (rc==0) {", UID(a), keysize, UID(a));
exe = expTy_Seq(exe, buf);

if (has_aggr) {
	/* call aggr_terminate */
	rc = transCallAggr(venv, tenv, a, sql, AGGR_TERMINATE, aggr_list, buf,
			aggregates, keysize);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transCallAggr");
		goto exit;
	}
	exe = expTy_Seq(exe, buf);
}
exe = expTy_Seq(exe, decodebuf);

if (isESL()) {
	sprintf(
			buf,
			"\n} else if(rc == DB_NOTFOUND) {"
			"\n} else {"
			"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: hash->get()\");"
			"\nreturn s_failure;"
			"\n}"
			"\n}", getUserName(), getQueryName());
} else if (isESLAggr()) {
	sprintf(
			buf,
			"\n} else if(rc == DB_NOTFOUND) {"
			"\n} else {"
			"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: hash->get()\");"
			"\nreturn;"
			"\n}"
			"\n}", getUserName(), getAggrName());
} else {
	sprintf(buf, "\n} else if(rc == DB_NOTFOUND) {"
			"\n} else adlabort(rc, \"hash->get()\");"
			"\n}");
}
exe = expTy_Seq(exe, buf);

//exe = expTy_Seq(exe, "\n} else if (rc == DB_NOTFOUND) {"
//		  "\n} else adlabort(rc, \"hash_get()\");"
//	  "\n}");

if (count_only_aggr_p) {
	exe = expTy_Seq(exe, "\n}");
}

exe = expTy_Seq(exe, "\n}\nbreak;");
/* end of case 0 */

/* case 1 to n, get final results of aggregates */
for (i = 0; i < aggr_list->length; i++) {
	Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
	A_exp aggr = aggr_info->aggr;

	sprintf(buf, "\ncase %d:\n{", i + 1); //+builtin_list->length);
	exe = expTy_Seq(exe, buf);

	//sprintf(buf, "\nprintf(\"case3 %d\\n\");fflush(stdout);", i+1);
	//exe = expTy_Seq(exe, buf);

	if (aggr_info->builtin) {
		rc = getBuiltInFinalResult(aggr_info, i, a, t_fields, count_only_aggr_p,
				name, exe);
	} else {
		rc = getUDAFinalResult(venv, tenv, aggr_info, i, a, t_fields, name, exe);
	}

	if (i + 1 == aggr_list->length) {
		/* this is the last aggr */
		if (!isESL()) {
			sprintf(buf, "\nfirst_entry_%d = 0;", UID(a));
			exe = expTy_Seq(exe, buf);
		}
	}

	exe = expTy_Seq(exe, "\n}\nbreak;");
}

if (sgtw == true) {
	sprintf(buf, "\n} /*end of switch*/"
			"\nif (rc == 0 && slide_out == 1) {"
			"\nindex_%d++;", UID(a));
} else {
	sprintf(buf, "\n} /*end of switch*/"
			"\nif (rc == 0) {"
			"\nindex_%d++;", UID(a));
}
exe = expTy_Seq(exe, buf);

if (!has_aggr) {
	/* fail unless terminating == 1 */
	sprintf(buf, "\nif (terminating_%d ==0) rc = DB_NOTFOUND;", UID(a));
	exe = expTy_Seq(exe, buf);
}

sprintf(buf, "\n}"
		"\nif (rc == DB_NOTFOUND) {"
		"\nindex_%d--;"
		"\nif (terminating_%d == 1 && index_%d == 0) {", UID(a), UID(a), UID(a));
exe = expTy_Seq(exe, buf);

/* clean up status data structure of each aggregate. The local
   variables are cleaned up in the terminate routine, and we take
   care of the "return" table and its cursor here. */
for (i = 0; i < aggr_list->length; i++) {
	Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
	A_exp aggr = aggr_info->aggr;

	if (!aggr_info->builtin) {
		char tmpname[80];
		strcpy(fname, S_name(aggr->u.call.func));
		A_win win = aggr->u.call.win;
		int winsize;
		A_slide slide;
		int slidesize = 1;

		if (win != (A_win) 0) {
			winsize = win->range->size;
			slide = win->slide;
			if (slide)
				slidesize = slide->size;
		}

		char ffname[80];
		sprintf(ffname, fname);
		ffname[strlen(ffname) - strlen("_window")] = '\0';

		//lookup base version
		E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

		//BaseOpt: check slide > window size here as well
		if (win && slide && slide->size >= winsize && winsize != -1 && base
				&& base->kind == E_aggrEntry) {
			sprintf(tmpname, "gbstatus_%d->%s_%d->retc", UID(a), ffname, i);
		} else {
			/* close cursor of each "return" table */
			sprintf(tmpname, "gbstatus_%d->%s_%d->retc", UID(a), fname, i);
		}

		transCursorClose2C(tmpname, buf);
		exe = expTy_Seq(exe, buf);

		/* close "return" table of each aggregate */
		if (win && slide && slide->size >= winsize && winsize != -1 && base
				&& base->kind == E_aggrEntry) {
			sprintf(buf,
					"\nsprintf(_adl_dbname, \"._%%d_ret\", gbstatus_%d->%s_%d);",
					UID(a), ffname, i);
		} else {
			sprintf(buf,
					"\nsprintf(_adl_dbname, \"._%%d_ret\", gbstatus_%d->%s_%d);",
					UID(a), fname, i);
		}
		exe = expTy_Seq(exe, buf);

		if (win && slide && slide->size >= winsize && winsize != -1 && base
				&& base->kind == E_aggrEntry) {
			sprintf(tmpname, "gbstatus_%d->%s_%d->ret", UID(a), ffname, i);
		} else {
			sprintf(tmpname, "gbstatus_%d->%s_%d->ret", UID(a), fname, i);
		}
		transTabClose2C(tmpname, "_adl_dbname", buf, TAB_LOCAL);

		exe = expTy_Seq(exe, buf);
	}
}

sprintf(buf, "\nrc = DB_NOTFOUND;"
		"\n}\n}\n}/*end of while */"
		"\nif (rc == 0) index_%d--;", UID(a));
exe = expTy_Seq(exe, buf);

/* if this GB is immediately inside an EXISTS/NOT EXISTS operator,
   then we need to clean up the hash table even if rc==0. This is
   because only one tuple is required here.

   the GB_EXISTS flag is set in transSelOpr()
 */

if ((!isESL() || sql->in_func == 1) && (a->distinct & GB_EXISTS_FLAG) == 0) {
	sprintf(buf, "\nelse ");
	exe = expTy_Seq(exe, buf);
} else if (isESL()) {
	sprintf(buf, "\nif(freeVars) rc = 1;");
	exe = expTy_Seq(exe, buf);
}

if (!isESL() || sql->in_func == 1)
	sprintf(buf, "\n{"
			"\nint rc;		/* local rc */ "
			"\nterminating_%d = 0;"
			"\nfirst_entry_%d = 1;"
			"\nindex_%d = 0;"
			"\n/* free gbstatus */"
			"\ndo {"
			"\nallkey = (char*)0;"
			"\nrc = hash_get(%d, _rec_id, allkey, %d, (char**)&gbstatus_%d);"
			"\nif (rc==0) {", UID(a), UID(a), UID(a), UID(a), keysize, UID(a));
else if (isESL())
	sprintf(buf, "\nif(freeVars){"
			"\nint rc;		/* local rc */ "
			"\nfreed_gbstatus = 1;"
			"\nterminating_%d = 0;"
			"\nindex_%d = 0;"
			"\n/* free gbstatus */"
			"\ndo {"
			"\nallkey = (char*)0;"
			"\nrc = hash_get(%d, _rec_id, allkey, %d, (char**)&gbstatus_%d);"
			"\nif (rc==0) {", UID(a), UID(a), UID(a), keysize, UID(a));
	exe = expTy_Seq(exe, buf);

	/* deallocate window, if any */
	/* We do not use this window anymore,
	   Note to self: however window deallocation is pending in current scheme
	   rc = transWindowRemove(a, aggr_list, exe);
	   if (rc) {
	   EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr", "transWindowRemove");
	   goto exit;
	   }
	 */
	/* deallocate the partition list if any */
	rc = transTimeExpListRemove(a, aggr_list, exe);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
				"transTimeExpListRemove");
		goto exit;
	}

/* release states for each UDA */
for (i = 0; i < aggr_list->length; i++) {
	Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
	A_exp aggr = aggr_info->aggr;
	A_win win = aggr->u.call.win;
	char fname[80];
	strcpy(fname, S_name(aggr->u.call.func));
	int winsize;
	A_slide slide;
	int slidesize = 1;

	if (win != (A_win) 0) {
		winsize = win->range->size;
		slide = win->slide;
		if (slide)
			slidesize = slide->size;
	}

	char ffname[80];
	sprintf(ffname, fname);
	ffname[strlen(ffname) - strlen("_window")] = '\0';

	//lookup base version
	E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

	//BaseOpt: here as well
	if (!aggr_info->builtin) {
		strcpy(fname, S_name(aggr->u.call.func));
		buf[0] = 0;
		if (win) {
			if (win && slide && slide->size >= winsize && winsize != -1 && base
					&& base->kind == E_aggrEntry) {
				sprintf(buf, "\nif(gbstatus_%d->%s_%d) {"
						"\nif(gbstatus_%d->%s_%d->win) {"
						"\ndelete(gbstatus_%d->%s_%d->win);"
						"\ngbstatus_%d->%s_%d->win = 0;"
						"\n}", UID(a), ffname, i, UID(a), ffname, i, UID(a), ffname, i,
						UID(a), ffname, i);
			} else {
				sprintf(buf, "\nif(gbstatus_%d->%s_%d) {"
						"\nif(gbstatus_%d->%s_%d->win) {"
						"\ndelete(gbstatus_%d->%s_%d->win);"
						"\ngbstatus_%d->%s_%d->win = 0;"
						"\n}", UID(a), fname, i, UID(a), fname, i, UID(a), fname, i,
						UID(a), fname, i);
			}
		}
		if (win && slide && slide->size >= winsize && winsize != -1 && base
				&& base->kind == E_aggrEntry) {
			sprintf(linebuf, "\nfree(gbstatus_%d->%s_%d);\n}", UID(a), ffname, i);
		} else if (win) {
			sprintf(linebuf, "\nfree(gbstatus_%d->%s_%d);\n}", UID(a), fname, i);
		} else {
			sprintf(linebuf, "\nfree(gbstatus_%d->%s_%d);", UID(a), fname, i);
		}
		strcat(buf, linebuf);
		exe = expTy_Seq(exe, buf);
	}
}
sprintf(buf, "\n//printf(\"freeing %d\\n\");"
		"\nfree(gbstatus_%d);"
		"\n}\n} while (rc==0);"
		"\nif (rc != DB_NOTFOUND) {", UID(a), UID(a));
exe = expTy_Seq(exe, buf);

if (isESL()) {
	sprintf(
			buf,
			"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: hash->get()\");"
			"\nreturn s_failure;"
			"\n}", getUserName(), getQueryName());
} else if (isESLAggr()) {
	sprintf(
			buf,
			"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: hash->get()\");"
			"\nreturn;"
			"\n}", getUserName(), getAggrName());
} else {
	sprintf(buf, "\nadlabort(rc, \"hash->get()\");"
			"\n}");
}
exe = expTy_Seq(exe, buf);

sprintf(buf, "\n/* release hash entry */"
		"\nhashgb_delete(%d, _rec_id);"
		"\n}", UID(a));
exe = expTy_Seq(exe, buf);

/* set the return type of GB opr */
exe->ty = Ty_Record(t_fields);

/* define window structure */
/* we don't use this kind of window anymore
   rc = transWindow(a, aggr_list, sql->predec);
   if (rc) {
   EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr", "transWindow");
   goto exit;
   }
 */

rc = transTimeExpList(a, aggr_list, sql->predec);
if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transGBOpr",
			"transTimeExpList");
	goto exit;
}

/* define gb_status data structure */
sprintf(buf, "\nint terminating_%d=0;"
		"\nstruct gb_status_%d {", UID(a), UID(a), UID(a));
sql->predec = expTy_Seq(sql->predec, buf);

//BaseOpt: may be here check for slide >= winsize

/* for UDAs, we use a structure to hold their states*/
for (i = 0; i < aggr_list->length; i++) {
	Aggr_info_t aggr_info = (Aggr_info_t) getNthElementList(aggr_list, i);
	A_exp aggr = aggr_info->aggr;
	char fname[80];
	strcpy(fname, S_name(aggr->u.call.func));
	A_win win = aggr->u.call.win;
	int winsize;
	A_slide slide;
	int slidesize = 1;

	if (win != (A_win) 0) {
		winsize = win->range->size;
		slide = win->slide;
		if (slide)
			slidesize = slide->size;
	}

	char ffname[80];
	sprintf(ffname, fname);
	ffname[strlen(ffname) - strlen("_window")] = '\0';

	//lookup base version
	E_enventry base = (E_enventry) S_look(venv, S_Symbol(ffname));

	/*if (win && slide && slide->size >= winsize && winsize != -1 &&
	  base && base->kind == E_aggrEntry) {
	  aggr->u.call.func = S_Symbol(ffname);
	  }*/

	//BaseOpt: check slide > window size here as well

	if (!aggr_info->builtin) {
		char cursorname[80];

		if (win && slide && slide->size >= winsize && winsize != -1 && base
				&& base->kind == E_aggrEntry) {
			sprintf(buf, "\nstruct %s_status *%s_%d;", ffname, /* name of aggr */
					ffname, /* name of aggr */
					i /* differentiate multiple invocation
					     of a same aggregate */
			       );
		} else {
			sprintf(buf, "\nstruct %s_status *%s_%d;", fname, /* name of aggr */
					fname, /* name of aggr */
					i /* differentiate multiple invocation
					     of a same aggregate */
			       );
		}
		sql->predec = expTy_Seq(sql->predec, buf);
	} else {
		/* for builtin aggregate, we need only an integer value and a
		   first_entry flag*/
		char stype[20];
		switch (aggr_info->builtin_type) {
			case Ty_int:
				strcpy(stype, "int");
				break;
			case Ty_real:
				strcpy(stype, "double");
				break;
			case Ty_string:
				sprintf(stype, "char");
				break;
			case Ty_timestamp:
				sprintf(stype, "struct timeval");
				break;
			case Ty_iext:
				sprintf(stype, "struct iExt_");
				break;
			case Ty_rext:
				sprintf(stype, "struct rExt_");
				break;
			case Ty_cext:
				sprintf(stype, "struct cExt_");
				break;
			case Ty_text:
				sprintf(stype, "struct tExt_");
				break;
			default:
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
						"Not supported buildin aggr data type", "TransGBOpr");
				break;
		}
		char winDec[512];
		sprintf(winDec, "\nwinbuf *%s_%d_win;", fname, i);
		if (aggr_info->builtin_type == Ty_string)
			sprintf(buf, "\n%s _baggr_%d_value[%d];"
					"\nint _baggr_%d_first_entry;"
					"%s", stype, i, MAX_STR_LEN, i, (win && win->range
						&& win->range->size != -1) ? winDec : "");
		else if (aggr_info->builtin == AGGR_BUILTIN_VAR) {
			sprintf(buf, "\n%s _baggr_%d_value;"
					"\ndouble _baggr_%d_value_sum;"
					"\nint _baggr_%d_value_cnt;"
					"\ndouble _baggr_%d_value_avg;"
					"\nint _baggr_%d_first_entry;"
					"%s", stype, i, i, i, i, i, (win && win->range && win->range->size
						!= -1) ? winDec : "");
		} else
			sprintf(buf, "\n%s _baggr_%d_value;"
					"\nint _baggr_%d_first_entry;"
					"%s", stype, i, i,
					(win && win->range && win->range->size != -1) ? winDec : "");
		sql->predec = expTy_Seq(sql->predec, buf);
		sprintf(buf, "\nint %s_%d_last_out;"
				"\nbool %s_%d_iterate;"
				"\nbool %s_%d_init;", fname, i, fname, i, fname, i);
		sql->predec = expTy_Seq(sql->predec, buf);
	}
}

if (!isESL())
	sprintf(buf, "\n};"
			"\nstruct gb_status_%d *gbstatus_%d = (struct gb_status_%d *)0;"
			"\n", UID(a), UID(a), UID(a));
	else
	sprintf(buf, "\n};"
			"\nstatic struct gb_status_%d *gbstatus_%d = (struct gb_status_%d *)0;"
			"\n", UID(a), UID(a), UID(a));

	sql->predec = expTy_Seq(sql->predec, buf);
	if (isESL()) {
		dec = expTy_Seq(dec, sql->predec);
	}

/* everything is done */
S_endScope(tenv);
S_endScope(venv);

exit: clearList(aggr_list);
deleteList(aggr_list);
//   clearList(builtin_list);
//   deleteList(builtin_list);
DBUG_RETURN(rc);
return rc;
}

char* getOuterAggregates(vector<void*> aggregates) {
	int size = aggregates.size();
	char ret[500];
	ret[0] = (char) NULL;
	for (int i = 0; i < size; i++) {
		S_symbol name = (S_symbol) aggregates.operator[](i);
		strcat(ret, "struct ");
		strcat(ret, S_name(name));
		strcat(ret, "_status *");
		strcat(ret, S_name(name));
		strcat(ret, "_Inst, ");
	}
	return strdup(ret);
}

err_t transAggrDec(S_table venv, S_table tenv, A_dec d, T_expty &edec,
		T_expty &einit, T_expty &eclean, int inaggr, vector<void*> aggregates,
		int default_win) {
	err_t rc = ERR_NONE;
	char argdef[4096];
	char handlename[80], dbname[80];
	char buf[LARGE_MAX_STR_LEN];

	T_expty lc_dec, lc_init, lc_clean, adec;
	T_expty curdec, curexe;
	curdec = (T_expty) 0;
	curexe = (T_expty) 0;
	A_list formalTys = A_List();
	Ty_ty windowTy;
	A_list declist = (A_list) 0;
	T_expty aggrdec = (T_expty) 0;
	char* fbuf;
	A_list reflist = A_List(0, A_EXP);
	Ty_ty resultTy;
	int i;
	int aggr_routines = 0;
	E_enventry te, x;
	char internalname[128];

	Sql_sem sql = SqlSem();
	sql->in_func = 1;
	sql->func_name = strdup(S_name(d->name));
	//printf("func_name set to %s\n", sql->func_name);

	/* return type of aggr (it is a record type) */
	//note the last arg in tabvar, since the result
	//of an aggregate is like a table thus iExt, etc are nto allowed
	rc = transTy(venv, tenv, d->u.aggr.result, resultTy, (A_list) 0, (int*) 0,
			A_tabVarDec);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec", "transTy");
		goto exit;
	}

	/* since resultTy is the type of the return table, we need to give
	   it a key (duplicates allowed) */
	if (resultTy->u.record->length > 0) {
		Ty_field f = (Ty_field) getNthElementList(resultTy->u.record, 0);
		f->iskey = 1;
	}

	/* arguments of aggr */
	*argdef = '\0';
	if (d->u.aggr.params) {
		rc = transFields(venv, tenv, d->u.aggr.params, formalTys, (A_list) 0,
				(int*) 0, A_aggregateDec); /* (A_list)0, (int*)0 */
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
					"transFields");
			goto exit;
		}
		/* c-type arguments list */
		rc = TyList2C(formalTys, argdef);
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec", "TyList2C");
			goto exit;
		}
		strcat(argdef, ",");
	}

	/* add aggr into symbol table now (so that it can be called
	   recursively in the aggr routines.) */

	if (d->u.aggr.init)
		aggr_routines |= AGGR_INIT;
	if (d->u.aggr.expire)
		aggr_routines |= AGGR_EXPIRE;
	if (d->u.aggr.terminate)
		aggr_routines |= AGGR_TERMINATE;
	if (d->u.aggr.iterate) {
		aggr_routines |= AGGR_ITERATE;
		if (d->u.aggr.init == d->u.aggr.iterate)
			aggr_routines |= AGGR_INIT_ITERATE;
	}

	if (d->u.aggr.type == A_window)
		S_enter(venv, d->name, E_AggrEntry(d->name, formalTys, resultTy,
					aggr_routines, inaggr, 1, 0, default_win));
	else
		S_enter(venv, d->name, E_AggrEntry(d->name, formalTys, resultTy,
					aggr_routines, inaggr, 0, 0, default_win));

	/*
	 * Add aggr arguments into symbol table.

	 * Aggr arguments are visible to local variables (during initialization).
	 */
	S_beginScope(venv);
	S_beginScope(tenv);

	/* add formal args into symbol table */
	for (i = 0; i < formalTys->length; i++) {
		Ty_field f = (Ty_field) getNthElementList(formalTys, i);
		S_enter(venv, f->name, E_VarEntry(f->name, f->ty, f->size));
	}

#ifdef TERMINATE_NO_ARGS
	/*
	 * Add aggr local variables into symbol table
	 * Local variables are visible in all the 3 aggr routines.
	 */
	S_beginScope(venv);
	S_beginScope(tenv);
#endif

	aggregates.push_back(d->name);

	/* local variables of this aggregate */
	if (!A_ListEmpty(d->u.aggr.decs)) {
		declist = d->u.aggr.decs;
		rc = transSeqDec(venv, tenv, declist, lc_dec, lc_init, lc_clean, 1,
				aggrdec, aggregates /* in aggr : delay transTabInit2C() */
				);
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
					"transSeqDec");
			goto exit;
		}

		/* All local variables are put into a structure (status) and passed
		   into aggregate routines as arguments, we need to rename a local
		   variable x to status->x

		   code moved to transDec() for efficiency.

		   for (i=0; i<declist->length; i++) {
		   A_dec lv = (A_dec)getNthElementList(declist, i);

		   sprintf(internalname, "status->%s", S_name(lv->name));
		   rc = setVarName(venv, lv->name, TAB_INAME, new_Symbol(internalname));
		   if (rc) {
		   EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec", "setInternalName");
		   goto exit;
		   }
		   }
		 */
	}
	aggregates.pop_back();

	/****************************************************************/
	/* "return" is the table to hold return values of the aggregate */
	/****************************************************************/
	te = E_VarEntry(S_Symbol("return"), resultTy, 0);
	te->u.var.inaggr = 1;

	strcpy(internalname, "status->ret");
	te->u.var.iname = new_Symbol(internalname);

	/* Now that we are using IMDB for hash tables, we need to define
	 * this flag in the symbol table
	 */
	te->u.var.scope = TAB_MEMORY;

	S_enter(venv, S_Symbol("return"), te);
	/****************************************************************/
	/* "expired" is the table to hold expired tuples in a window    */
	/****************************************************************/
	// We don't support expired table any more
	//windowTy = Ty_Record(formalTys);
	//te = E_VarEntry(S_Symbol("expired"), windowTy, 0);
	//te->u.var.inaggr = 1;
	//S_enter(venv, S_Symbol("expired"), te);
	//te->u.var.scope = TAB_EXPIRED;
	/****************************************************************/
	/* "inwindow" is the table to hold all tuples in a window         */
	/****************************************************************/
	te = (E_enventry) S_look(venv, S_Symbol("inwindow"));
	if (d->u.aggr.type == A_window) {
		if (d->u.aggr.window == NULL) { //inwindow not defined explicitly by user, the we define a default
			A_list li = A_List();
			int length = formalTys->length;
			for (int i = 0; i < length; i++) {
				Ty_field t = (Ty_field) getNthElementList(formalTys, i);

				Ty_field f = Ty_Field(t->name, t->ty, t->size);
				appendElementList(li, (nt_obj_t*) f);
			}

			/*Does not look like we need this extra _tid field
			  A_field fi = A_Field(d->pos, S_Symbol("_tid"),
			  S_Symbol("int"), sizeof(int));
			  Ty_ty ty = checkType(tenv, fi->typ);
			  if (!ty) {
			  rc = ERR_UNDEFINED_TYPE;
			  EM_error(fi->pos, rc, __LINE__, __FILE__, S_name(fi->typ));
			  goto exit;
			  }
			  int size = 0;
			  if (size <= 0) {
			  size = getDisplaySize(ty);
			  }

			  Ty_field f = Ty_Field(fi->name, ty, size);
			  appendElementList(li, (nt_obj_t*)f);*/
			windowTy = Ty_Record(li);

			te = E_VarEntry(S_Symbol("inwindow"), windowTy, 0, TAB_MEMORY);
			te->u.var.inaggr = 1;
			S_enter(venv, S_Symbol("inwindow"), te);
			te->u.var.scope = TAB_WINDOW;
		} else {
			//here almost same as above but borrow field names from
			//d->u.aggr.window and also the index dec
			A_list li = A_List();
			int length = formalTys->length;
			A_dec window = d->u.aggr.window;
			A_ty wTy = window->u.tabvar.ty;
			if (length != wTy->u.record->length) {
				rc = ERR_DATATYPE;
				EM_error(0, rc, __LINE__, __FILE__,
						"Invalid window type, must match aggregate input schema.");
				goto exit;
			}
			for (int i = 0; i < length; i++) {
				Ty_field t = (Ty_field) getNthElementList(formalTys, i);
				A_field wF = (A_field) getNthElementList(wTy->u.record, i);

				Ty_field f = Ty_Field(wF->name, t->ty, t->size);
				appendElementList(li, (nt_obj_t*) f);
			}

			/*Does not look like we need this extra _tid field
			  A_field fi = A_Field(d->pos, S_Symbol("_tid"),
			  S_Symbol("int"), sizeof(int));
			  Ty_ty ty = checkType(tenv, fi->typ);
			  if (!ty) {
			  rc = ERR_UNDEFINED_TYPE;
			  EM_error(fi->pos, rc, __LINE__, __FILE__, S_name(fi->typ));
			  goto exit;
			  }
			  int size = 0;
			  if (size <= 0) {
			  size = getDisplaySize(ty);
			  }

			  Ty_field f = Ty_Field(fi->name, ty, size);
			  appendElementList(li, (nt_obj_t*)f);*/
			windowTy = Ty_Record(li);

			te = E_VarEntry(S_Symbol("inwindow"), windowTy, 0, TAB_MEMORY,
					(A_index) 0, 0);
			//(window->u.tabvar.keydecs == (A_list)0)? 0:1);
			te->u.var.inaggr = 1;
			/* Ignoring keys of inwindow for now, only thing that is needed to do this is
			   to update genComposeCode and genGetCode funcs in aggr_info.cc
			   rc = checkTabIndex(window, windowTy, te);
			   if (rc) {
			   EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec", "checkTabIndex");
			   goto exit;
			   }
			   te->u.var.firstKey = window->u.tabvar.keyPos[0];
			 */
			S_enter(venv, S_Symbol("inwindow"), te);
			te->u.var.scope = TAB_WINDOW;
			windowTy = te->u.var.ty;
		}
		/*else {
		  A_list windowTys = A_List();
		  rc = transFields(venv, tenv, d->u.aggr.window->u.tabvar.ty->u.record,
		  windowTys);
		  S_symbol keySym = (S_symbol)getNthElementList(d->u.aggr.window->u.tabvar.keydecs, 0);
		  int length = windowTys->length;
		  for(int i = 0; i <length; i++) {
		  Ty_field f = (Ty_field)getNthElementList(windowTys, i);
		  if(keySym == f->name) {
		  f->iskey = 1;
		  }
		  }

		  if (rc) {
		  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec", "transFields");
		  goto exit;
		  }
		  windowTy = Ty_Record(windowTys);
		  }*/

		if (!isESL() && !isAdHoc()) {
			sprintf(buf, "\nstruct inWinType_%s {\n", S_name(d->name));
			edec = expTy_Seq(edec, buf);
			for (int i = 0; i < windowTy->u.record->length; i++) {
				char argdef[128];
				Ty_field f = (Ty_field) getNthElementList(windowTy->u.record, i);
				rc = TyName2C(f, argdef);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
							"TyName2C");
					goto exit;
				}
				sprintf(buf, "%s;\n", argdef);
				edec = expTy_Seq(edec, buf);
			}
			edec = expTy_Seq(edec, "};");
		}
	}

	if (d->u.aggr.is_c_aggr && d->u.aggr.c_global_decs) {
		edec = expTy_Seq(edec, d->u.aggr.c_global_decs);
	}

	/* declare the status structure */
	sprintf(buf, "\nstruct %s_status {", S_name(d->name));
	if (declist && lc_dec) {
		strcat(buf, lc_dec->exp);
	}
	if (d->u.aggr.is_c_aggr && d->u.aggr.c_decs) {
		strcat(buf, d->u.aggr.c_decs);
	}
	strcat(buf, "\nwinbuf *win;" // window construct
			"\nint last_out;"
			"\nbool iterate;"
			"\nbool init;"
			"\nIM_REL *ret;" // "return" table
			"\nIM_RELC *retc;" // cursor on "return" table
			"\nint retc_first_entry;"
			"\n};");
	edec = expTy_Seq(edec, buf);

	/* put the aggregates inside the aggregate first */
	if (declist && aggrdec) {
		sprintf(buf, "\n%s\n", aggrdec->exp);
		edec = expTy_Seq(edec, buf);
	}

	//Only if we are in non ESL, nonAdHoc case
	//Also, only translate if isESLAggr and aggregate name matches
	//we use strncase instead of strcase, because we have to also allow
	//  aggrName_window

	if (!isESL() && !isAdHoc()) {

		if (d->u.aggr.type == A_window) {
			char fieldBuf[4096];
			int keyoff = 0, dataoff = 0;
			sprintf(
					buf,
					"\ninWinType_%s* getLastTuple_%s(IM_REL* inwindow, inWinType_%s* tuple, bufferMngr* bm) {"
					"\nint rc;"
					"\nDBT key, data;"
					"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];"
					"\nIM_RELC* temp;", S_name(d->name), S_name(d->name), S_name(
						d->name));
			edec = expTy_Seq(edec, buf);

			sprintf(buf, "\n\nmemset(&key, 0, sizeof(key));"
					"\nmemset(&data, 0, sizeof(data));"
					"\ndata.data = datadata;"
					"\nkey.data = keydata;"
					"\nif ((rc = inwindow->cursor(inwindow, &temp, 0)) != 0) {");
			edec = expTy_Seq(edec, buf);

			if (isESL()) {
				sprintf(
						buf,
						"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->cursor()\");"
						"\nreturn NULL;"
						"\n}", getUserName(), getQueryName());
			} else if (isESLAggr()) {
				sprintf(
						buf,
						"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->cursor()\");"
						"\nreturn NULL;"
						"\n}", getUserName(), getAggrName());
			} else {
				sprintf(buf, "\nadlabort(rc, \"IM_REL->cursor()\");"
						"\n}");
			}
			edec = expTy_Seq(edec, buf);

			sprintf(buf, "\nrc = temp->c_get(temp, &key, &data, DB_FIRST);"
					"\nif (rc == DB_NOTFOUND) {");
			edec = expTy_Seq(edec, buf);

			if (isESL()) {
				sprintf(
						buf,
						"\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IMREL->c_get() in oldest()\");"
						"\n//return NULL;"
						"\n}", getUserName(), getQueryName());
			} else if (isESLAggr()) {
				sprintf(
						buf,
						"\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IMREL->c_get() in oldest()\");"
						"\n//return NULL;"
						"\n}", getUserName(), getAggrName());
			} else {
				sprintf(buf, "\nadlabort(rc, \"IMREL->c_get() in oldest()\");"
						"\n}");
			}
			edec = expTy_Seq(edec, buf);

			buf[0] = '\0';
			//assignField2C
			for (int i = 0; i < windowTy->u.record->length; i++) {
				Ty_field fi = (Ty_field) getNthElementList(windowTy->u.record, i);
				rc = assignField2C(fi, "(*tuple)", S_name(fi->name), &keyoff, &dataoff,
						fieldBuf);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
							"assignField2C");
					goto exit;
				}
				strcat(buf, fieldBuf);
			}
			edec = expTy_Seq(edec, buf);

			sprintf(buf, "\nreturn tuple;"
					"\n}");
			edec = expTy_Seq(edec, buf);
		}

		/* Forward declare all the aggr routines. This is necessary because
		   aggregates can be recursive */
		fbuf = getOuterAggregates(aggregates);

		if (d->u.aggr.init) {
			sprintf(
					buf,
					"\nextern \"C\" void %s_init(%sstruct %s_status *status, "
					"\n\t%s int _rec_id, int __is_init=1, bufferMngr* bm=NULL, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, "
					"\n\tvector<A_timeexp>* plist=NULL, int endSlide=0, "
					"\n\tchar* _modelId=NULL);", S_name(d->name), fbuf,
					S_name(d->name), argdef);
			edec = expTy_Seq(edec, buf);
		}

		if (d->u.aggr.iterate && d->u.aggr.iterate != d->u.aggr.init) {
			sprintf(
					buf,
					"\nextern \"C\" void %s_iterate(%sstruct %s_status *status, "
					"\n\t%s int _rec_id, bufferMngr* bm=NULL, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, "
					"\n\tvector<A_timeexp>* plist=NULL, int endSlide=0, "
					"\n\tchar* _modelId=NULL);", S_name(d->name), fbuf,
					S_name(d->name), argdef);
			edec = expTy_Seq(edec, buf);
		}
		if (d->u.aggr.expire) {
			sprintf(
					buf,
					"\nextern \"C\" void %s_expire(%sstruct %s_status *status, "
					"\n\t%s int _rec_id, bufferMngr* bm=NULL, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, "
					"\n\tvector<A_timeexp>* plist=NULL, int endSlide=0, "
					"\n\tchar* _modelId=NULL);", S_name(d->name), fbuf,
					S_name(d->name), argdef);
			edec = expTy_Seq(edec, buf);
		}
		if (d->u.aggr.terminate) {

#ifdef TERMINATE_NO_ARGS
			sprintf(buf, "\nextern \"C\" void %s_terminate(%sstruct %s_status *status, "
					"\n\tint _rec_id, int not_delete = 0, bufferMngr* bm=NULL, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, "
					"\n\tvector<A_timeexp>* plist=NULL, int endSlide=0,
					"\n\tchar* _modelId=NULL);",
					S_name(d->name), fbuf, S_name(d->name));
#else
			/* Instead of the entire argdef, only constants should be passed
			   to ther terminate routine, however, we are not making the
			   distinction here. */
			sprintf(
					buf,
					"\nextern \"C\" void %s_terminate(%sstruct %s_status *status, "
					"\n\t%s int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, "
					"\n\tvector<A_timeexp>* plist=NULL, int endSlide=0,"
					"\n\tchar* _modelId=NULL);", S_name(d->name), fbuf,
					S_name(d->name), argdef);
#endif
			edec = expTy_Seq(edec, buf);
		}

		/* compile init routine */
		if (d->u.aggr.init) {
			//if c aggregate no compilation here, just simple paste into curexe
			if (d->u.aggr.is_c_aggr == 1) {
				if (d->u.aggr.c_init) {
					curexe = (T_expty) 0;
					curexe = expTy_Seq(curexe, d->u.aggr.c_init);
				}
			} else {
				rc = transExp(venv, tenv, d->u.aggr.init, sql, curdec, curexe,
						aggregates);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
							"transExp");
					goto exit;
				}
			}
			if (d->u.aggr.type == A_window) {
				sprintf(buf,
						"\nextern \"C\" void %s_init(%sstruct %s_status *status, %s "
						"\n\tint _rec_id, int __is_init, bufferMngr* bm, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
						"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nIM_REL *window;"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;"
						"\nif(status && status->win)"
						"\nwindow = status->win->get_im_rel();"
						"\nstruct inWinType_%s tuple;", S_name(d->name), fbuf, S_name(
							d->name), argdef, S_name(d->name));
			} else {
				sprintf(buf,
						"\nextern \"C\" void %s_init(%sstruct %s_status *status, %s"
						"\n\tint _rec_id, int __is_init, bufferMngr* bm, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
						"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nIM_REL *window;"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;", S_name(d->name), fbuf, S_name(d->name),
						argdef);
			}
			edec = expTy_Seq(edec, buf);

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_init\", (int)status, 0);",
						S_name(d->name));
				edec = expTy_Seq(edec, buf);
			}

			sprintf(buf, "\nif (__is_init) {");
			edec = expTy_Seq(edec, buf);

			/* open all local tables */
			for (i = 0; declist && i < declist->length; i++) {
				A_dec lv = (A_dec) getNthElementList(declist, i);
				if (lv->kind != A_aggregateDec && lv->kind != A_dynamicVarDec) {
					int haskey = (lv->u.tabvar.index == (A_index) 0) ? 0 : 1;

					sprintf(handlename, "status->%s", S_name(lv->name));

					/* the dbname is encoded by status */
					sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_%s\", status);", S_name(
								lv->name));
					edec = expTy_Seq(edec, buf);

					tabindex_t index = (tabindex_t) 1;
					if (lv->u.tabvar.index != (A_index) 0)
						index = lv->u.tabvar.index->kind;
					/* these local tables are already compiled but their transTabInit2C() are delayed */
					transTabInit2C(handlename, "_adl_dbname", haskey,
							//(haskey)? DB_BTREE: DB_RECNO,
							buf, lv->u.tabvar.scope, index, Ty_nil, 1, lv->u.tabvar.isView,
							lv->name);
					/* only local tables in aggr, therefore we don't need to set BTREE Comparison Function */

					edec = expTy_Seq(edec, buf);
				}
			}
			/* open "return" table */
			sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_ret\", status);");
			edec = expTy_Seq(edec, buf);
			/* call newly defined function for imdb */
			transTabInit2C_imdb("status->ret", "_adl_dbname", TAB_LOCAL, 0, /* no key defined for "return" table, duplicates
											   allowed */
					buf);
			edec = expTy_Seq(edec, buf);
			/* open cursor of "return" table */

			/* since we are dealing explicitly with return table
			 * we can pass in the "TAB_MEMORY" flag here
			 */
			transCursorInit2C("status->ret", "status->retc", buf, TAB_MEMORY);

			edec = expTy_Seq(edec, buf);

			if (d->u.aggr.decs && d->u.aggr.decs->length > 0) {
				edec = expTy_Seq(edec, lc_init);
			}

			sprintf(buf, "\n} /* end of __is_init */");
			edec = expTy_Seq(edec, buf);

			if (curdec)
				edec = expTy_Seq(edec, curdec->exp);

			//      if (d->u.aggr.decs) {
			//        edec = expTy_Seq(edec, lc_init);
			//      }

			if (curexe)
				edec = expTy_Seq(edec, curexe->exp);

			edec = expTy_Seq(edec, "\nstatus->retc_first_entry=1;");

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_init\", (int)status, 1);",
						S_name(d->name));
				edec = expTy_Seq(edec, buf);
			}

			edec = expTy_Seq(edec, "\n}");
		}

		/* iterate */
		if (d->u.aggr.iterate && d->u.aggr.iterate != d->u.aggr.init) {
			//if c aggregate no compilation here, just simple paste into curexe
			if (d->u.aggr.is_c_aggr == 1) {
				if (d->u.aggr.c_iter) {
					curexe = (T_expty) 0;
					curexe = expTy_Seq(curexe, d->u.aggr.c_iter);
				}
			} else {
				rc = transExp(venv, tenv, d->u.aggr.iterate, sql, curdec, curexe,
						aggregates);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
							"transExp");
					goto exit;
				}
			}

			if (d->u.aggr.type == A_window) {
				sprintf(buf,
						"\nextern \"C\" void %s_iterate(%sstruct %s_status *status, "
						"\n\t%s int _rec_id, bufferMngr* bm, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
						"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nIM_REL *window;"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;"
						"\nif(status && status->win)"
						"\nwindow = status->win->get_im_rel();"
						"\nstruct inWinType_%s tuple;", S_name(d->name), fbuf, S_name(
							d->name), argdef, S_name(d->name));
			} else {
				sprintf(buf,
						"\nextern \"C\" void %s_iterate(%sstruct %s_status *status, "
						"\n\t%s int _rec_id, bufferMngr* bm, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
						"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nIM_REL *window;"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;", S_name(d->name), fbuf, S_name(d->name),
						argdef);
			}

			edec = expTy_Seq(edec, buf);

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_iterate\", (int)status, 0);", S_name(
							d->name));
				edec = expTy_Seq(edec, buf);
			}

			if (curdec)
				edec = expTy_Seq(edec, curdec);
			if (curexe)
				edec = expTy_Seq(edec, curexe);
			edec = expTy_Seq(edec, "\nstatus->retc_first_entry=1;");

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_iterate\", (int)status, 1);", S_name(
							d->name));
				edec = expTy_Seq(edec, buf);
			}
			edec = expTy_Seq(edec, "\n}");
		}

		/* expire */
		if (d->u.aggr.expire) {
			//if c aggregate no compilation here, just simple paste into curexe
			if (d->u.aggr.is_c_aggr == 1) {
				if (d->u.aggr.c_expire) {
					curexe = (T_expty) 0;
					curexe = expTy_Seq(curexe, d->u.aggr.c_expire);
				}
			} else {
				rc = transExp(venv, tenv, d->u.aggr.expire, sql, curdec, curexe,
						aggregates);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
							"transExp");
					goto exit;
				}
			}

			if (d->u.aggr.type == A_window) {
				sprintf(buf,
						"\nextern \"C\" void %s_expire(%sstruct %s_status *status, "
						"\n\t%s int _rec_id, bufferMngr* bm, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
						"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nIM_REL *window;"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;"
						"\nif(status && status->win)"
						"\nwindow = status->win->get_im_rel();"
						"\nstruct inWinType_%s tuple;", S_name(d->name), fbuf, S_name(
							d->name), argdef, S_name(d->name));
			} else {
				sprintf(buf,
						"\nextern \"C\" void %s_expire(%sstruct %s_status *status, "
						"\n\t%s int _rec_id, bufferMngr* bm, "
						"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
						"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;", S_name(d->name), fbuf, S_name(d->name),
						argdef);

			}
			edec = expTy_Seq(edec, buf);

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_iterate\", (int)status, 0);", S_name(
							d->name));
				edec = expTy_Seq(edec, buf);
			}

			if (curdec)
				edec = expTy_Seq(edec, curdec);
			if (curexe)
				edec = expTy_Seq(edec, curexe);
			edec = expTy_Seq(edec, "\nstatus->retc_first_entry=1;");

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_expire\", (int)status, 1);", S_name(
							d->name));
				edec = expTy_Seq(edec, buf);
			}
			edec = expTy_Seq(edec, "\n}");
		}

#ifdef TERMINATE_NO_ARGS
		/* delete aggr arguments from the symbol table. (We do not pass aggr
		   arguments into terminate routine) */
		S_swapScope(tenv);
		S_swapScope(venv);
#endif

		/* terminate */
		if (d->u.aggr.terminate) {
			//if c aggregate no compilation here, just simple paste into curexe
			if (d->u.aggr.is_c_aggr == 1) {
				if (d->u.aggr.c_term) {
					curexe = (T_expty) 0;
					curexe = expTy_Seq(curexe, d->u.aggr.c_term);
				}
			} else {
				rc = transExp(venv, tenv, d->u.aggr.terminate, sql, curdec, curexe,
						aggregates);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrDec",
							"transExp");
					goto exit;
				}
			}
#ifdef TERMINATE_NO_ARGS
			sprintf(buf, "\nextern \"C\" void %s_terminate(%sstruct %s_status *status, "
					"\n\tint _rec_id, int not_delete, bufferMngr* bm, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
					"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)"
					, S_name(d->name), fbuf, S_name(d->name));
#else
			sprintf(buf,
					"\nextern \"C\" void %s_terminate(%sstruct %s_status *status, "
					"\n\t%s int _rec_id, int not_delete, bufferMngr* bm, "
					"\n\thash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, "
					"\n\tvector<A_timeexp>* plist, int endSlide, char* _modelId)",
					S_name(d->name), fbuf, S_name(d->name), argdef);
#endif
			edec = expTy_Seq(edec, buf);

			if (d->u.aggr.type == A_window) {
				sprintf(buf, "\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nIM_REL *window;"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;"
						"\nif(status && status->win)"
						"\nwindow = status->win->get_im_rel();"
						"\nstruct inWinType_%s tuple;", S_name(d->name));
			} else {
				sprintf(buf, "\n{"
						"\nint rc;"
						"\nint _adl_sqlcode, _adl_cursqlcode;"
						"\nDBT key, data, windata;"
						"\nRect r_key;"
						"\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
						"\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
						"\nchar _adl_dbname[80];"
						"\nint slide_out = 1;"
						"\nwinbuf* rwinbuf = NULL;"
						"\nint rlast_out = 0;");
			}
			edec = expTy_Seq(edec, buf);

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_terminate\", (int)status, 0);", S_name(
							d->name));
				edec = expTy_Seq(edec, buf);
			}

			if (curdec)
				edec = expTy_Seq(edec, curdec->exp);
			if (curexe)
				edec = expTy_Seq(edec, curexe->exp);

			/* close all local tables */
			for (i = 0; declist && i < declist->length; i++) {
				A_dec lv = (A_dec) getNthElementList(declist, i);
				if (lv->kind != A_aggregateDec) {
					sprintf(handlename, "status->%s", S_name(lv->name));

					/* the dbname is encoded by status */
					sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_%s\", status);", S_name(
								lv->name));
					edec = expTy_Seq(edec, buf);

					/* call S_look to get the scope (local/imdb/bdb) */
					x = (E_enventry) S_look(venv, lv->name);

					transTabClose2C(handlename, "_adl_dbname", buf, x->u.var.scope,
							INDEX_BTREE, 1);
					edec = expTy_Seq(edec, buf);
				}
			}

			edec = expTy_Seq(edec, "\nif(!not_delete) status->retc_first_entry=1;");

			if (ntsys->verbose) {
				sprintf(buf, "\nadltrace(\"%s_terminate\", (int)status, 1);", S_name(
							d->name));
				edec = expTy_Seq(edec, buf);
			}

			edec = expTy_Seq(edec, "\n}");
		}
	}

	/* delete aggr local variables from the symbol table */
	S_endScope(tenv);
	S_endScope(venv);

	SqlSem_Delete(sql);

exit: return rc;
}
