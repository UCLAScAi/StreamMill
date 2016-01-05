#include "querySchdl.h"
#include "ios/ios.h"
#include <basic.h>
#include <stdlib.h>
#include <iostream>
#include <stmt.h>
#include <driver.h>
#include <strategy.h>
#include <monitor.h>
#include <compiler.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <adllib.h>
//#include <hash_map.h>
#include <ext/hash_map>

//Added by Hamid: 7/20/09
#include <SMLog.h>

using namespace std;
using namespace __gnu_cxx;

namespace ESL {

extern FILE* stdoutLog;
extern FILE* stderrLog;
extern char logFileName[100];

//these are for experiments only
extern bool exp_batch_activate_mode;
extern int activated;
extern int num_queries;

querySchdl* querySchdl::_instance = 0;
int querySchdl::verbose = 0;

querySchdl* querySchdl::getInstance() {
  if (!_instance) {
    _instance = new querySchdl();
    if (!_instance) {
      perror("malloc querySchdl");
      return NULL;
    }
  }
  return _instance;
}
int querySchdl::destroy() {
  if (_instance) {
    delete _instance;
    _instance = 0;
  }
}
int querySchdl::init() {
  dm = DrvMgr::getInstance();
  s = NULL;
}

querySchdl::querySchdl() {
  init();
}

int querySchdl::run() {
  bufferMngr *bm = bufferMngr::getInstance();

  //create global performance related buffers
  bm->create(TOTAL_OUTPUT_TUPLE, SHARED);
  bm->create(AVG_LATENCY, SHARED);
  bm->create(MAX_LATENCY, SHARED);
  bm->create(LAST_TOTAL_BUF_BYTES, SHARED);

  //redirecting stdout to a file
  stdoutLog = freopen(qsLog, "a", stdout);
  stderrLog = freopen(qsLog, "a", stderr);

  buffer* qBuf = bm->lookup("_queryBuffer");
  buffer* iob = bm->lookup("_ioBuffer");
  bm->create("stdout", SHARED);

  int messageSize = sizeof(int) + (MAX_BUF_COUNT + 2) * MAX_ID_LEN;
  int code;
  char id[MAX_ID_LEN];
  char id2[MAX_BUF_COUNT * MAX_ID_LEN];
  char id3[MAX_ID_LEN];
  cDBT cdbt(messageSize);

  SMLog::SMLOG(0, "starting to log. level is %i", SMLOG_LEVEL );
  SMLog::SMLOG(0, "---------R-U-N-(-)----S-T-A-R-T-S----------");

  while (1) {
    list<Driver*>* drvs = dm->getDrivers();

    while (qBuf->empty() && (drvs == NULL || drvs->size() == 0)) {
      //no driver exists yet.
      usleep(100);
    }

    while (!(qBuf->empty()) || (exp_batch_activate_mode && activated > 0
        && activated < num_queries)) {
      if (qBuf->empty())
        continue;

      SMLog::SMLOG(10, "--------------GETTING--NEW--COMMAND------------------");
      if (querySchdl::verbose)
        cout << "entering processing one command" << endl;

      fflush(stdout);
      fflush(stderr);

      //Parsing the command
      int rc = qBuf->get(&cdbt);
      char string_to_write[100];
      char filename[strlen(getUserName())];
      if (rc == 0) {
        memcpy(&code, cdbt.data, sizeof(int));
        strcpy(id, (char*) cdbt.data + sizeof(int));
        strcpy(id2, (char*) cdbt.data + sizeof(int) + strlen(id) + 1);
        strcpy(id3, (char*) cdbt.data + sizeof(int) + strlen(id) + 1 + strlen(
            id2) + 1);
        qBuf->pop();

        SMLog::SMLOG(8,
            "cmd gotten:\n\tcode: %i\n\tid: %s\n\tid2: %s\n\tid3: %s ", code,
            id, id2, id3);
        sprintf(string_to_write,
            "Command received -- code: %i id: %s id2: %s id3: %s\n", code, id,
            id2, id3);
        sprintf(filename, "%sQS.log", getUserName());
        WriteToLog(string_to_write, filename);

        if (querySchdl::verbose) {
          cout << "get command: " << endl;
          cout << "\tcode: " << code << endl;
          cout << "\tid:  " << id << endl;
          cout << "\tid2: " << id2 << endl;
          cout << "\tid3: " << id3 << endl;
        }
        fflush(stdout);
      } else {
        fprintf(stderr, "ERROR: could not read queryBuffer\n");
        continue;
      }

      //executing the command
      if (code == SET_USER_NAME) {
        setUserName(id);
      } else if (code == ADD_QUERY_CMD_CODE) {
        ADD_QUERY_CMD_CODE_hndl(bm, id);
      } else if (code == SNAP_SHOT_CMD_CODE) {
        SNAP_SHOT_CMD_CODE_hndl(bm, id);
      } else if (code == ADD_DECLARE_CMD_CODE) {
        ADD_DECLARE_CMD_CODE_hndl(bm, qBuf, id, code);
      } else if (code == ACTIVATE_QUERY_CMD_CODE) {
        ACTIVATE_QUERY_CMD_CODE_hndl(bm, id);
      } else if (code == DEACTIVATE_QUERY_CMD_CODE) {
        DEACTIVATE_QUERY_CMD_CODE_hndl(bm, id);
      } else if (code == DROP_QUERY_CMD_CODE) {
        DROP_QUERY_CMD_CODE_hndl(bm, id);
      } else if (code == DROP_DECLARE_CMD_CODE) {
        DROP_DECLARE_CMD_CODE_hndl(bm, id);
      } else if (code == GET_COMPONENTS) {
        GET_COMPONENTS_hndl(bm, id);
      } else if (code == VIEW_COMPONENT_DETAILS) {
        VIEW_COMPONENT_DETAILS_hndl(bm, id);
      } else if (code == SET_COMPONENT_PRIORITY) {
        SET_COMPONENT_PRIORITY_hndl(bm, id, id2);
      } else if (code == MOVE_STMT_TO_COMPONENT) {
        MOVE_STMT_TO_COMPONENT_hndl(bm, id, id2, id3);
      } else if (code == BREAK_COMPONENT) {
        BREAK_COMPONENT_hndl(bm, id, id2);
      } else if (code == JOIN_COMPONENTS) {
        JOIN_COMPONENTS_hndl(bm, id, id2);
      } else if (code == MONITOR_PERFORMANCE_BUFFER) {
        MONITOR_PERFORMANCE_BUFFER_hndl(bm, id);
      } else if (code == UNMONITOR_PERFORMANCE_BUFFER) {
        UNMONITOR_PERFORMANCE_BUFFER_hndl(bm, id);
      }

      // If buffer is empty, we assume evrything went well... 
      // TODO: This is somewhat hacky...
      /*if (iob->empty()) {
        printf("Putting command_successful into buffer\n");
        fflush(stdout);
        iob->put(COMMAND_SUCCESSFUL);
      }*/

      if (querySchdl::verbose) {
        cout << "querySchdl::Run: end processing one command***" << endl;
        sprintf(string_to_write,
            "Handled command -- code: %i id: %s id2: %s id3: %s\n", code, id,
            id2, id3);
        WriteToLog(string_to_write, filename);
      }

    }

    if (!(qBuf->empty())) {
      if (querySchdl::verbose) {
        cout << "after while loop, qBuf not empty" << endl;
      }
    }

    //check to see if we need to update wakeup flag of the monitor
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    Monitor* monitor = dm->getMonitor();
    if (monitor->next_check_wakeup.tv_sec < tv.tv_sec) {
      monitor->next_check_wakeup.tv_sec = tv.tv_sec + WAKEUP_CYCLE_SEC;
      monitor->updateWakeup();
    }

    bool no_input = true;
    run_rc rc;

    for (list<Driver*>::iterator itr = drvs->begin(); itr != drvs->end(); itr++) {
      rc = runDriver(*itr);
      if (rc == run_success || rc == run_failure)
        no_input = false;

    }

    fflush(stdout);
    fflush(stderr);

    if (no_input)
      usleep(100);
  } // end while
  return 0;
}

//constant for use with scheduler testing for now
static const bool use_strategies = true;

//this function handles all creation and deletion of strategy memory
//do not forget to delete memory when a strategy is no longer used.
run_rc querySchdl::runDriver(Driver* drv) {
  //if (querySchdl::verbose && drv->stateTableChanged) cout << "4 ";
  Strategy* strategy;
  ScheduleStrategy* weightedStrategy;

  if (drv->isDirty() && use_strategies) {
    dm->getMonitor()->updateMaps();

    //treat it as preferring using weighted strategy over unweighted ones.
    if (drv->getStrategy(true) != NULL)
      drv->getStrategy(true)->update();
    else if (drv->getStrategy() != NULL)
      drv->getStrategy()->update();
  }

  //if (querySchdl::verbose && drv->stateTableChanged) cout << "5 ";

  if (use_strategies && drv->getStrategy(true) == NULL && drv->getStrategy()
      == NULL) {
    dm->getMonitor()->updateMaps();

    //old format strategies
    strategy = new OcsStrategy(drv);
    //strategy = new RRStrategy(drv, 50);
    //strategy = new OcsQuotaStrategy(drv);
    //strategy = new OcsRouletteSegStrategy(drv);
    //strategy = new PcsStrategy(drv);
    //strategy = new TrainStrategy(drv);
    //strategy = new OcsSegStrategy(drv);
    //strategy = new TrainSegStrategy(drv);
    //strategy = new TrainQuotaStrategy(drv);
    //strategy = new OcsSegNoOptmztnStrategy(drv);
    //strategy = new PcsSegStrategy(drv);

    //weighted strategies
    //weightedStrategy = new WeightOnlyWRRStrategy(drv);
    //weightedStrategy = new OCSWRRStrategy(drv);
    //weightedStrategy = new OCSWRRDLStrategy(drv);
    //weightedStrategy = new OCSPriorityStrategy(drv);
    //weightedStrategy = new OCSPriorityDLStrategy(drv);

    if (querySchdl::verbose) {
      cout << "initial strategy table info" << endl;
      strategy->printStateTable();
      //weightedStrategy->printStateTable();
    }
  }

  drv->resetDirty();

  //if (querySchdl::verbose && drv->stateTableChanged) cout << "6 ";
  run_rc rc;
  if ((rc = dm->run(drv)) == run_failure) {
    cout << "ERROR occurred when running driver" << endl;
  }
  return rc;
  //if (querySchdl::verbose && drv->stateTableChanged) cout << "2 ";
}

void querySchdl::ADD_QUERY_CMD_CODE_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::ADD_QUERY_CMD_CODE_hndl");
  int rc = compile(id);
  if (rc == COMMAND_SUCCESSFUL) {
    freeStmts.push_back(s);
  }
}

