#ifndef __STRATEGY_H__
#define __STRATEGY_H__

#include "buffer.h"
#include "stmt.h"
#include "driver.h"
#include "monitor.h"
#include <iostream>
#include <string>
#include <ext/hash_map>
#include <list>
#include <map>
#include <queue>
#include <algorithm>

//#define NUM_INPUT_BEFORE_SET_STAT 1000
#define NUM_INPUT_BEFORE_SET_STAT 300

//default deadline, used when no deadline is assigned, if the strategy uses it.
#define DEFAULT_DEADLINE 20

//temporarily done this way, assume 50% CPU time is used for running stmts
//#define EFFECTIVE_CPU_UTILIZATION 0.5

//in deadline scheduling machanisms, check to see whether switching over to deadline mode every this many tuples are processed
//if there are not as many tuples (or stmts processing time too long), scheduler checks every Monitor::WINDOW_SIZE_SEC seconds, which usually is 1 sec
#define DEADLINE_CHECK_FREQUENCY_IN_COUNT 1000

using namespace std;
using namespace __gnu_cxx;

namespace ESL{
    
  //---------base constructs begin----------------
  struct StmtEntry {
    StmtEntry(stmt* s);
    
    virtual ~StmtEntry() {
      delete stmt_name;
    }
    
    string* stmt_name;
    stmt* pStmt; //cache pointer for ease of use
    double oriPriority; //priority assigned by algorithms
    double priority; //adjusted priority (to account for time in QUOTA_TIME_MODE)

    //theoritical max queue length this stmt can have, to still be finished within 1 second
    //calculated based on processor sharing fraction and processing time, once stat is available (if running GPS-alike algorithms)
    int expectedQMax;
    int lastWinQSize;  //not frequent enough for latency estimation, not used now.

    double outputProcTime; //total processing time from this stmt till output, assuming all selectivity 1. used for deadline estimation for tuple on the front of queue

    //Latest time the front tuple on this stmt has to be processed, to not miss the dealine. Calculated as described by CHAIN paper.
    //urgencyTime = tq+deadline-outputProcTime, where tq is the timestamp of the front tuple on the input queue
    //At source there can not be union and join stmts, no need to check
    struct timeval urgencyTime;
  };

  class eqSE : public std::unary_function<char*, bool> {
  public:

    eqSE(char* name) {value = name;}

    bool operator()(StmtEntry* se) const {
	return (se->stmt_name->compare(value) == 0);    
    }  

  private:
    char* value;
  };

  struct gtOriSE {
    bool operator()(const StmtEntry* s1, const StmtEntry* s2) const {
      return s1->oriPriority > s2->oriPriority; 
    }
    bool operator()(const StmtEntry* s1, const double s2) const {
      return s1->oriPriority > s2; 
    }
    bool operator()(const double s1, const StmtEntry* s2) const {
      return s1 > s2->oriPriority; 
    }
  };

  struct gtSE {
    bool operator()(const StmtEntry* s1, const StmtEntry* s2) const {
      return s1->priority > s2->priority; 
    }
    bool operator()(const StmtEntry* s1, const double s2) const {
      return s1->priority > s2; 
    }
    bool operator()(const double s1, const StmtEntry* s2) const {
      return s1 > s2->priority; 
    }
  };

  struct gtTimeSE {
    bool operator()(const StmtEntry* s1, const StmtEntry* s2) const {
      return (s1->urgencyTime.tv_sec > s2->urgencyTime.tv_sec) || (s1->urgencyTime.tv_sec == s2->urgencyTime.tv_sec && s1->urgencyTime.tv_usec > s2->urgencyTime.tv_usec); 
    }
    bool operator()(const StmtEntry* s1, const struct timeval s2) const {
      return (s1->urgencyTime.tv_sec > s2.tv_sec) || (s1->urgencyTime.tv_sec == s2.tv_sec && s1->urgencyTime.tv_usec > s2.tv_usec); 
    }
    bool operator()(const struct timeval s1, const StmtEntry* s2) const {
      return (s1.tv_sec > s2->urgencyTime.tv_sec) || (s1.tv_sec == s2->urgencyTime.tv_sec && s1.tv_usec > s2->urgencyTime.tv_usec); 
    }
  };
  
  //group of stmts/segments share the same weight  
  class WeightGroup {
  public:
    WeightGroup(int weight) { 
      this->weight = weight;
    }
    
    virtual ~WeightGroup();
    
    void addStmt(stmt* s) {
      vSEntries.push_back(new StmtEntry(s));
    }

    int getNumStmts() { return vSEntries.size();}

    vector<StmtEntry*>* getEntries() { return &vSEntries; }
    
    //this is a group of stmts/segments share the same weight
    int weight;
    
  protected:
    
    vector<StmtEntry*> vSEntries;
  };
  
  struct gtwg {
    bool operator()(const WeightGroup* w1, const WeightGroup* w2) const {
      return w1->weight > w2->weight; 
    }
    bool operator()(const WeightGroup* w1, const int w2) const {
      return w1->weight > w2; 
    }
    bool operator()(const int w1, const WeightGroup* w2) const {
      return w1 > w2->weight; 
    }
  };

  //the two modes determines how strides are incremented, by time elapsed for the unit, or # of tuple processed.
  enum quota_mode {
    QUOTA_TIME_MODE,
    QUOTA_COUNT_MODE
  };
//!!!!!!!!! to do: time quota is not handled !!!!!!!!!!!!!!!!//
  
  class WeightProfile {
  public:
    static const int DEFAULT_VALUE = 1000000000;
    static const int WEIGHT_MIN = 1;
    static int WEIGHT_MAX;  //changable
    
    WeightProfile(Driver* drv, int roundTotal = -1, quota_mode mode = QUOTA_COUNT_MODE);
    virtual ~WeightProfile();
    
