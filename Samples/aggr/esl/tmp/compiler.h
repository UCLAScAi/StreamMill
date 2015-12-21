#include <buffer.h>
#include <driver.h>

extern    S_table E_base_tenv;
extern    S_table E_base_venv;

typedef enum {
  cmp_simple=0,
  cmp_aggr,
  cmp_snapshot,
} compile_opt;

namespace ESL{
  class compiler{
    DrvMgr *dm ;
    bufferMngr *bm;

    FILE* tempStdout;
    FILE* tempStderr;

  public:
    compiler();
    int init();
    static compiler* _instance;
    static compiler* getInstance();
    int compile(const char* queryName, compile_opt opt = cmp_simple);
    

    void redirectStdout(char*, char*, FILE*);
    void redirectStderr(char*, char*, FILE*);
    FILE* getTempStdout();
    FILE* getTempStderr();
    void resetStdouterr();
  };
}


