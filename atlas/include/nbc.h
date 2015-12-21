
/* nbc.h */

#define MAX_CLASSES     20   /* maximum no. of classes */
#define MAX_FEATURES   200   /* maximum no. of features */
#define MAX_DATA       60000  /* maximum no. of instances */
#define MAX_NAME        60   /* maximum length of a name */
#define MAX_LINE      2000   /* maximum length of a line */

#define WINDOW_TR        20000 /* window for training */
#define WINDOW_TE        20000 /* window for testing */

#define INFINITE  (float)2.0E+10

#define KNOWN '*'
#define UNKNOWN '?'
#define UNDETERMINED 0
#define TRUE  1
#define FALSE 0

typedef int FEATURE;
typedef int CLASS;
typedef float VALUE;

typedef struct {
  VALUE value[MAX_FEATURES];
  char status[MAX_FEATURES];
  CLASS Class;			/* 0 for unknown */
} INSTANCE;

typedef struct {
  VALUE value;
  int count;
} VALCOUNT;


void read_info();                             /* input.c */
int  read_train_data(), read_test_data();     /* input.c */
void open_log_file(), log_message();          /* output.c */
void log_instance(), log_data();              /* output.c */
void show_instance(), show_data();            /* output.c */
void initializeNBC();