    //void setDriver(Driver* drv) {
    //  this->drv = drv; update();
    //}

    BufStateTblType* getQueryGraph() { return &(this->queryGraph); }
	
    //update is called whenever driver is changed, to add or remove entries
    //assign prioroty to added statements according to performance data 
    //when no data available everyone has the same default weight WeightProfile::WEIGHT_MIN
    //the base class method builds the query graph, calculate all weights, and put source stmts into their corresponding weight groups
    //it will be the children class responsibility to find out all priorities and sort the stmts on priority    
    virtual void update();
    
    //check whether the stat numbers have changed significantly, such that a priority update is needed. diff for diff algorithms
    virtual bool change_significant(stmt_entry* monitor_s_entry) {return false;}
    
    
    //by default every statement can trigger priority/quota update. 
    //specific algorithm may change this behavior, such as in OCS only sinks will trigger
    virtual bool isUpdateTrigger(stmt* s) {return true;} 

    virtual void printStateTable();

    virtual bool empty() {return vSGroups.empty();}

    quota_mode getMode() {return this->mode;}

    const vector<WeightGroup*>* getWeightGroups() { return &vSGroups; }

    void setStmtWithDeadline() {stmtWithDeadline = true;}
    void resetStmtWithDeadline() {stmtWithDeadline = false;}    
    bool stmtHasDeadline() {return stmtWithDeadline;}
    
  protected:
    
    //calculate individual priority for the statement, if not already set.
    //create entry in info table, if entry not already there
    //set priority on stmt object 
    //if priority set, do nothing
    virtual StmtEntry* update_priority(stmt* s) = 0;    
    virtual void update_priority(StmtEntry* se); //only wrap around update_priority(stmt*) to further modify the StmtEntry

    //return the calculated weight for stmt s, based on the query graph in the driver.
    virtual int calc_weight(stmt* s, BufStateTblType* drvMap);

    //this method update actual quotas for the statements, usually based on calculated priorities.
    //performs scaling here, if required.
    //usually used as the last step in the update() method
    virtual void update_quotas() = 0;
    
    Driver* drv;
    
    vector<WeightGroup*> vSGroups; //constaining sources only, grouped by their weights. Remain sorted all time.
    
    BufStateTblType queryGraph;

    //to scale the quotas. Indicate how many total tuples to process for all stmts in one round.
    //in a way, to guarantee the lowest priority will get at least one tuple, per this many tuples.
    int roundTotal;

    //use tuple quota or time quota
    quota_mode mode;

    int totalSourceStmt;
    //used to help predict expected max queue length
    double totalSrcOriPriority;
    //used to help scale quotas
    double totalSrcPriority;

    //indicate whether there is currently a statement with deadline, to save work of checking for deadlines when none is set
    bool stmtWithDeadline;
    
  private:
  };

  //abstract class representing all scheduling mechanisms
  class ScheduleMechanism {
  public:
      
    ScheduleMechanism(Driver* drv):deadlineMode(false) { this->drv = drv; }
	
    virtual ~ScheduleMechanism() {}
    
    virtual run_rc runNextUnit(int &nTuples) = 0;

    //update is called whenever driver is changed, to add or remove entries, or stmt weights are changed
    virtual void update() = 0;
    
    virtual void printStateTable() = 0;
    
  protected:
    
    Driver* drv;

    //default to false, only set to true when scheduling decision is moved into deadline handling mode
    bool deadlineMode;
        
  private:
  };

  /*
  struct QuotaEntry:public StmtEntry {
    QuotaEntry(stmt* s) : StmtEntry(s) { }
    
    union {
      long quota; //assigned quota
      double time_quota;
    };
    
    union {  
      long quota_left; //quota still left to be used, in this round.
      double time_quota_left;
    };
    
  };
  */
 
  class ScheduleStrategy; //forward decla
  class WeightGroupSchedule : public ScheduleMechanism {
  public:
    static int AVG_PER_JOB_ROUND;
    int recursion_count;
      
    WeightGroupSchedule(Driver* drv, ScheduleStrategy* strategy, WeightGroup* wg) : ScheduleMechanism(drv) { recursion_count = 0; this->strategy = strategy; this->wg = wg; last_stmt = NULL; /*stateTableChanged = false;*/}
    virtual ~WeightGroupSchedule() {}
    
    void setWeightGroup(WeightGroup* wg) {this->wg = wg;}
    
    virtual run_rc runNextUnit(int &nTuples) = 0;
    virtual void update();
    
    int getWeight() { return wg->weight; }
    
    virtual void printStateTable();
    
  protected:

    //responsible to run the graph started from this buffer, and if applicable to pop input buffer 
    //when it is source also decrement nTuples, decrement quota_left
    //set output flag whenever at least one tuple is produced.
    virtual run_rc runState(b_entry state, int &nTuples, bool* output = NULL);

    //default does nothing, only the deadline aware schedules will overwrite this method
    virtual void updateQuotaByDeadline(StmtEntry* se) {}
    
    char* last_stmt; //to be used to remember where last exit site was, when prompted out to process messages

    //moved over to the Driver class
    //flag used for interaction between runNextUnit and RunState. RunState may trigger statetables rebuild
    //when rebuid happens, iteration in runNextUnit on the statetable has to terminate to avoid exceptions.
    //bool stateTableChanged; 
    
    WeightGroup* wg;
    ScheduleStrategy* strategy;
    //vector<QuotaEntry*> vQuotas; //sorted by stmt name?

    //default to false, only set to true when scheduling decision is moved into deadline handling mode
    //moved up to schedule machanism.
    //bool deadlineMode;
    
  private:
  };
  
  //represent all profile schedulers
  class ProfileSchedule : public ScheduleMechanism {
    
