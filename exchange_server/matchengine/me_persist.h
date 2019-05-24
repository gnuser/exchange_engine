/*
 * Description: 
 *     History: yang@haipo.me, 2017/04/04, create
 */

# ifndef _ME_PERSIST_H_
# define _ME_PERSIST_H_

# include <time.h>

int init_persist(void);

int init_from_db(void);
int dump_to_db(time_t timestamp);
int make_slice(time_t timestamp);
int clear_slice(time_t timestamp);

int init_asset_from_db(MYSQL *conn);
int init_market_from_db(MYSQL *conn);
int init_asset_and_market(bool market);

int insert_asset_to_db(const char *asset);
int insert_market_to_db(uint32_t buy_id, uint32_t sell_id, double min_count);

// NOTE:主动冻结的资产总量
mpd_t *get_user_freeze_balance(uint32_t user_id, const char *asset, int prec);

# endif

