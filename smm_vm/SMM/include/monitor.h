#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "buffer.h"
#include "stmt.h"
#include "driver.h"
#include <iostream>
#include <string>
#include <ext/hash_map>
#include <list>
#include <queue>
#include <set>

//number of tuples to monitor for a statement, before turn off the sensor (then wake up periodically)
#define NUM_TUPLES_TO_MONITOR_BEFORE_REST 10000

//check wakeup flags every this many seconds
#define WAKEUP_CYCLE_SEC 60

//measure total buffer usage once every MEM_MEASURE_INTERVAL tuples processed, and at least once every WINDOW_SIZE_SEC seconds if tuples come slowly.
#define MEM_MEASURE_INTERVAL 50
//measure total input buffer size once every STMT_BUF_MEM_MEASURE_INTERVAL tuples processed for that stmt
#define STMT_BUF_MEASURE_INTERVAL 10

using namespace std;
using namespace __gnu_cxx;

namespace ESL {

  struct ltstr {
    bool operator()(const char* s1, const char* s2) const {
      return strcmp(s1, s2) < 0;
    }
  };

  struct perf_stat_info {
    string stmt_name;
    string stat_name;
    string buf_name; //this is really a concatenation of the previous two
    bool isGlobal; //global stats have empty stmt_name
    
    perf_stat_info():stmt_name(""),stat_name(""),buf_name(""),isGlobal(true) {}
  };  

  struct ltsspair {
    bool operator()(const perf_stat_info* s1, const perf_stat_info* s2) const {
      return s1->buf_name < s2->buf_name;
    }
  };
  
  typedef struct sensor_t {
    struct timeval last_updated;
    struct timeval wakeup_time; //this is set whenever on flag is set to false, indicate when to set to true again.
    bool on; //whether currently being monitored
    //bool send; //whether currently being sent to client to view
    //sensor_t(bool on=true, bool send=false):on(on),send(send){last_updated.tv_sec = 0; last_updated.tv_usec = 0; wakeup_time.tv_sec = 0; wakeup_time.tv_usec = 0;}
    sensor_t(bool on=true):on(on){last_updated.tv_sec = 0; last_updated.tv_usec = 0; wakeup_time.tv_sec = 0; wakeup_time.tv_usec = 0;}
    virtual ~sensor_t() {}
  } sensor;

  typedef struct int_sensor_t:public sensor {
    int value;
    int_sensor_t():value(0){}
  } int_sensor;

  typedef struct long_sensor_t:public sensor {
    long value;
    long_sensor_t():value(0){}
  } long_sensor;

  typedef struct double_sensor_t:public sensor {
    double value;
    double_sensor_t():value(0.0){}
  } double_sensor;

  typedef struct time_sensor_t:public sensor {
    struct timeval value;
    time_sensor_t(){value.tv_sec = 0; value.tv_usec = 0;}    
  } time_sensor;

  //stmt_entry keeps stats for every statement
  //to calculate Selectivity and Average Processing Time for one input tuple
  typedef struct stmt_ety {
    //for cumulative stats, used for things like selectivity, average output rate, etc.
    long_sensor total_input;
    long_sensor total_output;
    double_sensor total_p_time;

    //for windowed stats, performance monitoring
    int_sensor win_total_input;
    int_sensor win_total_output;
    double_sensor win_total_p_time; //in seconds

    time_sensor start_time;  //this is to calculate average consumption rate, which is used to estimated how often to measure the stats. always on.
    time_sensor last_in_turn; //this is always on, indicate when this statement was last granted a turn to run (even if there were no input to run it)

    //this is measured in # of tuples
    //total size of all input buffer, cumulative total for current window (used for average calculation.)
    //avg_buf_size_at_any_moment=(float)win_cumul_input_buf_size/((int)(win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1)
    long_sensor win_cumul_input_buf_size; 

    float last_known_selectivity; //cached old value, to be used when starting again from scratch 
    float last_known_process_rate; //cached old value, # input consumed per sec of processing time.
    int input_tuple_size;   //assume tuple size fixed for now
    int output_tuple_size;     //assume tuple size fixed for now
    stmt_ety():last_known_selectivity(0),last_known_process_rate(0),input_tuple_size(0),output_tuple_size(0) {}
  } stmt_entry;
  
  //output entry keeps stats for every stmt that outputs to the system
  //to calculate average latency
  //these are currently all always on
  typedef struct output_ety {
    //for cumulative stats.
    long_sensor total_output;
    double_sensor total_latency; 
    double_sensor max_latency; 

    //for windowed stats, performance monitoring
    int_sensor win_total_output;
    double_sensor win_total_latency; 
    double_sensor win_max_latency; 
    
  } output_entry;
  