  public:
      
    ProfileSchedule(Driver* drv, WeightProfile* wp, ScheduleStrategy* strategy) : ScheduleMechanism(drv) {
      this->wp = wp; this->strategy = strategy; //update(); //base class can not call pure virtual in constructor, make sure children call
    }
    
    virtual ~ProfileSchedule();
    
    void setWeightProfile(WeightProfile* wp) {this->wp = wp;}

    virtual void update();
    
    virtual void printStateTable();

    virtual run_rc runNextUnit(int &nTuples) = 0;
    
    vector<WeightGroupSchedule*> vSGroupSchedules; //pair a WeightGroupSchedule with each WeightGroup in the WeightProfile, sorted by weights
    
  protected:
    
    WeightProfile* wp;
    ScheduleStrategy* strategy;
    
  private:
  };
  
  //becomes a mere wrapper to pair up a ProfileSchedule for a WeightProfile, and a WeightGroupSchedule for each Weight Group in the profile
  class ScheduleStrategy {
  public:
    ScheduleStrategy(Driver* drv);
    virtual ~ScheduleStrategy();

    void setDriver(Driver* drv); 

    WeightProfile* getWeightProfile() {return this->profile;}
    ProfileSchedule* getSchedule() {return this->schedule;}
    
    virtual run_rc runNextUnit(int &nTuples) {return schedule->runNextUnit(nTuples);} 
    
    //update is called whenever driver is changed, to add or remove entries, or stmt weights are changed
    virtual void update();
    
    virtual void printStateTable() {this->profile->printStateTable(); this->schedule->printStateTable();}

    virtual ProfileSchedule* createSchedule() = 0;
    virtual WeightProfile* createProfile() = 0;
    virtual WeightGroupSchedule* createWeightGroupSchedule(WeightGroup* wg) = 0;
    
    //if no statement/fragment to run in this strategy yet
    virtual bool empty() {return (profile == NULL || profile->empty());}

    const string& getName() {return name;}
    
  protected:
    string name;
    
    Driver* drv;
    ProfileSchedule* schedule;
    WeightProfile* profile;
    
  private:
  };

  //---------base constructs end----------------

  //---------classes specific for Weights-only profile begin----------------
  class WeightOnlyProfile : public WeightProfile {
  public:
    //static const int DEFAULT_QUOTA = 20;
    static int DEFAULT_QUOTA;
    static int MIN_QUOTA;    
    static const int MAX_QUOTA = 100000;    //this is only used for when quota is irrelevant, to prevent runState to finish prematurely due to small quota set
    
    WeightOnlyProfile(Driver* drv, int roundTotal = -1, quota_mode mode = QUOTA_COUNT_MODE) : WeightProfile(drv, roundTotal, mode) {update();}
    
    virtual ~WeightOnlyProfile() {}

    virtual void update() {WeightProfile::update(); update_quotas();}
    
    //no updates of stmt weight due to stat changes, since they are all equal
    virtual bool isUpdateTrigger(stmt* s) {return false;}
    
  protected:

    //for this class, update_entry does nothing, since weights are assigned once by user
    //weight are not affected by statistics updates
    virtual StmtEntry* update_priority(stmt* s) {return NULL;}
    virtual void update_quotas();
  };
  
  //---------classes specific for Weights-only profile end----------------

  //---------classes specific for ocs profile begin----------------
  
  struct OCSEntry :public StmtEntry {
    double path_capacity; //used for incremental calculation of output capacity
    double path_selectivity;
    OCSEntry(stmt* s):StmtEntry(s),path_capacity(WeightProfile::DEFAULT_VALUE),path_selectivity(WeightProfile::DEFAULT_VALUE) {priority = WeightProfile::DEFAULT_VALUE;}
    static bool change_significant(stmt_entry* s_entry);
    virtual ~OCSEntry() {}
  };
  
  struct ltse {
    bool operator()(const StmtEntry* s1, const StmtEntry* s2) const {
      return s1->stmt_name->compare((*s2->stmt_name)) < 0; 
    }
    bool operator()(const StmtEntry* s1, const string* s2) const {
      return s1->stmt_name->compare((*s2)) < 0; 
    }
    bool operator()(const string* s1, const StmtEntry* s2) const {
      return s1->compare((*s2->stmt_name)) < 0; 
    }
    bool operator()(const StmtEntry* s1, const char* s2) const {
      return s1->stmt_name->compare(s2) < 0; 
    }
    bool operator()(const char* s1, const StmtEntry* s2) const {
      return s2->stmt_name->compare(s1) > 0; 
    }    
  };
  
  class OCSProfile : public WeightProfile {
  public:
    //static const int DEFAULT_QUOTA = 10;
    
    //OCSProfile(Driver* drv, int roundTotal = -1, quota_mode mode = QUOTA_COUNT_MODE) : WeightProfile(drv, roundTotal, mode), maxPriority(0), minPriority(0) {update();}
    OCSProfile(Driver* drv, int roundTotal = -1, quota_mode mode = QUOTA_TIME_MODE) : WeightProfile(drv, roundTotal, mode), maxPriority(0)  {update();}
    
    virtual ~OCSProfile();

    virtual void update();
    
    virtual bool isUpdateTrigger(stmt* s);

    virtual void printStateTable();
    virtual bool change_significant(stmt_entry* monitor_s_entry);
    
  protected:

    virtual StmtEntry* update_priority(stmt* s);
    virtual void update_priority(StmtEntry* se) { return WeightProfile::update_priority(se);}
    virtual void update_quotas();
    
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);
    vector<OCSEntry*> ocsInfo;

