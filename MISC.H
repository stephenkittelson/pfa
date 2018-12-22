#include "list.h"

//                  reset_cl & reset_dcl must come before reset_tl
#define reset_all() reset_dcl(); reset_cl(); reset_tl();

void sys_cmd(B8 *str,...);
#ifdef RANDOM_INPUT
	void init_misc(void);
#else
	#define init_misc()
#endif
void print_version(void);
void handle_globalkeys(struct LIST *,B16 keycode,void (*display)(void),B8 last_not_tmplt);
#ifdef DEBUG
	#ifdef RANDOM_INPUT
		void sig_catch(int x);
	#endif
	void dlog_lists(void);
#endif