void querySchdl::SNAP_SHOT_CMD_CODE_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::SNAP_SHOT_CMD_CODE_hndl id: %s", id);

  compiler *cc = compiler::getInstance();
  char outFile[256];
  char filename[256];
  sprintf(filename, "./%s.so", id);
  sprintf(outFile, "./%s.out", id);

  int fd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR
      |S_IRGRP);
  if (fd > 0)
    close(fd);

  //printf("here1 %s\n", outFile);
  //fflush(stdout);

  cc->redirectStdout("./errorsFile", "w", stdoutLog);
  cc->redirectStderr("./errorsFile", "a", stderrLog);
  int ret = cc->compile(id, cmp_snapshot);
  stdoutLog = freopen(qsLog, "a", cc->getTempStdout());
  stderrLog = freopen(qsLog, "a", cc->getTempStderr());

  //printf("here after compile\n");
  //fflush(stdout);

  char errors[1024];
  char command[256];
  char* error;
  int rc;
  buffer* iob = bm->lookup("_ioBuffer");

  if (ret == 0) {
    sprintf(command, "./eslcc %s >& errorsFile", id);

    //printf("executing %s\n", command);
    //fflush(stdout);
    ret = system(command);
    if (ret == 0) {
      sprintf(command, "cp ../exe/%s.so %s.so", id, id);
      //printf("doing %s\n", command);
      //fflush(stdout);
      ret = system(command);
      if (ret == 0) {
        //printf("here %s\n", filename);
        //fflush(stdout);
        void *handle = dlopen(filename, RTLD_NOW);
        //printf("after dlopen\n");
        //fflush(stdout);
        if (!handle) {
          sprintf(errors, "In Snapshot query, %s\n", dlerror());
          printf("ERROR: %s\n", errors);
          fflush(stdout);
          ret = 1;
        } else {
          /*int fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
           if(fd > 0)
           close(fd);*/
          //printf("calling redirect\n");
          //fflush(stdout);

          cc->redirectStdout(outFile, "w", stdoutLog);
          cc->redirectStderr(outFile, "a", stderrLog);

          //printf("good here filename %s\n", filename);
          //fflush(stdout);
          int stmts = getAdHocNum(); //gives number of statements for snapshot
          int stmt;
          //printf("the snapshot has %d stmts.\n", stmts);
          //fflush(stdout);
          for (stmt = 0; stmt < stmts; stmt++) {
            char funcName[50];
            sprintf(funcName, "adhoc_%d", stmt);
            //printf("Looking for func %s\n", funcName);
            //fflush(stdout);
            int
                (*snapshot)(bufferMngr*, int, buffer*, hash_map<const char*,
                    void*, std::hash<const char*> , eqstrTab>*) =
                (int(*)(bufferMngr*, int, buffer*,
                        hash_map<const char*, void*, std::hash<const char*>, eqstrTab>*))dlsym(handle, funcName);

            if ((error = dlerror()) != NULL) {
              printf("dlerror in snapshot query, stmt %d\n", stmt);
              fflush(stdout);
              sprintf(errors, "In Snap shot query, %s\n", error);
              printf("ERROR: %s\n", errors);
              fflush(stdout);
              ret = 1;
              break;
            } else {
              /*printf("no dlerror in snapshot\n");
               fflush(stdout);
               bm->displayBufNames();*/
              //printf("calling function %s\n", funcName);
              //fflush(stdout);
              printf("Query %d:\n---------\n", stmt + 1);
              fflush(stdout);
              (*(snapshot))(bm, 0, NULL, getInMemTables());
              printf("---------\n\n");
              fflush(stdout);
              //printf("back from function\n");
              //fflush(stdout);
            }
          }
          if (stmt == stmts) {
            iob->put(COMMAND_SUCCESSFUL);
          }
          dlclose(handle);
          stdoutLog = freopen(qsLog, "a", cc->getTempStdout());
          stderrLog = freopen(qsLog, "a", cc->getTempStderr());
        }
      }
    }
  }

  if (ret != 0) {
    iob->put(COMMAND_FAILED);
  }

  //printf("here filename %s\n", filename);
  //fflush(stdout);

  //printf("here handle %d\n", handle);
  //fflush(stdout);
}

void querySchdl::ADD_DECLARE_CMD_CODE_hndl(bufferMngr* bm, buffer* qBuf,
    char * id, int code) {
  SMLog::SMLOG(10, "Entering querySchdl::ADD_DECLARE_CMD_CODE_hndl");
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;
  buffer *iob = bm->lookup("_ioBuffer");

  cc->redirectStdout("./errorsFile", "w", stdoutLog);
  cc->redirectStderr("./errorsFile", "a", stderrLog);
  int c_rc = cc->compile("__dummy__");
  stdoutLog = freopen(qsLog, "a", cc->getTempStdout());
  stderrLog = freopen(qsLog, "a", cc->getTempStderr());

  if (c_rc != 0) {
    if (querySchdl::verbose) {
      cout << "compile failed " << endl;
    }
    iob->put(COMMAND_FAILED);
  } else {
    while (qBuf->empty())
      usleep(100);
    qBuf->get(code, (char*) id);
    qBuf->pop();

    SMLog::SMLOG(8, "2nd cmd gotten:\n\tcode: %i\n\tid: %s ", code, id);
    if (querySchdl::verbose) {
      cout << "get command 1: " << endl;
      cout << "\tcode: " << code << endl;
      cout << "\tid: " << id << endl;
    }
    fflush(stdout);

    bool error = false;

    if (code == ADDED_BUFFER) {
      if (querySchdl::verbose) {
        cout << "get added_buffer, id is " << id << endl;
        cout << "freepool has " << freeStmts.size() << " statements" << endl;
      }

      //check to see if there is any statement using this id, and if so, recompile them all to make sure they still fit
      list<stmt*> toRemove;
      list<stmt*> toAdd;
      //list<stmt*> toRemove2;
      //list<stmt*> toAdd2;

      //check in free pool
      for (list<stmt*>::iterator itr = freeStmts.begin(); itr
          != freeStmts.end() && !error; itr++) {
        bool found = false;

        if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
          uStmt* s1 = static_cast<uStmt*> (*itr);
          int sub_count = s1->sub_stmts.size();

          for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
              != s1->sub_stmts.end(); itr2++) {
            if (strcmp((*itr2)->inBufName, id) == 0) {
              found = true;
            }
          }

          if (strcmp((*itr)->outBufName, id) == 0) {
            found = true;
          }
        } else if ((*itr)->type == stmt_coll) {
          cStmt* s1 = static_cast<cStmt*> (*itr);
          int sub_count = s1->sub_stmts.size();

          for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
              != s1->sub_stmts.end(); itr2++) {
            if ((*itr2)->type == stmt_t_union || ((*itr2)->type
                == stmt_tl_union)) {
              uStmt* s2 = static_cast<uStmt*> (*itr2);
              int sub_count2 = s2->sub_stmts.size();

              for (list<stmt*>::iterator itr3 = s2->sub_stmts.begin(); itr3
                  != s2->sub_stmts.end(); itr3++) {
                if (strcmp((*itr3)->inBufName, id) == 0) {
                  found = true;
                }
              }
              if (strcmp((*itr2)->outBufName, id) == 0) {
                found = true;
              }
            } else if (strcmp((*itr2)->inBufName, id) == 0) {
              found = true;
            }
          }
        } else if ((*itr)->type == stmt_join) {
          jStmt* s1 = static_cast<jStmt*> (*itr);
          int sub_count = s1->sub_stmts.size();

          for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
              != s1->sub_stmts.end(); itr2++) {
            if (strcmp((*itr2)->inBufName, id) == 0) {
              found = true;
            }
          }

          if (strcmp((*itr)->inBufName, id) == 0 || strcmp((*itr)->outBufName,
              id) == 0) {
            found = true;
          }
        } else if ((*itr)->type == stmt_normal) {
          if (strcmp((*itr)->inBufName, id) == 0 || strcmp((*itr)->outBufName,
              id) == 0) {
            found = true;
          }
          if (querySchdl::verbose) {
            //cout << "buffer names in statement " << (*itr)->name << " are: in " << (*itr)->inBufName << ", out " << (*itr)->outBufName << endl;
          }
        }
        if (found) {
          if (querySchdl::verbose) {
            cout << "try to recompile statement " << (*itr) -> name << endl;
          }
          int rc2 = compile((*itr) -> name);

          if (rc2 == COMMAND_SUCCESSFUL) {
            if (querySchdl::verbose) {
              cout << "recompile statement successful" << endl;
            }
            toRemove.push_back((*itr));
            toAdd.push_back(s);
          } else {
            if (querySchdl::verbose) {
              cout << "re-compile of existing statement failed " << endl;
            }
            error = true;
            break;
          }
        } else {
          if (querySchdl::verbose) {
            //cout << "new buffer not found in free pool" << endl;
          }
        }
      }

      if (querySchdl::verbose) {
        //cout << "after check free pool, toremove has : " << toRemove.size() << " statements, toadd has " << toAdd.size() << " statements" << endl;
      }

      if (!error) {
        //the old statement now has an invalid pointer, needs to be replaced in freepool
        for (list<stmt*>::iterator itr3 = toRemove.begin(); itr3
            != toRemove.end(); itr3++) {
          freeStmts.remove((*itr3));
        }

        for (list<stmt*>::iterator itr3 = toAdd.begin(); itr3 != toAdd.end(); itr3++) {
          freeStmts.push_back((*itr3));
        }
      }

      if (error) {
        if (querySchdl::verbose) {
          cout << "add declare command failed " << endl;
        }

        iob->put(COMMAND_FAILED);
      } else {
        iob->put(COMMAND_SUCCESSFUL);
      }
    } else if (code == COMPILE_SUCCESS || code == COMPILE_FAILURE) {
      //do nothing
    } else {
      if (querySchdl::verbose) {
        cout << "Internal error, id not recieved for stream adding" << endl;
      }
    }
  }

}

