# ifndef _AW_KLINE_H_
# define _AW_KLINE_H_

# include "nw_svr.h"
# include "nw_clt.h"
# include "nw_job.h"
# include "nw_timer.h"
# include "nw_state.h"

# include "ut_log.h"
# include "ut_sds.h"
# include "ut_cli.h"
# include "ut_misc.h"
# include "ut_list.h"
# include "ut_kafka.h"
# include "ut_signal.h"
# include "ut_config.h"
# include "ut_decimal.h"
# include "ut_rpc_clt.h"
# include "ut_rpc_svr.h"
# include "ut_rpc_cmd.h"
# include "ut_ws_svr.h"


int init_rates(void);

int rates_subscribe(nw_ses *ses);
int rates_unsubscribe(nw_ses *ses);

int rates_on_update(json_t* msg);

# endif