    double maxPriority;
    //double minPriority;
  };
  
  //---------classes specific for ocs profile end----------------
  
  //---------classes specific for Two-Level-Weights-Normailized profile begin----------------
  //based on GPS principle, make every stmt to have "equal progress".
  //Weight is normalized by processing time under QUOTA_TIME_MODE, which enable to still use count quotas to control equal time progress
  //under QUOTA_COUNT_MODE, weight is not normalized by time, but used directly.
  //WeightOnlyProfile above is a special case of QUOTA_COUNT mode case, where all count-based weights are equal.
  //this class is similar to OCS in the sense that it requires path_capacity calculation.
  class TlwNormalizedProfile : public WeightProfile {
  public:
    static const int DEFAULT_STMT_WEIGHT = 1;
    
    TlwNormalizedProfile(Driver* drv, int roundTotal = -1, quota_mode mode = QUOTA_TIME_MODE) : WeightProfile(drv, roundTotal, mode) {update();}
    
    virtual ~TlwNormalizedProfile() {}

    virtual void update();
    
    //under QUOTA_TIME_MODE, stat is needed for finalizing weights.
    //under QUOTA_COUNT_MODE, weights are not affected by stats.
    virtual bool isUpdateTrigger(stmt* s);
    
  protected:

    virtual StmtEntry* update_priority(stmt* s);
    
  };
  
  //---------classes specific for Two-Level-Weights-Normailized profile begin----------------
  
  //---------classes specific for PriorityProfileSchedule begin----------------
  