void querySchdl::ACTIVATE_QUERY_CMD_CODE_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::ACTIVATE_QUERY_CMD_CODE_hndl");
  //first look for it in the freeStmt pool
  list<stmt*> added;
  stmt* s2 = NULL;
  for (list<stmt*>::iterator itr = freeStmts.begin(); itr != freeStmts.end(); itr++) {
    printf("here to %s %s\n", (*itr)->name, id);
    if (strcmp((*itr)->name, id) == 0) {
      s2 = *itr;
      break;
    }
  }
  bool err = false;
  int rc = COMMAND_SUCCESSFUL;
  buffer *iob = bm->lookup("_ioBuffer");

  printf("rc here1 %d, %d\n", COMMAND_SUCCESSFUL, s2);
  //in activate, still check if need to do compliation
  if (s2 == NULL) {
    rc = compile(id);

    if (rc == COMMAND_SUCCESSFUL) {
      s2 = s;
    } else {
      err = true;
    }

  }

  //printf("HAT %d %d\n", s2, s2->type);

  //printf("rc here1 %d\n", COMMAND_SUCCESSFUL);
  //optional compilation done, now add stmt to driver, together with everything upstream of it.
  Driver *drv = NULL;
  if (!err) {
    list<stmt*> q;
    q.push_back(s2);
    list<char*> bufs;

    while (!err && !(q.empty())) {
      stmt* s3 = q.back();
      q.pop_back();

      if (s3->type == stmt_coll) {
        cStmt* s1 = static_cast<cStmt*> (s3);
        int sub_count = s1->sub_stmts.size();
        //printf("we here %d\n", sub_count);
        if (querySchdl::verbose) {
          cout << "there are " << sub_count
              << " sub statements in this coll statement." << endl;
        }

        for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); !err && itr
            != s1->sub_stmts.end(); itr++) {
          if (querySchdl::verbose) {
            cout << "current sub statement name is " << (*itr)->name << endl;
          }
          if (querySchdl::verbose) {
            cout << "add sub stmt to driver: " << (*itr)->name << endl;
          }
          if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
            uStmt* s2 = static_cast<uStmt*> (*itr);
            int sub_count2 = s2->sub_stmts.size();
            if (querySchdl::verbose) {
              cout << "there are " << sub_count2
                  << " sub statements in this union statement." << endl;
            }

            for (list<stmt*>::iterator itr2 = s2->sub_stmts.begin(); !err
                && itr2 != s2->sub_stmts.end(); itr2++) {
              if (querySchdl::verbose) {
                cout << "current sub statement name is " << (*itr2)->name
                    << endl;
              }
              if (querySchdl::verbose) {
                cout << "add sub stmt to driver: " << (*itr2)->name << endl;
              }

              else if (dm->addStmt(*itr2, drv) == SUCCESS) {
                if (querySchdl::verbose) {
                  drv->printStateTable();
                }
                added.push_back(*itr2);
                bufs.push_back((*itr2)->in->name);
              } else {
                err = true;
                break;
              }
            }
            if (!err && dm->addStmt(s2, drv) == SUCCESS) {
              //drv->setPriority(5);
              if (querySchdl::verbose) {
                if (drv != NULL)
                  drv->printStateTable();
              }
              added.push_back(s3);
            }
          } else if (dm->addStmt(*itr, drv) == SUCCESS) {
            if (querySchdl::verbose) {
              drv->printStateTable();
            }
            added.push_back(*itr);
            bufs.push_back((*itr)->in->name);
          } else {
            err = true;
            break;
          }
        }
      } else if (s3->type == stmt_t_union || s3->type == stmt_tl_union) {
        uStmt* s1 = static_cast<uStmt*> (s3);
        int sub_count = s1->sub_stmts.size();
        if (querySchdl::verbose) {
          cout << "there are " << sub_count
              << " sub statements in this union statement." << endl;
        }

        for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); !err && itr
            != s1->sub_stmts.end(); itr++) {
          if (querySchdl::verbose) {
            cout << "current sub statement name is " << (*itr)->name << endl;
          }
          if (querySchdl::verbose) {
            cout << "add sub stmt to driver: " << (*itr)->name << endl;
          }

          if (dm->addStmt(*itr, drv) == SUCCESS) {
            if (querySchdl::verbose) {
              drv->printStateTable();
            }
            added.push_back(*itr);
            bufs.push_back((*itr)->in->name);
          } else {
            err = true;
            break;
          }
        }
      } else if (s3->type == stmt_join) {
        jStmt* s1 = static_cast<jStmt*> (s3);
        int sub_count = s1->sub_stmts.size();
        if (querySchdl::verbose) {
          cout << "there are " << sub_count
              << " sub statements in this join statement." << endl;
        }

        for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); !err && itr
            != s1->sub_stmts.end(); itr++) {
          if (querySchdl::verbose) {
            cout << "current sub statement name is " << (*itr)->name << endl;
          }
          if (querySchdl::verbose) {
            cout << "add sub stmt to driver: " << (*itr)->name << endl;
          }

          if (dm->addStmt(*itr, drv) == SUCCESS) {
            if (querySchdl::verbose) {
              drv->printStateTable();
            }
            added.push_back(*itr);
            bufs.push_back((*itr)->in->name);
          } else {
            err = true;
            break;
          }
        }
      } else if (s3->type == stmt_normal) {
        printf("we here: %s\n", s3->in->name);
        bufs.push_back(s3->in->name);
      }

      //printf("rc here3 %d\n", COMMAND_SUCCESSFUL);
      if (!err && querySchdl::verbose) {
        cout << "add stmt to driver: " << s3->name << ", of type " << s3->type
            << ", with out buffer " << s3->out->name << endl;
      }

      if (s3->type == stmt_coll) {
        //drv->setPriority(5);
        if (querySchdl::verbose) {
          if (drv != NULL)
            drv->printStateTable();
        }
        added.push_back(s3);
      } else if (!err && dm->addStmt(s3, drv) == SUCCESS) {
        //drv->setPriority(5);
        if (querySchdl::verbose) {
          if (drv != NULL)
            drv->printStateTable();
        }
        added.push_back(s3);
      } else {
        err = true;
        break;
      }
      fflush(stdout);
      //now go through the free pool, look for upstream stmts
      bool found = false;
      for (list<stmt*>::iterator itr = freeStmts.begin(); itr
          != freeStmts.end(); itr++) {
        //check if it is already added, since we will not change free pool until the end,
        //when we know there is no error
        for (list<stmt*>::iterator itr2 = added.begin(); itr2 != added.end(); itr2++) {
          if ((*itr) == (*itr2)) {
            found = true;
            break;
          }
        }

        for (list<char*>::iterator itr3 = bufs.begin(); !found && itr3
            != bufs.end(); itr3++) {
          if ((*itr)->out && strcmp((*itr)->out->name, *itr3) == 0) {
            q.push_back(*itr);
            break;
          }
        }
      }
    }
  }
  if (!err) {
    //remove added ones from free pool
    for (list<stmt*>::iterator itr = added.begin(); itr != added.end(); itr++) {
      //inform ios it is activated.
      iob->put(ACTIVATED_QUERY, (*itr)->name);
      freeStmts.remove((*itr));
    }
    iob->put(rc);
  } else {
    //need to undo the partial adds, before return error.
    for (list<stmt*>::iterator itr = added.begin(); itr != added.end(); itr++) {
      dm->dropStmt(*itr, drv);
    }
    rc = COMMAND_FAILED;
    iob->put(rc);
  }

  if (exp_batch_activate_mode)
    activated++;

}

