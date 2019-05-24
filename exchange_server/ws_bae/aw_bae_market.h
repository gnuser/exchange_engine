#ifndef _AW_LOAD_H_
#define _AW_LOAD_H_

# include "ut_mysql.h"

#define MAX_MARKET_NUM 200
#define MAX_SHORT_LEN 8 
#define MAX_MARKET_LEN 16 
#define MAX_CHINESENAME_LEN 32 

typedef struct market_type{
    char market[MAX_MARKET_LEN];
    char *sellname;
    char *buyname;
    char *sellchinesename; 
    char *buychinesename; 
}market_type;

int init_from_db();

#endif