//!!!!!!!!!!!!!! incomplete !!!!!!!!!!!!

  //finishes everything in highest weights, before going to lower ones
  class PriorityProfileSchedule : public ProfileSchedule{
  public:
    PriorityProfileSchedule(Driver* drv, WeightProfile* wp, ScheduleStrategy* strategy) : ProfileSchedule(drv, wp, strategy) {update();}
    
    virtual ~PriorityProfileSchedule() {}
    
    virtual run_rc runNextUnit(int &nTuples);
    
  protected:      
    
  private:
  };
  
  //---------classes specific for PriorityProfileSchedule end----------------  
  
  //---------classes specific for weighted Round Robin machanism begin----------------
  //weighted Round Robin. Can be used to "simulate" priority based algorithms.
  class WRRGroupSchedule : public WeightGroupSchedule{
  public:

    WRRGroupSchedule(Driver* drv, ScheduleStrategy* strategy, WeightGroup* wg) : WeightGroupSchedule(drv, strategy, wg)/*,shuffled(false)*/ {update();}
    
    virtual ~WRRGroupSchedule() {}

    //runs Weighted Round Robin
    //assume quotas were already assigned by WeightProfile, just use it and run round robin, allocate each segment asigned quota per round.
    virtual run_rc runNextUnit(int &nTuples);

  protected:
    //bool shuffled;
  private:
  };

  //---------classes specific for weighted Round Robin machanism end----------------
  
  //---------classes specific for weighted Round Robin with deadline machanism begin----------------
  //weighted Round Robin. With deadline considered
  class WRRDLGroupSchedule : public WRRGroupSchedule{
  public:

    WRRDLGroupSchedule(Driver* drv, ScheduleStrategy* strategy, WeightGroup* wg) : WRRGroupSchedule(drv, strategy, wg) {}
    
    virtual ~WRRDLGroupSchedule() {}
    

    //runs Weighted Round Robin, with deadline consideration
    virtual void updateQuotaByDeadline(StmtEntry* se);
    
    protected:      
      
    private:
  };
  
  //---------classes specific for Weighted Round Robin with deadline machanism end----------------

  //---------classes specific for priority based group schedule machanism begin----------------
  //runs priority based mechanism, finish high priority before ever going to low ones
  class PriorityGroupSchedule : public WeightGroupSchedule{
  public:

    PriorityGroupSchedule(Driver* drv, ScheduleStrategy* strategy, WeightGroup* wg) : WeightGroupSchedule(drv, strategy, wg) {update();}
    
    virtual ~PriorityGroupSchedule() {}

    //runs priority based mechanism, finish high priority before ever going to low ones
    virtual run_rc runNextUnit(int &nTuples);

  protected:
  private:
  };

  //---------classes specific for priority based group schedule machanism end----------------  

  //---------classes specific for priority based group schedule machanism (with deadline considerations) begin----------------  
  //runs priority based mechanism, finish high priority before ever going to low ones
  //record deadline information and current predicted delay for each queue, and switch to those about to be late
  //this version maitains queue delay information in a heap, which is updated each time about leaving an operator 
  class PriorityDLGroupSchedule : public PriorityGroupSchedule{
  public:

    PriorityDLGroupSchedule(Driver* drv, ScheduleStrategy* strategy, WeightGroup* wg) : PriorityGroupSchedule(drv, strategy, wg) {}
    
    virtual ~PriorityDLGroupSchedule() {}

    //runs priority based mechanism, finish high priority before ever going to low ones
    virtual run_rc runNextUnit(int &nTuples);

    //virtual void update(); //no need to overwrite this anymore
    
  protected:
    //need to maintain delay information
    virtual run_rc runState(b_entry state, int &nTuples, bool* output = NULL);

    //a heap structure maintained by their urgencyTime, in order to find the smallest urgencyTime value under deadlineMode
    vector<StmtEntry*> hSEntriesByTime;
      
  private:
  };

  //---------classes specific for priority based group schedule machanism (with deadline considerations) begin----------------  
  
  //---------classes specific for stride machanism begin----------------
  
  struct StrideEntry {
    virtual ~StrideEntry() {}
    
    double stride; //stride = 1/priority
    double nextPass; //marker for how soon I should be next scheduled, to be compared against others.    
  };

  struct gtstrideen {
    bool operator()(const StrideEntry* s1, const StrideEntry* s2) const {
      return s1->nextPass > s2->nextPass; 
    }
  };
  
  struct StrideSegEntry:public StrideEntry, StmtEntry {
    StrideSegEntry(stmt* s):StmtEntry(s) {
    }
    virtual ~StrideSegEntry() {}
  };

  struct StrideWeightGroup :public StrideEntry, WeightGroup {
    StrideWeightGroup(int weight) : WeightGroup(weight) {}
    
    virtual ~StrideWeightGroup() {}

    //the segments form a priority queue, compare using nextPass of each StrideSegEntry
    priority_queue<StrideSegEntry*, vector<StrideSegEntry*>, gtstrideen> queue;

    double numTicketsMax;
    double numTicketsMin;
  };


  class StrideWeightProfile :public WeightProfile{
  public:  
    StrideWeightProfile(Driver* drv, int roundTotal = -1) : WeightProfile(drv, roundTotal)  {update();}
    
    virtual ~StrideWeightProfile();
    
  protected:

    //the groups form a priority queue, compare using nextPass of each StrideWeightGroup
    priority_queue<StrideWeightGroup*, vector<StrideWeightGroup*>, gtstrideen> queue;

    //to change
    virtual StmtEntry* update_priority(stmt* s) {return NULL;}    
    
  private:
    
  };
  
  //---------classes specific for stride machanism end----------------

  //---------classes specific for WeightOnly WRR schedule begin
  class WeightOnlyWRRStrategy : public ScheduleStrategy {
  public:
    WeightOnlyWRRStrategy(Driver* drv) : ScheduleStrategy(drv) {name = "WeightOnlyWRR"; update();}
      
    virtual ~WeightOnlyWRRStrategy() {}

    virtual WeightProfile* createProfile();    
    virtual ProfileSchedule* createSchedule();
    virtual WeightGroupSchedule* createWeightGroupSchedule(WeightGroup* wg);
    
  protected:    
  private:
  };

  //---------classes specific for WeightOnly WRR schedule end  

  //---------classes specific for OCS WRR schedule begin
  class OCSWRRStrategy : public ScheduleStrategy {
  public:
    OCSWRRStrategy(Driver* drv) : ScheduleStrategy(drv) {name ="OCSWRR"; update();}
      
    virtual ~OCSWRRStrategy() {}

    virtual WeightProfile* createProfile();    
    virtual ProfileSchedule* createSchedule();
    virtual WeightGroupSchedule* createWeightGroupSchedule(WeightGroup* wg);
    
  protected:    
  private:
  };

  //---------classes specific for OCS WRR schedule end
  
  //---------classes specific for OCS WRR with deadline schedule begin
  class OCSWRRDLStrategy : public ScheduleStrategy {
  public:
    OCSWRRDLStrategy(Driver* drv) : ScheduleStrategy(drv) {name ="OCSWRRDL"; update();}
      
    virtual ~OCSWRRDLStrategy() {}

    virtual WeightProfile* createProfile();    
    virtual ProfileSchedule* createSchedule();
    virtual WeightGroupSchedule* createWeightGroupSchedule(WeightGroup* wg);
    
  protected:    
  private:
  };
  
  //---------classes specific for WeightOnly WRR with deadline schedule end  

  //---------classes specific for OCS Priority schedule begin
  class OCSPriorityStrategy : public ScheduleStrategy {
  public:
    OCSPriorityStrategy(Driver* drv) : ScheduleStrategy(drv) {name ="OCSPriority"; update();}
      
    virtual ~OCSPriorityStrategy() {}

    virtual WeightProfile* createProfile();    
    virtual ProfileSchedule* createSchedule();
    virtual WeightGroupSchedule* createWeightGroupSchedule(WeightGroup* wg);
    
  protected:    
  private:
  };

  //---------classes specific for OCS Priority schedule end

  //---------classes specific for OCS Priority schedule with deadline begin
  class OCSPriorityDLStrategy : public ScheduleStrategy {
  public:
    OCSPriorityDLStrategy(Driver* drv) : ScheduleStrategy(drv) {name ="OCSPriorityDL"; update();}
      
    virtual ~OCSPriorityDLStrategy() {}

    virtual WeightProfile* createProfile();    
    virtual ProfileSchedule* createSchedule();
    virtual WeightGroupSchedule* createWeightGroupSchedule(WeightGroup* wg);
    
  protected:    
  private:
  };

  //---------classes specific for OCS Priority schedule with deadline end
  
  //---------classes specific for Strategy Factory begin
  //create and assign strategies, clean up un-used ones in one central location.
  
  class StrategyFactory {
  public:
    ~StrategyFactory();

    static StrategyFactory* getInstance();
    static void destroy();
    void addToTrash(ScheduleStrategy* strat) {trash.push_back(strat);}
    
    void cleanupTrash();
    
  protected:
    StrategyFactory();
  private:
    static StrategyFactory* _instance;
    vector<ScheduleStrategy*> trash;
  };

  //---------classes specific for Strategy Factory end
  
  
  /**------------------------
     Everything below are old code for strategies, before instroducing the stride machanism.
     After which it seems to be more benificial to distinguish weight/priority/quota calculation (such as chain)
     from the mechanism of actual executing the segments in a certain order (such as stride).
  ------------------------**/
  
  typedef struct strat_ety {
    char* stmt_name;
    //struct timeval last_run;

    strat_ety(char* name) { 
      this->stmt_name = new char[strlen(name)+1]; 
      strcpy(this->stmt_name, name);
    }
    virtual ~strat_ety() { delete this->stmt_name;}
  } strat_entry;

  class eqse : public std::unary_function<char*, bool> {
  public:

    eqse(char* name) {value = name;}

    bool operator()(strat_entry* s) const {
	return strcmp(s->stmt_name, value) == 0;    
    }  

  private:
    char* value;
  };
 
  class Strategy {
  public:
      Strategy(Driver* drv);
      virtual ~Strategy() = 0;

      void setDriver(Driver* drv);

      //default implementation that runs 1 tuple at a time, not very efficient.
      run_rc runNextUnit() { int nTuples = 1; return runNextUnit(&nTuples); }

      //different algorithm may have different unit of scheduling (stmt or segment of stmts)
      //call this one unit at a time, such that new tuple arrivals will be able to get in front
      virtual run_rc runNextUnit(int* nTuples) = 0; 

      //update is called whenever driver is changed, to add or remove entries
      //assign prioroty to added statements according to performance data 
      //when no data available everyone has the same default priority OperatorStrategy::MAX_PRIORITY
      virtual void update() = 0;

      //update_all is a maintenance call, periodically update priority of all statements based on latest performance data
      //rebuild the entire map (to get the correct order)
      virtual void update_all() = 0;

      //check whether the stat numbers have changed significantly, such that a priority update is needed. diff for diff algorithms
      virtual bool change_significant(stmt_entry* monitor_s_entry) = 0;

      //by default every statement can trigger priority/quota update. 
      //specific algorithm may change this behavior, such as in OCS only sinks will trigger
      virtual bool isUpdateTrigger(stmt* s) {return true;} 

      virtual void printStateTable() = 0;

      //if no statement/fragment to run in this strategy yet
      virtual bool empty() = 0;

      const string& getName() {return name;}
    
    protected:

      //calculate individual priority for the statement, if not already set.
      //create entry in info table, if entry not already there
      //set priority on stmt object 
      //if priority set, do nothing
      virtual void update_entry(stmt* s) = 0;

      Driver* drv;

      string name;
      
    private:
  };

  //keyed and sorted by last known (minus) priority of the stmt.
  //since this is by definition in acending order, use the minus of priority as the key, to ensure high priority at front
  typedef multimap<double, strat_entry*> StratInfoType; 

  //all priority based strategy.
  class PriorityStrategy:public Strategy {
  public:    

    static const double MAX_PRIORITY = 100000000.0;

    PriorityStrategy(Driver* drv);

    virtual void printStateTable();

    virtual ~PriorityStrategy();

    StratInfoType* getInfoTbl() {return stmtsInfo;}

    virtual bool empty() {return (stmtsInfo == NULL || stmtsInfo->size() == 0);}

  protected:

    virtual void update_entry(stmt* s) = 0; //make this class abstract

    StratInfoType* stmtsInfo;    

  private:
  };

  typedef struct quota_ety:public strat_entry {
    long quota; //assigned quota
    long quota_left; //quota still left to be used, in this round.
    rank_t rank; //used to store information to derive the actual quota. This could be a priority, or another theoritical quota (before derive into the actual quota)

    quota_ety(char* name);
    quota_ety(char* name, long q);
    virtual ~quota_ety() {}
  } quota_entry;

  //all quota-based strategy.
  class QuotaStrategy:public Strategy {
  public:    

    static const long DEFAULT_QUOTA = 20;

    QuotaStrategy(Driver* drv, long default_quota=QuotaStrategy::DEFAULT_QUOTA, bool fixed_quota=false);

    virtual ~QuotaStrategy();

    list<quota_entry*>* getInfoTbl() {return stmtsInfo;}

    virtual void printStateTable();

    virtual bool empty() {return (stmtsInfo == NULL || stmtsInfo->size() == 0);}

  protected:

    virtual void update_entry(stmt* s) = 0; //make this class abstract    

    list<quota_entry*>* stmtsInfo;

    long default_quota;

    bool fixed_quota;

  private:
  };

  class SegmentMap {
  public:    
    SegmentMap(Driver* drv);

    virtual ~SegmentMap(); 

  protected:

    BufStateTblType* segmentMap;

  private:
  };

  //priority based and operator based strategy
  //all operator based strategies run one stmt at a time
  //can share the same implementation of runNextUnit, update_all, etc. (unless there is a need to overwrite)  
  class OperatorPriorityStrategy:public PriorityStrategy {
  public:    
      
    OperatorPriorityStrategy(Driver* drv);
    
    virtual void update_all();
    
    virtual run_rc runNextUnit(int* nTuples);
    
    virtual ~OperatorPriorityStrategy();
    
  protected:
    //virtual void update_entry(stmt* s);

  private:
  };

  //priority based and segment based strategy
  //segmentedMap is a copy of the full query graph, but with algorithm-specific segmentations.
  //all segment based strategies run one segment at a time (a segment is a group of operators, possibly from multiple paths, especially in the case of forks)
  //can share the same implementation of runNextUnit, update_all, etc. (unless this is a need to overwrite)
  class SegmentPriorityStrategy:public PriorityStrategy, public SegmentMap {
    public:    
      
    SegmentPriorityStrategy(Driver* drv);
    
    virtual void update_all();
    
    virtual run_rc runNextUnit(int* nTuples);
    
    virtual ~SegmentPriorityStrategy(); 
    
  protected:

  private:
  };

  //the reason to store rank (either priority, or anything else) separately from strat_entry is that, in segment strategy strat_entry is only kept for stmts that are sources.
  //however, to calculate quota/priority of the sources, we frequently need extra information kept for EVERY statement. hence this data type.
  typedef struct extra_info_ety {
    rank_t rank;
    virtual ~extra_info_ety() {}
  } extra_info_entry;

  typedef hash_map<char*, extra_info_entry*, std::hash<char*>, eqstr> ExtraInfoMapType; 

  //a funtor used to specifically sort stmtInfo based on rank stored
  struct greater_on_rank : public binary_function<quota_entry*, quota_entry*, bool> {
    bool operator()(quota_entry* x, quota_entry* y) { return x->rank.priority > y->rank.priority; }
  };

  //quota based and segment based strategy
  //segmentedMap is a copy of the full query graph, but with algorithm-specific segmentations.
  //all segment based strategies run one segment at a time (a segment is a group of operators, possibly from multiple paths, especially in the case of forks)
  //can share the same implementation of runNextUnit, update_all, etc. (unless this is a need to overwrite)
  class SegmentQuotaStrategy:public QuotaStrategy, public SegmentMap {
  public:    
      
    SegmentQuotaStrategy(Driver* drv, long default_quota=QuotaStrategy::DEFAULT_QUOTA, bool fixed_quota=false);

    //update the stmtsInfo list with segments information, after driver changes are made
    //each stmt with some source buffer (later will include those become source buffer after segmentation) has an entry. 
    //execution will be bush-depth-first on each stmt, round robin on stmts, with quota
    virtual void update_all();
    
    //execution will be round robin on each stmt with some source buffer (in topology maintained here, which may have extra break points, 
    //besides the original source buffers)  with quota.
    //on each stmt it will be bushy-depth-first, with backtrack.
    virtual run_rc runNextUnit(int* nTuples);
    
    virtual void printStateTable();

    virtual ~SegmentQuotaStrategy(); 
    
  protected:

    //to be used for child classes for any extra info to keep for every stmt. 
    //since here the main list only has source stmts, due to using segment as unit.
    //now changed from list to map, to assist fast access
    ExtraInfoMapType* extraInfo;
    //list<strat_entry*>* extraInfo; 

    //responsible to run the graph started from this buffer, and if applicable to pop input buffer 
    //when it is source also decrement nTuples, decrement quota_left
    //set output flag whenever at least one tuple is produced.
    virtual run_rc runState(b_entry state, int* nTuples, bool* output = NULL);

    virtual void update_entry(stmt* s); 

    list<quota_entry*>::iterator last_itr; //to be used to remember where last exit site was, in sequential style processing of sources

    //flag used for interaction between runNextUnit and RunState. RunState may trigger statetables rebuild
    //when rebuid happens, iteration in runNextUnit on the statetable has to terminate to avoid exceptions.
    //bool stateTableChanged; 

  private:
  };

  typedef struct ocs_ety {
    double path_capacity; //used for incremental calculation of output capacity
    double path_selectivity;
    ocs_ety():path_capacity(PriorityStrategy::MAX_PRIORITY),path_selectivity(PriorityStrategy::MAX_PRIORITY) {}
    static bool change_significant(stmt_entry* s_entry);
    virtual ~ocs_ety() {}
  } ocs_entry;

  typedef struct ocs_strat_ety : public strat_entry, public ocs_entry {
    ocs_strat_ety(char* name):strat_entry(name),ocs_entry() {}
    virtual ~ocs_strat_ety() {}
  } ocs_strat_entry;

  //Output Capacity Strategy
  class OcsStrategy:public OperatorPriorityStrategy {
  public:
    
    static const double SEL_CHANGE_THRESHOLD = 0.3; //used by function change_significant()
    static const double PROC_CHANGE_THRESHOLD = 0.3; //used by function change_significant()

    OcsStrategy(Driver* drv);
    
    virtual void update();

    virtual bool change_significant(stmt_entry* s_entry);
    virtual bool isUpdateTrigger(stmt* s);

    virtual void printStateTable();

    virtual ~OcsStrategy();
    
  protected:
    //due to the calculation of priority in this strategy, this method will recursively update entries for all downstream operators, if the entries are not already there.
    //this one set path_capacity and path_selectivity here too, besides set priority in stmt.
    virtual void update_entry(stmt* s);
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);

  private:
  };

  //path capacity strategy
  //only difference is in how to calculate priority, which is only the path_capacity here.
  class PcsStrategy:public OcsStrategy {
  public:

    PcsStrategy(Driver* drv);

    virtual ~PcsStrategy();
    
  protected:
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);

  private:
  };

  //TRAIN strategy, which is really CHAIN, but without the segmentation (always consider to the end of path). To make this CHAIN, one only needs to add segmentation.
  //only difference from ocs is how to calculate priority : it is the path_capacity times input tuple size.
  class TrainStrategy:public OcsStrategy {
  public:

    TrainStrategy(Driver* drv);

    virtual ~TrainStrategy();
    
  protected:
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);
  private:
  };

  //Strict Round Robin, take one tuple at a time
  class RRStrategy:public SegmentQuotaStrategy {
    public:    
      
    RRStrategy(Driver* drv, long default_quota=1, bool fixed_quota=true);

    virtual void update();
    virtual void update_all();
    
    virtual bool change_significant(stmt_entry* s_entry);
 
    virtual ~RRStrategy(); 
    
  protected:

    //this is not needed in RR, since default implementation in SegmentQuotaStrategy is enough. I never need to update quota.
    //virtual void update_entry(stmt* s); 

  private:
  };

  //keep priority here, since in the parent it was kept elsewhere in the map. Here we need to keep this information explicitely in this data structure.
  //to be used in the extraInfo list. Here we use the priority field of rank_t to store ocs priority.
  typedef struct ocs_quota_extra_ety : public ocs_entry, public extra_info_entry {
    ocs_quota_extra_ety() {rank.priority = PriorityStrategy::MAX_PRIORITY;}
  } ocs_quota_extra_entry;

  //Quota on segments, based on OCS priority score. 
  //the scheme to use quota: a fixed total value for all, and assign all to the highest OCS one, until it finished all tuples, assign the rest to the next one, and so on.
  //reserve enough so that every other segment at least has one tuple as quota each round.
  class OcsQuotaStrategy:public SegmentQuotaStrategy {
    public:    
      
    static long TOP_QUOTA; //quota for top priority
    static long NEW_QUOTA; //quota for new queries, in phase of learning.

    OcsQuotaStrategy(Driver* drv, long default_quota=1, bool fixed_quota=false);

    //this will be overwritten to update quota after execution. The order of execution is also no longer FCFS, but by priority.
    virtual run_rc runNextUnit(int* nTuples);

    virtual void update();
    virtual void update_all();
    
    virtual bool change_significant(stmt_entry* s_entry);
    //only sinks are update trigger
    virtual bool isUpdateTrigger(stmt* s);

    virtual void printStateTable();
 
    virtual ~OcsQuotaStrategy(); 
    
  protected:

    //this function now has to take care of calculating ocs related information in extraInfo too, besides handling stmtsInfo.
    virtual void update_entry(stmt* s); 
    //return priority
    //the workhorse to actually build extraInfo list
    virtual double updatePriority(stmt* s);
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);

  private:
  };
  
  //the same as OcsQuotaStrategy, except using a Roulette method to select the next segment to run.
  //statements are selected probablistically, based on the value of their ocs priority.
  //the quota and quota_left values are not used, only rank.priority is used for calculation.
  class OcsRouletteSegStrategy:public OcsQuotaStrategy {
    public:    

    //no longer use this, try to use average of available priorities.
    //static long NEW_RANK_ROULETTE; //rank for new queries before stats are available for priority calculation, used in roulette.

    OcsRouletteSegStrategy(Driver* drv, long default_quota=OcsQuotaStrategy::TOP_QUOTA);

    //this will be overwritten to use probability to select next one.
    virtual run_rc runNextUnit(int* nTuples);

    virtual void printStateTable();
 
    virtual ~OcsRouletteSegStrategy(); 
    
  protected:
    //does the roulette selection of next stmt to run
    stmt* roulette(list<quota_entry*>* pool, long* total_rank, long* avg_rank);

  private:
  };

  //utilize code from OcsQuotaStrategy, but always go to highest priority and not use quotas
  //the quota and quota_left values are not used, only rank.priority is used.
  class OcsSegStrategy:public OcsQuotaStrategy {
    public:    

    OcsSegStrategy(Driver* drv, long default_quota=OcsQuotaStrategy::TOP_QUOTA);

    //this will be overwritten to always go to highest priority whenever a new tuple arrives.
    virtual run_rc runNextUnit(int* nTuples);

    virtual void printStateTable();
 
    virtual ~OcsSegStrategy(); 
    
  protected:

  private:
  };

  //only different from OcsSegStrategy in the way next stmt is selected
  //no optimization, always go back to the front of queries after one tuple.
  class OcsSegNoOptmztnStrategy:public OcsSegStrategy {
    public:

    OcsSegNoOptmztnStrategy(Driver* drv);
 
    virtual ~OcsSegNoOptmztnStrategy(); 
    
    //always go to highest priority, after every tuple processed.
    virtual run_rc runNextUnit(int* nTuples);
 
  protected:

  private:
  };

  //only different from OcsSegStrategy in the way priority is calculated.
  class PcsSegStrategy:public OcsSegStrategy {
    public:    

    PcsSegStrategy(Driver* drv);
 
    virtual ~PcsSegStrategy(); 
    

  protected:
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);

  private:
  };

  //only different from OcsSegStrategy in the way priority is calculated.
  class TrainSegStrategy:public OcsSegStrategy {
    public:    

    TrainSegStrategy(Driver* drv);
 
    virtual ~TrainSegStrategy(); 
    

  protected:
    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);

  private:
  };

  //Same as OCSQuota except prioritiy is calculated differently
  class TrainQuotaStrategy:public OcsQuotaStrategy {
    public:    

    TrainQuotaStrategy(Driver* drv);

    virtual ~TrainQuotaStrategy(); 
    
  protected:

    virtual double calculatePriority(double path_capacity, double path_selectivity, stmt_entry* stat);

  private:
  };
  
  /* these do not work, remain to find out why
  struct ltstmt {
    bool operator()(const stmt* s1, const stmt* s2) const
    {
      return s1->priority < s2->priority;
    }
  };

  struct gtstmt {
    bool operator()(const stmt* s1, const stmt* s2) const
    {
      return s1->priority > s2->priority;
    }
  };
  
  
  template<class T>
    struct dereference_less {
      bool operator()(T* t1, T* t2)
      {
	return *t1 < *t2;
      }
    };

  template<class T>
    struct dereference_greater {
      bool operator()(T* t1, T* t2)
      {
	return *t1) > *t2;
      }
    };
  */

  /* this one works, but since switch to dummy, no longer needed
  //used for find_if, in inserting stmt to correct location on the list
  class GreaterThanStmt : public std::unary_function<stmt*, bool> {
    public:
    
    GreaterThanStmt(stmt* s)  {value_ = s;}

    bool operator()(stmt* t) const {
      return t->priority > value_->priority;
    }
    
    private:
    
    stmt* value_;
  };
  */

  /* this one works, but eventually decide not to use it, instead use priority as key is the most easy
  struct ltstrat {
    bool operator()(char* s1, char* s2) const {
      DrvMgr* dm = DrvMgr::getInstance();

      bool s1InUse = false;
      bool s2InUse = false;

      if (dm->stmtInUse(s1)) s1InUse = true;
      if (dm->stmtInUse(s2)) s2InUse = true;

      if(s1InUse && !s2InUse) return true; //if there are not-in-use ones, they are placed at the end removed.
      else if (!s1InUse && s2InUse) return false;
      else if (s1InUse && s2InUse) {
	//use greater than instead of less than, since we want descending on priority
	return (dm->getStmtByName(s1)->priority) > (dm->getStmtByName(s2)->priority);
      }
      return false;
    }
  };
  */
} // end of namespace ESL
#endif