void querySchdl::DEACTIVATE_QUERY_CMD_CODE_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::DEACTIVATE_QUERY_CMD_CODE_hndl");

  if (querySchdl::verbose) {
    cout << "enter deactivate query: " << id << endl;
  }

  int rc = COMMAND_SUCCESSFUL;
  buffer *iob = bm->lookup("_ioBuffer");
  Driver *drv = NULL;

  s = dm->getStmtByName(id);
  bool err = false;
  list<stmt*> dropped;

  if (s != NULL && (s->type == stmt_coll)) {
    cStmt* s1 = static_cast<cStmt*> (s);
    int sub_count = s1->sub_stmts.size();
    if (querySchdl::verbose) {
      cout << "there are " << sub_count
          << " sub statements in this coll statement." << endl;
    }

    for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
        != s1->sub_stmts.end(); itr++) {
      if (querySchdl::verbose) {
        cout << "current sub statement name is " << (*itr)->name << endl;
        cout << "drop sub stmt from driver: " << (*itr)->name << endl;
      }
      if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
        uStmt* s2 = static_cast<uStmt*> (*itr);
        int sub_count2 = s2->sub_stmts.size();
        if (querySchdl::verbose) {
          cout << "there are " << sub_count2
              << " sub statements in this union statement." << endl;
        }

        for (list<stmt*>::iterator itr2 = s2->sub_stmts.begin(); itr2
            != s2->sub_stmts.end(); itr2++) {
          if (querySchdl::verbose) {
            cout << "current sub statement name is " << (*itr2)->name << endl;
            cout << "drop sub stmt from driver: " << (*itr2)->name << endl;
          }

          if ((dm->dropStmt(*itr2, drv)) == SUCCESS) {
            dropped.push_back(*itr2);
            if (querySchdl::verbose) {
              //if (drv != NULL) drv->printStateTable();
            }
          } else {
            err = true;
            break;
          }
        }
      } else if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
        dropped.push_back(*itr);
        if (querySchdl::verbose) {
          //if (drv != NULL) drv->printStateTable();
        }
      } else {
        err = true;
        break;
      }
    }
  } else if (s != NULL && (s->type == stmt_t_union || s->type == stmt_tl_union)) {
    uStmt* s1 = static_cast<uStmt*> (s);
    int sub_count = s1->sub_stmts.size();
    if (querySchdl::verbose) {
      cout << "there are " << sub_count
          << " sub statements in this union statement." << endl;
    }

    for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
        != s1->sub_stmts.end(); itr++) {
      if (querySchdl::verbose) {
        cout << "current sub statement name is " << (*itr)->name << endl;
        cout << "drop sub stmt from driver: " << (*itr)->name << endl;
      }

      if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
        dropped.push_back(*itr);
        if (querySchdl::verbose) {
          //if (drv != NULL) drv->printStateTable();
        }
      } else {
        err = true;
        break;
      }
    }
  } else if (s != NULL && (s->type == stmt_join)) {
    jStmt* s1 = static_cast<jStmt*> (s);
    int sub_count = s1->sub_stmts.size();
    if (querySchdl::verbose) {
      cout << "there are " << sub_count
          << " sub statements in this join statement." << endl;
    }

    for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
        != s1->sub_stmts.end(); itr++) {
      if (querySchdl::verbose) {
        cout << "current sub statement name is " << (*itr)->name << endl;
        cout << "drop sub stmt from driver: " << (*itr)->name << endl;
      }

      if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
        dropped.push_back(*itr);
        if (querySchdl::verbose) {
          //if (drv != NULL) drv->printStateTable();
        }
      } else {
        err = true;
        break;
      }
    }
  } else if (s != NULL && s->type == stmt_normal) {
  }

  if (s != NULL && querySchdl::verbose) {
    cout << "drop stmt from driver: " << s->name << endl;
  }

  if (s != NULL && !err && (dm->dropStmt(s, drv)) == SUCCESS) {
    if (querySchdl::verbose) {
      cout << "deactivate query success: " << id << endl;
    }

    //free common state
    if (s != NULL && (s->type == stmt_coll)) {
      cStmt* s1 = static_cast<cStmt*> (s);
      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
          uStmt* s2 = static_cast<uStmt*> (*itr);
          for (list<stmt*>::iterator itr2 = s2->sub_stmts.begin(); itr2
              != s2->sub_stmts.end(); itr2++) {
            if ((dm->dropStmt(*itr2, drv)) == SUCCESS) {
              (*itr2)->exe(1);
            }
          }
        } else if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
          (*itr)->exe(1);
        }
      }
    } else if (s != NULL && (s->type == stmt_t_union || s->type
        == stmt_tl_union)) {
      uStmt* s1 = static_cast<uStmt*> (s);
      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
          (*itr)->exe(1);
        }
      }
    } else if (s != NULL && (s->type == stmt_join)) {
      jStmt* s1 = static_cast<jStmt*> (s);
      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        (*itr)->exe(1);
      }

      s->exe(1);
    } else if (s != NULL && s->type == stmt_normal) {
      s->exe(1);
    }

    //put it back into the freepool
    freeStmts.push_back(s);
    iob->put(rc);

  } else {
    //need to undo the partial drops, before return error.
    if (s != NULL) {
      for (list<stmt*>::iterator itr = dropped.begin(); itr != dropped.end(); itr++) {
        dm->addStmt(*itr, drv);
      }
    }

    if (querySchdl::verbose) {
      cout << "deactivate query failure: " << id << endl;
    }

    rc = COMMAND_FAILED;
    iob->put(rc);
  }

  if (s != NULL && querySchdl::verbose) {
    if (drv != NULL)
      drv->printStateTable();
  }
}

void querySchdl::DROP_QUERY_CMD_CODE_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::DROP_QUERY_CMD_CODE_hndl");

  int rc = COMMAND_SUCCESSFUL;
  buffer *iob = bm->lookup("_ioBuffer");
  Driver *drv = NULL;
  bool err = false;

  //first check if I also need to do deactivation too
  s = NULL;
  for (list<stmt*>::iterator itr = freeStmts.begin(); itr != freeStmts.end(); itr++) {
    if (strcmp((*itr)->name, id) == 0) {
      s = *itr;
      break;
    }
  }

  if (s == NULL) {
    //I need to do deactivation
    s = dm->getStmtByName(id);

    list<stmt*> dropped;

    if (s != NULL && (s->type == stmt_coll)) {
      cStmt* s1 = static_cast<cStmt*> (s);
      int sub_count = s1->sub_stmts.size();
      if (querySchdl::verbose) {
        cout << "there are " << sub_count
            << " sub statements in this coll statement." << endl;
      }

      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if (querySchdl::verbose) {
          cout << "current sub statement name is " << (*itr)->name << endl;
          cout << "drop sub stmt from driver: " << (*itr)->name << endl;
        }
        if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
          uStmt* s2 = static_cast<uStmt*> (*itr);
          int sub_count2 = s2->sub_stmts.size();
          if (querySchdl::verbose) {
            cout << "there are " << sub_count2
                << " sub statements in this union statement." << endl;
          }

          for (list<stmt*>::iterator itr2 = s2->sub_stmts.begin(); itr2
              != s2->sub_stmts.end(); itr2++) {
            if (querySchdl::verbose) {
              cout << "current sub statement name is " << (*itr2)->name << endl;
              cout << "drop sub stmt from driver: " << (*itr2)->name << endl;
            }

            if ((dm->dropStmt(*itr2, drv)) == SUCCESS) {
              dropped.push_back(*itr2);
              if (querySchdl::verbose) {
                //if (drv != NULL) drv->printStateTable();
              }
            } else {
              err = true;
              break;
            }
          }
        } else if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
          dropped.push_back(*itr);
          if (querySchdl::verbose) {
            //if (drv != NULL) drv->printStateTable();
          }
        } else {
          err = true;
          break;
        }
      }
    } else if (s != NULL && (s->type == stmt_t_union || s->type
        == stmt_tl_union)) {
      uStmt* s1 = static_cast<uStmt*> (s);
      int sub_count = s1->sub_stmts.size();
      if (querySchdl::verbose) {
        cout << "there are " << sub_count
            << " sub statements in this union statement." << endl;
      }

      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if (querySchdl::verbose) {
          cout << "current sub statement name is " << (*itr)->name << endl;
          cout << "drop sub stmt from driver: " << (*itr)->name << endl;
        }

        if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
          dropped.push_back(*itr);
          if (querySchdl::verbose) {
            //if (drv != NULL) drv->printStateTable();
          }
        } else {
          err = true;
          break;
        }
      }
    } else if (s != NULL && (s->type == stmt_join)) {
      jStmt* s1 = static_cast<jStmt*> (s);
      int sub_count = s1->sub_stmts.size();
      if (querySchdl::verbose) {
        cout << "there are " << sub_count
            << " sub statements in this join statement." << endl;
      }

      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if (querySchdl::verbose) {
          cout << "current sub statement name is " << (*itr)->name << endl;
          cout << "drop sub stmt from driver: " << (*itr)->name << endl;
        }

        if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
          dropped.push_back(*itr);
          if (querySchdl::verbose) {
            //if (drv != NULL) drv->printStateTable();
          }
        } else {
          err = true;
          break;
        }
      }
    } else if (s != NULL && s->type == stmt_normal) {
    }

    if (s != NULL && querySchdl::verbose) {
      cout << "drop stmt from driver: " << s->name << endl;
    }

    if (s != NULL && !err && (dm->dropStmt(s, drv)) == SUCCESS) {
      //free common state
      if (s != NULL && (s->type == stmt_coll)) {
        cStmt* s1 = static_cast<cStmt*> (s);
        for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
            != s1->sub_stmts.end(); itr++) {
          if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
            uStmt* s2 = static_cast<uStmt*> (*itr);
            for (list<stmt*>::iterator itr2 = s2->sub_stmts.begin(); itr2
                != s2->sub_stmts.end(); itr2++) {
              if ((dm->dropStmt(*itr2, drv)) == SUCCESS) {
                (*itr2)->exe(1);
              }
            }
          } else if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
            (*itr)->exe(1);
          }
        }
      } else if (s != NULL && (s->type == stmt_t_union || s->type
          == stmt_tl_union)) {
        uStmt* s1 = static_cast<uStmt*> (s);
        for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
            != s1->sub_stmts.end(); itr++) {
          if ((dm->dropStmt(*itr, drv)) == SUCCESS) {
            (*itr)->exe(1);
          }
        }
      } else if (s != NULL && (s->type == stmt_join)) {
        jStmt* s1 = static_cast<jStmt*> (s);
        for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
            != s1->sub_stmts.end(); itr++) {
          (*itr)->exe(1);
        }

        s->exe(1);
      } else if (s != NULL && s->type == stmt_normal) {
        s->exe(1);
      }

      //put it back into the freepool.
      freeStmts.push_back(s);
    } else {
      //need to undo the partial drops, before return error.
      if (s != NULL) {
        for (list<stmt*>::iterator itr = dropped.begin(); itr != dropped.end(); itr++) {
          dm->addStmt(*itr, drv);
        }
      }

      err = true;
    }

    if (s != NULL && querySchdl::verbose) {
      if (drv != NULL)
        drv->printStateTable();
    }
  }

  //finished handling dropping from drivers
  //delete stdout entry and kill union/window buffers, remove from free pool
  if (s != NULL && !err) {
    char stdoutBuffer[75];
    sprintf(stdoutBuffer, "stdout_%s", id);
    buffer* stdoutBuf = bm->lookup(stdoutBuffer);
    if (stdoutBuf)
      bm->kill(stdoutBuffer);
    freeStmts.remove(s);

    if (s->type == stmt_t_union || s->type == stmt_tl_union) {
      uStmt* s1 = static_cast<uStmt*> (s);
      for (list<buffer*>::iterator itr = s1->union_bufs.begin(); itr
          != s1->union_bufs.end(); itr++) {
        bm->kill((*itr)->name);
      }
    } else if (s->type == stmt_join) {
      jStmt* s1 = static_cast<jStmt*> (s);
      for (list<buffer*>::iterator itr = s1->window_bufs.begin(); itr
          != s1->window_bufs.end(); itr++) {
        bm->kill((*itr)->name);
      }
    }

    iob->put(rc);
  } else {
    rc = COMMAND_FAILED;
    iob->put(rc);
  }

}

