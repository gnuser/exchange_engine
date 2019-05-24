#include "aw_bae_market.h"
#include "aw_config.h"
#include "ut_sds.h"
#include "ut_log.h"
#include <stdlib.h>
#include <stdio.h>
extern struct settings settings;

static void init_markets(size_t market_num)
{
    settings.market_num = market_num;    
    settings.markets = malloc(sizeof(market_type) * market_num);
    memset(settings.markets, 0 , sizeof(market_type) * market_num);
}



static int load_markets_from_db(MYSQL* conn, const char* table1, const char* table2)
{
    sds sql = sdsempty(); 
    sql = sdscatprintf(sql,"SELECT a.name,a.short_name,b.name,b.short_name \
                            FROM \
                            (SELECT name,short_name,buy_coin_id, system_trade_type.id AS tradeid FROM %s, %s \
                            WHERE system_coin_type.id = buy_coin_id AND system_trade_type.status =1)a \
                            INNER JOIN \
                            (SELECT name,short_name, sell_coin_id , system_trade_type.id AS tradeid \
                            FROM system_coin_type, system_trade_type WHERE system_coin_type.id = sell_coin_id AND system_trade_type.status =1)b \
                            ON a.tradeid = b.tradeid;",table1,table2);
    log_trace("exec sql: %s", sql);
    int ret = mysql_real_query(conn, sql, sdslen(sql));
    if (ret != 0) {
            log_error("exec sql: %s fail: %d %s", sql, mysql_errno(conn), mysql_error(conn));
            sdsfree(sql);
            return -__LINE__;
    }   
    sdsfree(sql);

    MYSQL_RES *result = mysql_store_result(conn);
    size_t num_rows = mysql_num_rows(result);
    init_markets(num_rows);
    for (size_t i = 0; i < num_rows; ++i) 
    {
       MYSQL_ROW row = mysql_fetch_row(result);
       settings.markets[i].sellname = strdup(row[3]); 
       settings.markets[i].buyname = strdup(row[1]); 
       settings.markets[i].sellchinesename = strdup(row[2]); 
       settings.markets[i].buychinesename = strdup(row[0]); 
       // setting.market[i].market = sellname + buyname;
       strncpy(settings.markets[i].market, settings.markets[i].sellname,strlen(settings.markets[i].sellname));
       strncpy(settings.markets[i].market+strlen(settings.markets[i].sellname), 
                settings.markets[i].buyname,strlen(settings.markets[i].buyname));
       if(!settings.markets[i].sellname || !settings.markets[i].buyname || !settings.markets[i].sellchinesename 
            || !settings.markets[i].buychinesename)
       {
            log_error("get order detail of order id: %d fail", i);
            mysql_free_result(result);
            return -__LINE__;
       }
    }
    // DEBUG
    for(int i=0; i<settings.market_num; ++i)
    {
       printf("market pair: %s ,sell: %s , buy: %s , sell_chinese:%s , buy_chinese:%s\n", 
        settings.markets[i].market,settings.markets[i].sellname,settings.markets[i].buyname,
    settings.markets[i].sellchinesename,settings.markets[i].buychinesename);
    }
    printf("market numbers : %d\n",settings.market_num);
    mysql_free_result(result);
    return 0;
}

int init_from_db()
{
    MYSQL *conn = mysql_connect(&settings.db_bitasia);
    if (conn == NULL) 
    {
            log_error("connect mysql fail");
            log_stderr("connect mysql fail");
            return -__LINE__;
    } 
    const char* table1 = "system_coin_type";    
    const char* table2 = "system_trade_type";    
    int ret = load_markets_from_db(conn,table1,table2);
    if(ret < 0)
    {
        goto cleanup;
    }
    mysql_close(conn);
    log_stderr("load sucess");
    return 0;

cleanup:
    mysql_close(conn);
    return ret;
}













