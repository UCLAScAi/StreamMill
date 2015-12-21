#ifndef SQL_SYSTEM_H
#define SQL_SYSTEM_H

#define REL_TABLE ".nt_table"
#define REL_SIZE 1024

#define UDF_TABLE ".nt_udf"
#define UDA_TABLE ".nt_uda"

int addRelation(table_t *t);
int deleteRelation(char *name);

#endif