void querySchdl::DROP_DECLARE_CMD_CODE_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::DROP_DECLARE_CMD_CODE_hndl");
  buffer *iob = bm->lookup("_ioBuffer");

  int rc = FAILURE;

  //make sure this buffer is not still in use for any active query (can happen now if used by inactive queries)
  //if (dm->bufferInUse(id) || this->bufferInUse(id)) {
  if (dm->bufferInUse(id)) {
    if (querySchdl::verbose) {
      cout << "buffer is still in use in active queries!" << endl;
    }

    rc = FAILURE;
  } else {
    if (bm->kill(id) == 0) {
      //if (querySchdl::verbose) {
      //  cout << "kill returned 0" << endl;
      //}

      rc = SUCCESS;

      //now set any statement using it to be invalid
      for (list<stmt*>::iterator itr = freeStmts.begin(); itr
          != freeStmts.end(); itr++) {
        bool found = false;

        if ((*itr)->type == stmt_coll) {
          cStmt* s1 = static_cast<cStmt*> (*itr);
          int sub_count = s1->sub_stmts.size();

          for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
              != s1->sub_stmts.end(); itr2++) {
            if ((*itr2)->type == stmt_t_union || (*itr2)->type == stmt_tl_union) {
              uStmt* s2 = static_cast<uStmt*> (*itr2);
              int sub_count = s2->sub_stmts.size();
              for (list<stmt*>::iterator itr3 = s2->sub_stmts.begin(); itr3
                  != s2->sub_stmts.end(); itr3++) {
                if (strcmp((*itr3)->inBufName, id) == 0) {
                  found = true;
                }
              }
              if (strcmp((*itr2)->outBufName, id) == 0) {
                found = true;
              }
            } else if (strcmp((*itr2)->inBufName, id) == 0) {
              found = true;
            }
          }

          if (strcmp((*itr)->outBufName, id) == 0) {
            found = true;
          }
        } else if ((*itr)->type == stmt_t_union || (*itr)->type
            == stmt_tl_union) {
          uStmt* s1 = static_cast<uStmt*> (*itr);
          int sub_count = s1->sub_stmts.size();

          for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
              != s1->sub_stmts.end(); itr2++) {
            if (strcmp((*itr2)->inBufName, id) == 0) {
              found = true;
            }
          }

          if (strcmp((*itr)->outBufName, id) == 0) {
            found = true;
          }
        } else if ((*itr)->type == stmt_join) {
          jStmt* s1 = static_cast<jStmt*> (*itr);
          int sub_count = s1->sub_stmts.size();

          for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
              != s1->sub_stmts.end(); itr2++) {
            if (strcmp((*itr2)->inBufName, id) == 0) {
              found = true;
            }
          }

          if (strcmp((*itr)->inBufName, id) == 0 || strcmp((*itr)->outBufName,
              id) == 0) {
            found = true;
          }
        } else if ((*itr)->type == stmt_normal) {
          if (strcmp((*itr)->inBufName, id) == 0 || strcmp((*itr)->outBufName,
              id) == 0) {
            found = true;
          }
          if (querySchdl::verbose) {
            cout << "buffer names in statement " << (*itr)->name << " are: in "
                << (*itr)->inBufName << ", out " << (*itr)->outBufName << endl;
          }
        }

        if (found) {
          (*itr)->valid = false;
        }
      }

    } else {
      //if (querySchdl::verbose) {
      //  cout << "kill returned -1" << endl;
      //}
    }
  }

  iob->put(rc);
  if (querySchdl::verbose) {
    cout << "return code for dropping decl " << id << " is " << rc << endl;
  }

}

void querySchdl::GET_COMPONENTS_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::GET_COMPONENTS_hndl");
  buffer *iob = bm->lookup("_ioBuffer");

  int rc = COMMAND_SUCCESSFUL;
  char drvs[MAX_MSG_LEN];
  drvs[0] = '\0';
  char idstr[10];
  int d_id;
  bool first = true;

  list<Driver*>* ld = dm->getDrivers();
  for (list<Driver*>::iterator itr = ld->begin(); itr != ld->end(); itr++) {
    d_id = (*itr)->getId();
    if (first) {
      sprintf(drvs, "%d %d", d_id, (*itr)->getPriority());
      first = false;
    } else {
      if (strlen(drvs) + strlen(idstr) + 1 > MAX_MSG_LEN) {
        if (querySchdl::verbose) {
          cout << "message overflow for component list. " << endl;
        }
        rc = COMMAND_FAILED;
        break;
      }

      sprintf(idstr, "||%d %d", d_id, (*itr)->getPriority());
      strcat(drvs, idstr);
    }
  }

  if (rc == COMMAND_SUCCESSFUL) {
    if (querySchdl::verbose) {
      cout << "list of drivers are " << drvs << endl;
    }

    iob->put(rc, drvs);
  } else
    iob->put(rc);
}

void querySchdl::VIEW_COMPONENT_DETAILS_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::VIEW_COMPONENT_DETAILS_hndl");
  buffer *iob = bm->lookup("_ioBuffer");

  int rc = COMMAND_SUCCESSFUL;

  int d_id = atoi(id);
  char detail[MAX_MSG_LEN];

  Driver* drv = dm->getDrvById(d_id);

  if (drv == NULL) {
    if (querySchdl::verbose) {
      cout << "driver not found for id: " << id << endl;
    }
    rc = COMMAND_FAILED;
  } else {
    list<b_entry> sources;
    BufStateTblType* bufStateTable = drv->getBufGraph();

    //first get all sources
    for (BufStateTblType::iterator itr = bufStateTable->begin(); itr
        != bufStateTable->end(); itr++) {
      if ((*itr).second->b_type == t_source || (*itr).second->b_type
          == t_source_fork) {
        sources.push_back((*itr).second);
      }
    }

    list<stmt*>* stmts = drv->getStmts();
    if (stmts -> empty()) {
      detail[0] = '\0';
    } else {
      //now, DFS starting on each source, and stop after encountering a statement that has been output.
      //(including output the first such statement, such that on client side it is clear how trees fit together
      //skip internals of union statements and fork dummies.
      bool first = true;
      for (list<b_entry>::iterator itr = sources.begin(); itr != sources.end()
          && rc != COMMAND_FAILED; itr++) {
        list<stack_item*> entry_stack;
        int level = 1;

        //now push self onto stack, and process stack
        stack_item* se = new stack_item();
        se->level = 1;
        se->entry = (*itr);
        se->statement = NULL;
        entry_stack.push_front(se);

        while (!entry_stack.empty() && rc != COMMAND_FAILED) {
          stack_item* pse = entry_stack.front();
          entry_stack.pop_front();
          //put children onto stack
          if (pse->entry->b_type == t_sink || pse->entry->b_type
              == t_merge_sink) {
            //no-op
          } else if (pse->entry->b_type == t_fork || pse->entry->b_type
              == t_source_fork || pse->entry->b_type == t_merge_fork) {
            //my next are fork dummies
            int nFork = pse->entry->forward.fork_count;
            //now find out the dummy states, the # should agree with nFork
            int cnt = 0;
            for (BufStateTblType::iterator itr2 = bufStateTable->begin(); itr2
                != bufStateTable->end(); itr2++) {
              if (itr2->second->b_type != t_fork_dummy)
                continue;
              else if (strcmp(itr2->second->inplace, pse->entry->buf->name)
                  == 0) {
                //as a fork dummy, my next is always unique
                b_entry next =
                    (*bufStateTable)[itr2->second->forward.forward_buf];
                se = new stack_item();
                se->level = (pse->level) + 1;
                se->statement = itr2->second->statement;
                se->entry = next;
                entry_stack.push_front(se);

                cnt++;
              }
            }
            if (nFork != cnt) {
              if (querySchdl::verbose)
                cout
                    << "internal error, number of dummy states does not match the fork count for buffer: "
                    << pse->entry->buf << endl;
              rc = COMMAND_FAILED;
              break;
            }
          } else {

            //there is only unique next, check its type
            b_entry next = (*bufStateTable)[pse->entry->forward.forward_buf];

            if (next->b_type != t_sink && next->b_type != t_merge_sink
                && (next->statement->type == stmt_t_union
                    || next->statement->type == stmt_tl_union
                    || (next->statement->type == stmt_join && next->buf->type
                        == WINBUF))) {
              //union internal buffer and window buffers can only have unique next.
              //skip internal ones
              b_entry nn = (*bufStateTable)[next->forward.forward_buf];
              se = new stack_item();
              se->level = (pse->level) + 1;
              se->statement = next->statement;
              se->entry = nn;
              entry_stack.push_front(se);
            } else {
              //next is a normal buffer, add it.
              se = new stack_item();
              se->level = (pse->level) + 1;
              se->statement = pse->entry->statement;
              se->entry = next;
              entry_stack.push_front(se);
            }
          }

          //finished children above, now add myself to return message
          if (first) {
            sprintf(detail, "| %s#", pse->entry->buf->name);
            first = false;
          } else {
            for (int k = 0; k < pse->level; k++) {
              strcat(detail, "|");
            }
            if (pse->statement != NULL) {
              strcat(detail, " ");
              strcat(detail, pse->statement->name);
            }
            strcat(detail, " ");
            strcat(detail, pse->entry->buf->name);
            strcat(detail, "#");
          }

          delete pse;

        } //end while
      } //end for
    }
  }

  if (rc == COMMAND_SUCCESSFUL) {
    if (querySchdl::verbose) {
      cout << "component details: " << detail << endl;
    }

    iob->put(rc, detail);
  } else
    iob->put(rc);

}

