/*
 * Description: 
 *     History: yang@haipo.me, 2017/04/28, create
 */

# ifndef _AW_BAE_STATE_H_
# define _AW_BAE_STATE_H_

int init_bae_state(void);

int bae_state_subscribe(nw_ses *ses, const char *market);
int bae_state_unsubscribe(nw_ses *ses);
int bae_state_send_last(nw_ses *ses, const char *market);

# endif

