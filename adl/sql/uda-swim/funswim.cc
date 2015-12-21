//#include <dlfcn.h>
#include "adllib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C" {
#include "swimlib.h"
}

extern char * _allocateResultSpace(int size);
extern void _adl_dlm_init();
extern void _adl_dlm_delete();

const int MAX_FREQ_ITEM_LEN = 100; // should change the singature too.

static SwimFisStatus sfs;
static int __init = 1;

extern "C" 
int swimfis(int tid, struct iExt_ cur_trans)
{
  if(__init == 1) {
	__init = 0;
		char tabName[20];
                int rc;
                DBT key, data;
                DB* paramTbl;
		DBC * paramTable;
                char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];
		char param_name[MAX_FREQ_ITEM_LEN+1];
		char param_value[MAX_FREQ_ITEM_LEN+1];
		int argv[6] = {0,0,0,0,0,0};
		int first_entry = 1;

		    if ((rc = db_create(&paramTbl, NULL, 0)) != 0) {
		      adlabort(rc, "db_create()");
		   }
		   if ((rc = paramTbl->set_pagesize(paramTbl, 2048)) != 0) {
		      adlabort(rc, "set_pagesize()");
		   }
		   if ((rc = paramTbl->set_flags(paramTbl, DB_DUP)) != 0) {
		      adlabort(rc, "set_flags()");
		   }
   		if ((rc = paramTbl->open(paramTbl, "paramTable.data", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      		adlabort(rc, "open()");
  		 }


		sfs.trans_avg_len = 0;
		sfs.no_items = 0;
		sfs.global_min_supp = 0;
		sfs.slide_min_supp = 0;
		sfs.window_size = 0; 
		sfs.slide_size = 0;
		sfs.L_delay_max = 0;
		sfs.no_slides = 0;
		sfs.sum_all_pat_lens = 0;
		sfs.max_needed_mem = 0;

		sfs.StreamMillPtr = 0;

		//create table swimparams(param_name char(100), param_value char(100)) memory;
		sprintf(tabName, "swimparams"); 
		if ((rc = paramTbl->cursor(paramTbl, NULL, &paramTable, 0)) != 0) {
			adlabort(rc, "DB->cursor()");
		}

		while (rc==0) {
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));
		rc = paramTable->c_get(paramTable, &key, &data, (first_entry)? DB_FIRST:DB_NEXT);
		if (rc==0) {
		first_entry = 0;
		memcpy(param_name, (char*)data.data, 100);
		param_name[100] = '\0';
		memcpy(param_value, (char*)data.data+100, 100);
		param_value[100] = '\0';
		//TODO: Now use these pairs to fill up the configuration parameters.
		if (!strcmp(param_name, "trans_avg_len")) {
			sfs.trans_avg_len = atoi(param_value);
			argv[0] = 1;
		}	else if (!strcmp(param_name, "window_size")) {
			sfs.window_size = atoi(param_value);
			argv[1] = 1;
		}	else if (!strcmp(param_name, "slide_size")) {
			sfs.slide_size = atoi(param_value);
			argv[2] = 1;	
		}	else if (!strcmp(param_name, "L_delay_max")) {
			sfs.L_delay_max = atoi(param_value);
			argv[3] = 1;
		}	else if (!strcmp(param_name, "no_items")) {
			sfs.no_items = atoi(param_value);
			argv[4] = 1;
		}	else if (!strcmp(param_name, "min_supp_10000")) {
			sfs.global_min_supp = atoi(param_value);
			sfs.slide_min_supp = atoi(param_value);
			argv[5] = 1;
		}
		} 

		} /* while (rc==0) */
		//Initialization starts
		for (int i=0; i<6 ; ++i)
		if (!argv[i]){
		fprintf(stderr,"Too few parameters. Agrv[%d] missing. ",i);
		return -1000;// should be probably replaced with a fail() function.
		}

		sfs.global_min_supp = ceil(sfs.global_min_supp * sfs.window_size / 10000.0);
		sfs.slide_min_supp = ceil(sfs.slide_min_supp * sfs.slide_size / 10000.0);

		fprintf(stderr,"# sfs.global_min_supp=%d,sfs.slide_min_supp=%d,sfs.window_size=%d,sfs.slide_size=%d,sfs.L_delay_max=%d\n",sfs.global_min_supp,sfs.slide_min_supp,sfs.window_size,sfs.slide_size,sfs.L_delay_max);

		sfs.no_slides = sfs.window_size / sfs.slide_size;
		if (sfs.window_size != (sfs.no_slides*sfs.slide_size) || sfs.L_delay_max < 0 || sfs.L_delay_max >= sfs.no_slides){
		fprintf(stderr,"Error:sfs.slide_size must be a divisor of sfs.window_size.\n");
		return -1000;	
		}

		//Allocating memories and initialization	
		sfs.StreamMillBuffer = (int *)malloc(MAX_FREQ_ITEM_LEN*sizeof(int)*sfs.slide_size);
		sfs.trans.items = (int*)malloc(sizeof(int)*sfs.no_items);
		sfs.archive_fpos = (long*)malloc(sizeof(long)*(sfs.no_slides+1));
		sfs.localFPs = (Tree **)malloc((sfs.no_slides+1)*sizeof(Tree *));
		for (int i=0; i<=sfs.no_slides; i++)
		sfs.localFPs[i] = new_tree(sfs.no_items);
		sfs.local_all_trans_len = (int *)malloc((sfs.no_slides+1)*sizeof(int));
		//itemName is what we read/write from/in the input/output
		//But itemNo is our ordering over single items
		// In SWIM implementations we assumet the single item numbers exatly like the output of IBM 
		// generator:	
		//Basically, sfs.order[itemName] = itemNo	
		//For the first scan, itemNo = sfs.order[itemName] = itemName
		//Also recall that the sfs.order's sfs.index is always between 0 and sfs.no_items-1
		//And that's why we do not say malloc(sizeof(int)*(sfs.no_items+1)
		sfs.order = (int*)malloc(sizeof(int)*(sfs.no_items));
		sfs.rev_order = (int*)malloc(sizeof(int)*(sfs.no_items));
		for (int i=0; i<sfs.no_items; i++){
		sfs.order[i] = i;
		sfs.rev_order[i] = i;
		}

		//Notice that 0<= itemNo <= sfs.no_items-1
		sfs.patternTree = new_tree(sfs.no_items);
		sfs.temp_patternTree = new_tree(sfs.no_items);

                return 1;
	}
		//Initialization end
		int windowid;
		/*printf("Tran %d - %d: ", tid, cur_trans.length);
		for (int i=0; i<cur_trans.length; ++i)
		printf("%d ",cur_trans.pt[i]);
		printf("\n");
                */
		sfs.StreamMillPtr += addTuple(cur_trans.length, cur_trans.pt, sfs.StreamMillBuffer, sfs.StreamMillPtr);
		if ( ((tid+1)%sfs.slide_size)==0 ){
		windowid = tid/sfs.slide_size;
		ProcessMain(windowid, &(sfs));
		sfs.StreamMillPtr = 0;
  		}
  
}