void querySchdl::SET_COMPONENT_PRIORITY_hndl(bufferMngr* bm, char * id,
    char * id2) {
  SMLog::SMLOG(10, "Entering querySchdl::SET_COMPONENT_PRIORITY_hndl");

  buffer *iob = bm->lookup("_ioBuffer");
  int rc = COMMAND_SUCCESSFUL;
  int d_id = atoi(id);
  int priority = atoi(id2);
  Driver* drv = dm->getDrvById(d_id);

  if (drv == NULL) {
    if (querySchdl::verbose) {
      cout << "driver not found for id: " << id << endl;
    }
    rc = COMMAND_FAILED;
  } else {
    drv->setPriority(priority);
  }

  iob->put(rc);

}
void querySchdl::MOVE_STMT_TO_COMPONENT_hndl(bufferMngr* bm, char * id,
    char * id2, char* id3) {
  SMLog::SMLOG(10, "Entering querySchdl::MOVE_STMT_TO_COMPONENT_hndl");
  buffer *iob = bm->lookup("_ioBuffer");

  int rc = COMMAND_SUCCESSFUL;

  //id is stmtid, id2 is From, id3 is To
  stmt* s = dm->getStmtByName(id);
  int idFrom = atoi(id2);
  int idTo = atoi(id3);
  Driver* drv1 = dm->getDrvById(idFrom);
  Driver* drv2 = dm->getDrvById(idTo);

  if (s == NULL || drv1 == NULL || drv2 == NULL) {
    if (querySchdl::verbose) {
      cout << "stmt or driver not found in move component: " << id << ", "
          << id2 << ", " << id3 << endl;
    }

    rc = COMMAND_FAILED;
  } else {
    Driver *drv4 = NULL;
    if (dm->dropStmt(s, drv4) != SUCCESS) {
      if (querySchdl::verbose) {
        cout << "unable to drop stmt " << s->name << endl;
      }

      rc = COMMAND_FAILED;
    } else if (dm->addStmt(s, drv2) != SUCCESS) {
      if (querySchdl::verbose) {
        cout << "unable to add stmt " << s->name << " to driver "
            << drv2->getId() << endl;
      }

      rc = COMMAND_FAILED;
    }
  }

  if (rc == COMMAND_SUCCESSFUL) {
    if (querySchdl::verbose) {
      cout << "move stmt to component resulting driver tables: " << endl;
      //drv1->printStateTable();
      drv2->printStateTable();
    }
  }

  if (querySchdl::verbose) {
    cout << "move stmt to component return code is " << rc << endl;
  }

  iob->put(rc);

}

void querySchdl::BREAK_COMPONENT_hndl(bufferMngr* bm, char * id, char * id2) {
  SMLog::SMLOG(10, "Entering querySchdl::BREAK_COMPONENT_hndl");
  if (querySchdl::verbose) {
    cout << "break component, buffer to be broke on " << id2 << endl;
  }
  buffer *iob = bm->lookup("_ioBuffer");

  int rc = COMMAND_SUCCESSFUL;

  //id is driver id, id2 is buf list
  int d_id = atoi(id);

  char delims[] = "||";
  char *buf = NULL;
  list<buffer*> bufs;
  buf = strtok(id2, delims);
  while (buf != NULL) {
    buffer* b = dm->getBufByName(buf);
    if (b == NULL) {
      if (querySchdl::verbose) {
        cout << "can not find buffer " << buf << endl;
      }
      rc = COMMAND_FAILED;
      break;
    }

    bufs.push_back(b);
    buf = strtok(NULL, delims);
  }
  if (dm->setBreakPnt(&bufs) != SUCCESS) {
    if (querySchdl::verbose) {
      cout << "unable to set breakpoints for bufs " << id2 << endl;
    }
    rc = COMMAND_FAILED;
  }

  if (rc == COMMAND_SUCCESSFUL) {
    if (querySchdl::verbose) {
      cout << "break component resulting driver tables: " << endl;
      list<Driver*>* drvs = dm->getDrivers();
      for (list<Driver*>::iterator itr = drvs->begin(); itr != drvs->end(); itr++) {
        cout << "table: " << endl;
        (*itr)->printStateTable();
      }
    }
  }
  if (querySchdl::verbose) {
    cout << "break component return code is " << rc << endl;
  }

  iob->put(rc);

}

void querySchdl::JOIN_COMPONENTS_hndl(bufferMngr* bm, char * id, char * id2) {
  SMLog::SMLOG(10, "Entering querySchdl::JOIN_COMPONENTS_hndl");
  buffer *iob = bm->lookup("_ioBuffer");

  int rc = COMMAND_SUCCESSFUL;

  //id and id2 are the two drivers
  int idFrom = atoi(id);
  int idTo = atoi(id2);
  Driver* drv1 = dm->getDrvById(idFrom);
  Driver* drv2 = dm->getDrvById(idTo);

  if (drv1 == NULL || drv2 == NULL) {
    if (querySchdl::verbose) {
      cout << "driver not found in join components: " << id << ", " << id2
          << endl;
    }

    rc = COMMAND_FAILED;
  } else {
    list<stmt*> remove;
    list<stmt*>* stmts = drv1->getStmts();
    for (list<stmt*>::iterator itr = stmts->begin(); itr != stmts->end(); itr++) {
      remove.push_back((*itr));
    }

    if (!remove.empty()) {
      for (list<stmt*>::iterator itr = remove.begin(); itr != remove.end()
          && rc != COMMAND_FAILED; itr++) {
        Driver *drv4 = NULL;
        if (dm->dropStmt((*itr), drv4) != SUCCESS) {
          if (querySchdl::verbose)
            cout << "error dropping statement " << (*itr)->name << endl;
          rc = COMMAND_FAILED;
        }
      }

      for (list<stmt*>::iterator itr = remove.begin(); itr != remove.end()
          && rc != COMMAND_FAILED; itr++) {
        if (dm->addStmt((*itr), drv2) != SUCCESS) {
          if (querySchdl::verbose)
            cout << "error adding statement " << (*itr)->name << " to driver "
                << drv2->getId() << endl;
          rc = COMMAND_FAILED;
        }
      }
    }
  }

  if (rc == COMMAND_SUCCESSFUL) {
    if (querySchdl::verbose) {
      cout << "join component resulting driver table: " << endl;
      drv2->printStateTable();
    }
  }

  if (querySchdl::verbose) {
    cout << "join component return code is " << rc << endl;
  }
  iob->put(rc);
}

void querySchdl::MONITOR_PERFORMANCE_BUFFER_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::MONITOR_PERFORMANCE_BUFFER_hndl");
  Monitor* monitor = dm->getMonitor();
  //if (strlen(id2) == 0) {
  monitor->addSendStat(id);
  //} else {
  //monitor->addSendStat(id, id2);
  //}

  buffer *iob = bm->lookup("_ioBuffer");
  int rc = COMMAND_SUCCESSFUL;
  iob->put(rc);
}

void querySchdl::UNMONITOR_PERFORMANCE_BUFFER_hndl(bufferMngr* bm, char * id) {
  SMLog::SMLOG(10, "Entering querySchdl::UNMONITOR_PERFORMANCE_BUFFER_hndl");
  Monitor* monitor = dm->getMonitor();

  //if (strlen(id2) == 0) {
  monitor->removeSendStat(id);
  //} else {
  //monitor->removeSendStat(id, id2);
  //}

  buffer *iob = bm->lookup("_ioBuffer");
  int rc = COMMAND_SUCCESSFUL;
  iob->put(rc);

}