  //input entry keeps stats for every stmt that receives input to the system
  //to measure starvation and tuple arrival rate
  //these are currently all always on
  typedef struct input_ety {
    //for cumulative stats.
    long_sensor total_input;

    time_sensor start_time;  //this is to calculate cumulative average arrival rate, always on

    //for windowed stats, performance monitoring
    int_sensor win_total_input;
    double_sensor win_max_tuple_wait_time; //incoming tuples, how long it has to wait before being served (max).
    double_sensor win_total_tuple_wait_time; //incoming tuples, how long it has to wait before being served (total, used for getting average).

    //this is measured in # of tuples
    //avg_buf_size_at_any_moment=(float)win_cumul_input_buf_size/((int)(win_total_input.value-1)/(int)STMT_BUF_MEASURE_INTERVAL+1)
    long_sensor win_cumul_input_buf_size; 
  } input_entry;

  typedef hash_map<char*, stmt_entry*, hash<char*>, eqstr> StmtStatMapType; //keyed by a copy of stmt.name
  typedef hash_map<char*, output_entry*, hash<char*>, eqstr> OutputStatMapType; //keyed by the same copy of stmt.name as above
  typedef hash_map<char*, input_entry*, hash<char*>, eqstr> InputStatMapType; //keyed by a copy of inbuffer.name

  class Monitor {
    friend class DrvMgr;
    
    public:
      static const int WINDOW_SIZE_SEC;

      static double EFFECTIVE_CPU_UTILIZATION;
  
      ~Monitor();
      
      //update is called each time something is added or removed from driver ( driver "dirty")
      void updateMaps();

      //update flags of sensors that are currently set to "off" but due to be turned on
      //this should be called periodically.
      void updateWakeup();

      //force all stmt.exe() to be called through here. such that the performance can be monitored.
      stmt_rc exeStmt(stmt* s, Driver* drv);

      stmt_entry* getStmtStat(stmt* s);

      void updateLastInTurn(stmt* s);

      void printStateTables();

      void addSendStat(char* statName, char* stmt_name = NULL);
      void removeSendStat(char* statName, char* stmt_name = NULL);
      
      struct timeval next_check_wakeup;

      long win_start_sec; //current measuring window starting second, relative to "start_time"
      
      double last_win_total_p_time; //to be used to record the last value of win_total_p_time.value, for utilization approximation while the new window is being calculated
      
      //the following are cumulative
      double_sensor total_scheduling_time;
      double_sensor total_processing_time; //this is for overhead calculation, considers things in Monitor as processing time
      long_sensor total_empty_buffer_hit;

      //indicate a new window is just entered, reset by Monitor after the first call of exeStmt inside the new window
      bool newWindow;

      int_sensor win_total_input; //all source buffers input only
      int_sensor win_total_output; //all sink buffers output only

      bool collectStats;
	
    protected:
      //can only be instatiated by DrvMgr
      Monitor();

    private:
      StmtStatMapType* stmtStats;
      //struct timeval stmts_last_monitored;
      InputStatMapType* inputStats;
      //struct timeval inputs_last_monitored;
      OutputStatMapType* outputStats;
      //struct timeval outputs_last_monitored;

      struct timeval start_time; //start time of the monitor, used as landmark for windowing
      
      double_sensor win_total_latency;
      double_sensor win_max_latency;
      double_sensor win_max_stmt_wait_time; //for stmts containing source buffers only, for now.
      double_sensor win_max_tuple_wait_time; //incoming tuples, how long it has to wait before being served (max).
      double_sensor win_total_tuple_wait_time; //incoming tuples, how long it has to wait before being served (total, used for getting average).      
      double_sensor win_total_p_time; //total processing time for all stmts, in seconds. this only includes actual processing time of statements
      
      //total buffer usage is measured for once every MEM_MEASURE_INTERVAL tuples processed, and at least once every WINDOW_SIZE_SEC seconds if tuples come slowly. 
      //this is measured in bytes.
      long_sensor win_max_total_buf_bytes;
      long win_cumul_total_buf_bytes; //use to calculate average total buf size. No need to be sensor, goes on and off with win_max_total_buf_bytes sensor
      long win_cumul_total_source_buf_counts; //use to calculate avg source buf size in terms of counts.
      int win_buf_size_meas_cnt;
      int win_input_processed_cnt;

      //the list of performance data that needs to be sent to buffers, which will be monitored by clients.
      set<perf_stat_info*, ltsspair> stat_to_send_bufs;
  };
  
} // end of namespace ESL

#endif