int querySchdl::compile(char* id) {
  SMLog::SMLOG(10, "Entering querySchdl::compile -> id: %s", id);

  void *handle;
  compiler *cc = compiler::getInstance();
  bool err = false;

  cc->redirectStdout("./errorsFile", "w", stdoutLog);
  cc->redirectStderr("./errorsFile", "a", stderrLog);
  int c_rc = cc->compile(id);
  stdoutLog = freopen(qsLog, "a", cc->getTempStdout());
  stderrLog = freopen(qsLog, "a", cc->getTempStderr());
  cc->resetStdouterr();

  int rc = COMMAND_SUCCESSFUL;
  if (c_rc == 0) {
    void *handle;

    char libName[strlen(id) + 11];
    strcpy(libName, "../exe/");
    strcat(libName, id);
    strcat(libName, ".so");

    if (!(handle = dlopen(libName, RTLD_NOW))) {
      fprintf(stderr, "dlopen: %s\n", dlerror());
      err = true;
    } else if (s->type == stmt_coll) {
      cStmt* s1 = static_cast<cStmt*> (s);
      int sub_count = s1->sub_stmts.size();
      if (querySchdl::verbose) {
        cout << "there are " << sub_count
            << " sub statements in this coll statement." << endl;
      }

      SMLog::SMLOG(12, "there are %i sub statements in this coll statement.",
          sub_count);

      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if (querySchdl::verbose) {
          cout << "current sub statement name is " << (*itr)->name << endl;
        }
        if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
          uStmt* s2 = static_cast<uStmt*> (*itr);
          int sub2_count = s2->sub_stmts.size();
          if (querySchdl::verbose) {
            cout << "there are " << sub2_count
                << " sub statements in this union statement." << endl;
          }
          for (list<stmt*>::iterator itr2 = s2->sub_stmts.begin(); itr2
              != s2->sub_stmts.end(); itr2++) {
            if (querySchdl::verbose) {
              cout << "current sub statement name is " << (*itr2)->name << endl;
            }
            if (!(((*itr2)->func)
                = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
                    void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle,
                    (*itr2)->name))) {
              fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
              err = true;
              break;
            }
          }
        } else if (!(((*itr)->func)
            = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*, void*,
                std::hash<const char*> , eqstrTab>*)) dlsym(handle, (*itr)->name))) {
          fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
          err = true;
          break;
        }
      }
    } else if (s->type == stmt_t_union || s->type == stmt_tl_union) {
      uStmt* s1 = static_cast<uStmt*> (s);
      int sub_count = s1->sub_stmts.size();
      if (querySchdl::verbose) {
        cout << "there are " << sub_count
            << " sub statements in this union statement." << endl;
      }

      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if (querySchdl::verbose) {
          cout << "current sub statement name is " << (*itr)->name << endl;
        }
        if (!(((*itr)->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<
            const char*, void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle,
            (*itr)->name))) {
          fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
          err = true;
          break;
        }
      }
    } else if (s->type == stmt_join) {
      jStmt* s1 = static_cast<jStmt*> (s);
      int sub_count = s1->sub_stmts.size();
      if (querySchdl::verbose) {
        cout << "there are " << sub_count
            << " sub statements in this join statement." << endl;
      }

      for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
          != s1->sub_stmts.end(); itr++) {
        if (querySchdl::verbose) {
          cout << "current sub statement name is " << (*itr)->name << endl;
        }
        if (!(((*itr)->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<
            const char*, void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle,
            (*itr)->name))) {
          fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
          err = true;
          break;
        }
      }

      //different from union, the join statement itself also has compiled function. (Union does not)
      if (!((s->func)
          = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*, void*,
              std::hash<const char*> , eqstrTab>*)) dlsym(handle, id))) {
        fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
        err = true;
      }

    } else if (s->type == stmt_normal) {
      if (!((s->func)
          = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*, void*,
              std::hash<const char*> , eqstrTab>*)) dlsym(handle, id))) {
        fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
        err = true;
      }
    }
  }

  if (c_rc == 0 && !err) {
  } else {
    rc = COMMAND_FAILED;
  }

  return rc;
}

bool querySchdl::bufferInUse(char* bname) {
  SMLog::SMLOG(10, "Entering querySchdl::bufferInUse");
  for (list<stmt*>::iterator itr = freeStmts.begin(); itr != freeStmts.end(); itr++) {
    if ((*itr)->type == stmt_coll) {
      cStmt* s1 = static_cast<cStmt*> (*itr);
      int sub_count = s1->sub_stmts.size();

      for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
          != s1->sub_stmts.end(); itr2++) {
        if ((*itr2)->type == stmt_t_union || (*itr2)->type == stmt_tl_union) {
          uStmt* s2 = static_cast<uStmt*> (*itr2);
          int sub_count2 = s2->sub_stmts.size();

          for (list<stmt*>::iterator itr3 = s2->sub_stmts.begin(); itr3
              != s2->sub_stmts.end(); itr3++) {
            if (strcmp((*itr3)->in->name, bname) == 0) {
              return true;
            }
          }

          if (strcmp((*itr2)->out->name, bname) == 0) {
            return true;
          }
        } else if (strcmp((*itr2)->in->name, bname) == 0) {
          return true;
        }
      }

      if (strcmp((*itr)->out->name, bname) == 0) {
        return true;
      }
    } else if ((*itr)->type == stmt_t_union || (*itr)->type == stmt_tl_union) {
      uStmt* s1 = static_cast<uStmt*> (*itr);
      int sub_count = s1->sub_stmts.size();

      for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
          != s1->sub_stmts.end(); itr2++) {
        if (strcmp((*itr2)->in->name, bname) == 0) {
          return true;
        }
      }

      if (strcmp((*itr)->out->name, bname) == 0) {
        return true;
      }
    } else if ((*itr)->type == stmt_join) {
      jStmt* s1 = static_cast<jStmt*> (*itr);
      int sub_count = s1->sub_stmts.size();

      for (list<stmt*>::iterator itr2 = s1->sub_stmts.begin(); itr2
          != s1->sub_stmts.end(); itr2++) {
        if (strcmp((*itr2)->in->name, bname) == 0) {
          return true;
        }
      }

      if (strcmp((*itr)->in->name, bname) == 0 || strcmp((*itr)->out->name,
          bname) == 0) {
        return true;
      }
    } else if ((*itr)->type == stmt_normal) {
      if (strcmp((*itr)->in->name, bname) == 0 || strcmp((*itr)->out->name,
          bname) == 0) {
        return true;
      }
    }
  }
  return false;
}

bool querySchdl::stmtInUse(char* name) {
  for (list<stmt*>::iterator itr = freeStmts.begin(); itr != freeStmts.end(); itr++) {
    if (strcmp((*itr)->name, name) == 0)
      return true;
  }
  return false;
}

int querySchdl::simpleTest() {
  void *handle;
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;

  cc->compile("stream");

  // add stmt
  if (!(handle = dlopen("../exe/stream.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }

  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->setPriority(5);
  drv->printStateTable();

  cout << "run driver" << endl;
  if (dm->run(drv) == run_failure) {
    cout << "ERROR running driver" << endl;
  }

  //for (int i = 0; i < 3; i++){
  //      if(!bm->emtpy("_queryBuffer")){  // new query module
  // compile query module
  //cout<<s->func(bufferMngr::getInstance())<<endl;
  //sleep(2);
  //}  // end while 1
}

int querySchdl::forkTest() {
  void *handle;
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;

  cc->compile("stream");

  // add stmt
  if (!(handle = dlopen("../exe/stream.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }

  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->setPriority(5);
  drv->printStateTable();

  cc->compile("stream2");
  // add stmt
  if (!(handle = dlopen("../exe/stream2.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream2"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  cc->compile("stream3");
  // add stmt
  if (!(handle = dlopen("../exe/stream3.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream3"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  cc->compile("stream4");
  // add stmt
  if (!(handle = dlopen("../exe/stream4.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream4"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  /*
   cc->compile("stream5");
   // add stmt
   if (!(handle = dlopen ("../exe/stream5.so", RTLD_NOW))) {
   fprintf(stderr, "dlopen: %s\n", dlerror());
   return -1;
   }
   if (!((void*)(s->func) = dlsym(handle, "stream5"))) {
   fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
   exit(1);
   }
   cout << "add stmt to driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;
   dm->addStmt(s, drv);
   drv->printStateTable();

   //test remove statement
   cc->compile("stream3");
   // add stmt
   if (!(handle = dlopen ("../exe/stream3.so", RTLD_NOW))) {
   fprintf(stderr, "dlopen: %s\n", dlerror());
   return -1;
   }
   if (!((void*)(s->func) = dlsym(handle, "stream3"))) {
   fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
   exit(1);
   }
   cout << "remove stmt from driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;

   dm->dropStmt(s);
   drv->printStateTable();
   */

  cout << "run driver" << endl;
  if (dm->run(drv) == run_failure) {
    cout << "ERROR running driver" << endl;
  }
  return 0;
}

int querySchdl::tUnionTest() {
  void *handle;
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;

  //test timed union
  char* sname = "test1";
  cc->compile(sname);
  drv = NULL;

  // add stmt
  if (!(handle = dlopen("../exe/test1.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (s->type == stmt_t_union || s->type == stmt_tl_union) {
    uStmt* s1 = static_cast<uStmt*> (s);
    int sub_count = s1->sub_stmts.size();
    cout << "there are " << sub_count
        << " sub statements in this union statement." << endl;
    for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
        != s1->sub_stmts.end(); itr++) {
      cout << "current sub statement name is " << (*itr)->name << endl;
      if (!(((*itr)->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<
          const char*, void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle,
          (*itr)->name))) {
        fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
        exit(1);
      }
      cout << "add sub stmt to driver: " << (*itr)->name << endl;
      dm->addStmt(*itr, drv);
      drv->printStateTable();
    }
  } else if (s->type == stmt_normal) {
    if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
        void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, sname))) {
      fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
      exit(1);
    }
  }

  cout << "add stmt to driver: " << s->name << ", of type " << s->type
      << ", with out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->setPriority(5);
  drv->printStateTable();

  cout << "run driver" << endl;
  if (dm->run(drv) == run_failure) {
    cout << "ERROR running driver" << endl;
  }
  return 0;
}

int querySchdl::tlUnionTest() {
  void *handle;
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;

  //test timeless union
  char* sname = "test";
  cc->compile(sname);
  drv = NULL;

  // add stmt
  if (!(handle = dlopen("../exe/test.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (s->type == stmt_t_union || s->type == stmt_tl_union) {
    uStmt* s1 = static_cast<uStmt*> (s);
    int sub_count = s1->sub_stmts.size();
    cout << "there are " << sub_count
        << " sub statements in this union statement." << endl;
    for (list<stmt*>::iterator itr = s1->sub_stmts.begin(); itr
        != s1->sub_stmts.end(); itr++) {
      cout << "current sub statement name is " << (*itr)->name << endl;
      if (!(((*itr)->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<
          const char*, void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle,
          (*itr)->name))) {
        fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
        exit(1);
      }
      cout << "add sub stmt to driver: " << (*itr)->name << endl;
      dm->addStmt(*itr, drv);
      drv->printStateTable();
    }
  } else if (s->type == stmt_normal) {
    if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
        void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "test"))) {
      fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
      exit(1);
    }
  }

  cout << "add stmt to driver: " << s->name << ", of type " << s->type
      << ", with out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->setPriority(5);
  drv->printStateTable();

  cout << "run driver" << endl;
  if (dm->run(drv) == run_failure) {
    cout << "ERROR running driver" << endl;
  }

  return 0;
}

int querySchdl::splitTest() {
  void *handle;
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;

  cc->compile("stream");

  // add stmt
  if (!(handle = dlopen("../exe/stream.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }

  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->setPriority(5);
  drv->printStateTable();

  cc->compile("stream2");
  // add stmt
  if (!(handle = dlopen("../exe/stream2.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream2"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  cc->compile("stream3");
  // add stmt
  if (!(handle = dlopen("../exe/stream3.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream3"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  cc->compile("stream4");
  // add stmt
  if (!(handle = dlopen("../exe/stream4.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream4"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  cc->compile("stream5");
  // add stmt
  if (!(handle = dlopen("../exe/stream5.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream5"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  //test breakpoint
  cc->compile("stream3");
  // add stmt
  if (!(handle = dlopen("../exe/stream3.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream3"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }

  list<buffer*> bufs;
  bufs.push_back(s->out);
  dm->setBreakPnt(&bufs);

  //cout << "run driver" << endl;
  //if (dm->run(drv) == run_failure) {
  //  cout << "ERROR running driver" << endl;
  // }
  return 0;
}

int querySchdl::compTest() {
  void *handle;
  compiler *cc = compiler::getInstance();
  Driver *drv = NULL;

  cc->compile("stream");

  // add stmt
  if (!(handle = dlopen("../exe/stream.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }

  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->setPriority(5);
  drv->printStateTable();

  cc->compile("stream5");
  // add stmt
  if (!(handle = dlopen("../exe/stream5.so", RTLD_NOW))) {
    fprintf(stderr, "dlopen: %s\n", dlerror());
    return -1;
  }
  if (!((s->func) = (int(*)(bufferMngr*, int, buffer*, hash_map<const char*,
      void*, std::hash<const char*> , eqstrTab>*)) dlsym(handle, "stream5"))) {
    fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
    exit(1);
  }
  cout << "add stmt to driver: " << s->name << ", with in buffer: "
      << s->in->name << ", out buffer " << s->out->name << endl;
  dm->addStmt(s, drv);
  drv->printStateTable();

  bufferMngr *bm = bufferMngr::getInstance();
  buffer* qBuf = bm->lookup("_queryBuffer");
  cDBT cdbt(MAX_MSG_LEN);

  //test merging drivers
  int code = JOIN_COMPONENTS;
  ;

  memcpy(cdbt.data, &code, sizeof(int));
  strcpy((char*) cdbt.data + sizeof(int), "1");
  strcpy((char*) cdbt.data + sizeof(int) + 2, "2");
  strcpy((char*) cdbt.data + sizeof(int) + 4, "");

  qBuf->put(&cdbt);

  //test breakpoint
  code = BREAK_COMPONENT;
  s = dm->getStmtByName("stream5");
  cout << "stmt out buffer to be used as break point: " << s->out->name << endl;
  //cout << "driver id is " << drv->getId() << endl;

  memcpy(cdbt.data, &code, sizeof(int));
  strcpy((char*) cdbt.data + sizeof(int), "2");
  strcpy((char*) cdbt.data + sizeof(int) + 2, s->out->name);
  strcpy((char*) cdbt.data + sizeof(int) + 2 + strlen(s->out->name) + 1, "");

  qBuf->put(&cdbt);

  cout << "exit compTest" << endl;

  return 0;
}

/*
 int compTest_old()
 {
 void *handle;
 compiler *cc = compiler::getInstance();
 Driver *drv=NULL;


 cc->compile("stream");

 // add stmt
 if (!(handle = dlopen ("../exe/stream.so", RTLD_NOW))) {
 fprintf(stderr, "dlopen: %s\n", dlerror());
 return -1;
 }
 if (!((void*)(s->func) = dlsym(handle, "stream"))) {
 fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
 exit(1);
 }

 cout << "add stmt to driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;
 dm->addStmt(s, drv);
 drv->setPriority(5);
 drv->printStateTable();

 cc->compile("stream2");
 // add stmt
 if (!(handle = dlopen ("../exe/stream2.so", RTLD_NOW))) {
 fprintf(stderr, "dlopen: %s\n", dlerror());
 return -1;
 }
 if (!((void*)(s->func) = dlsym(handle, "stream2"))) {
 fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
 exit(1);
 }
 cout << "add stmt to driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;
 dm->addStmt(s, drv);
 drv->printStateTable();

 cc->compile("stream3");
 // add stmt
 if (!(handle = dlopen ("../exe/stream3.so", RTLD_NOW))) {
 fprintf(stderr, "dlopen: %s\n", dlerror());
 return -1;
 }
 if (!((void*)(s->func) = dlsym(handle, "stream3"))) {
 fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
 exit(1);
 }
 cout << "add stmt to driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;
 dm->addStmt(s, drv);
 drv->printStateTable();

 cc->compile("stream4");
 // add stmt
 if (!(handle = dlopen ("../exe/stream4.so", RTLD_NOW))) {
 fprintf(stderr, "dlopen: %s\n", dlerror());
 return -1;
 }
 if (!((void*)(s->func) = dlsym(handle, "stream4"))) {
 fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
 exit(1);
 }
 cout << "add stmt to driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;
 dm->addStmt(s, drv);
 drv->printStateTable();

 cc->compile("stream5");
 // add stmt
 if (!(handle = dlopen ("../exe/stream5.so", RTLD_NOW))) {
 fprintf(stderr, "dlopen: %s\n", dlerror());
 return -1;
 }
 if (!((void*)(s->func) = dlsym(handle, "stream5"))) {
 fprintf(stderr, "dlsym: %s %d\n", dlerror(), __LINE__);
 exit(1);
 }
 cout << "add stmt to driver: " << s->name << ", with in buffer: " << s->in->name << ", out buffer " << s->out->name << endl;
 dm->addStmt(s, drv);
 drv->printStateTable();

 bufferMngr *bm = bufferMngr::getInstance();
 buffer* qBuf = bm->lookup("_queryBuffer");
 cDBT cdbt(MAX_MSG_LEN);

 //test list drivers
 int code = GET_COMPONENTS;
 memcpy(cdbt.data, &code, sizeof(int));
 strcpy((char*)cdbt.data+sizeof(int), "");
 strcpy((char*)cdbt.data+sizeof(int)+1, "");
 strcpy((char*)cdbt.data+sizeof(int)+1+1, "");
 qBuf->put(&cdbt);

 //test list driver detail
 code = VIEW_COMPONENT_DETAILS;
 char d_id[10];
 sprintf(d_id, "%d", drv->getId());

 memcpy(cdbt.data, &code, sizeof(int));
 strcpy((char*)cdbt.data+sizeof(int), d_id);
 strcpy((char*)cdbt.data+sizeof(int)+strlen(d_id)+1, "");
 strcpy((char*)cdbt.data+sizeof(int)+strlen(d_id)+1+1, "");
 qBuf->put(&cdbt);


 //test breakpoint
 code = BREAK_COMPONENT;
 s = dm->getStmtByName("stream3");
 cout << "stmt out buffer to be used as break point: " << s->out->name << endl;
 cout << "driver id is " << drv->getId() << endl;

 memcpy(cdbt.data, &code, sizeof(int));
 strcpy((char*)cdbt.data+sizeof(int), d_id);
 strcpy((char*)cdbt.data+sizeof(int)+strlen(d_id)+1, s->out->name);
 strcpy((char*)cdbt.data+sizeof(int)+strlen(d_id)+1+strlen(s->out->name)+1, "");

 qBuf->put(&cdbt);

 //test move statement
 code = MOVE_STMT_TO_COMPONENT;
 s = dm->getStmtByName("stream5");
 cout << "move stmt: " << s->name << endl;

 memcpy(cdbt.data, &code, sizeof(int));
 strcpy((char*)cdbt.data+sizeof(int), s->name);
 strcpy((char*)cdbt.data+sizeof(int)+strlen(s->name)+1, "1");
 strcpy((char*)cdbt.data+sizeof(int)+strlen(s->name)+1+2, "2");

 qBuf->put(&cdbt);

 //test merging drivers
 code = JOIN_COMPONENTS;;

 memcpy(cdbt.data, &code, sizeof(int));
 strcpy((char*)cdbt.data+sizeof(int), "1");
 strcpy((char*)cdbt.data+sizeof(int)+2, "2");
 strcpy((char*)cdbt.data+sizeof(int)+4, "");

 qBuf->put(&cdbt);

 cout << "exit compTest" << endl;

 return 0;
 }
 */

}
