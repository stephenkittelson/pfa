#include <myalleg.h>
#include LIBDIR"std.h"
#include "scr.h"
#include "help.h"

extern B8 *help_data[];

void help(void)
{
	B32 c1,num_lines;
	SB32 cur_line;
	B16 keycode;

	#ifdef RANDOM_INPUT
		print_infoline("Cannot access the help section while in random input mode.");
		return;
	#endif
	clear_scr();
	cur_line=0;
	for(num_lines=0;num_lines<0xFFFFFFFFL && help_data[num_lines][0]!=1;num_lines++);
	for(c1=0;c1<50 && c1<num_lines;c1++)
		write_scr(0,c1,help_data[c1]);
	while(1)
	{
		keycode=readkey();
		switch(keycode>>8)
		{
			case KEY_PGDN:
			{
				if(cur_line+49<num_lines)
				{
					clear_scr();
					cur_line+=49;
					for(c1=cur_line;c1-cur_line<50 && c1<num_lines;c1++)
						write_scr(0,c1-cur_line,help_data[c1]);
				}
				break;
			}
			case KEY_PGUP:
			{
				if(cur_line-49>=0)
				{
					clear_scr();
					cur_line-=49;
					for(c1=cur_line;c1-cur_line<50 && c1<num_lines;c1++)
						write_scr(0,c1-cur_line,help_data[c1]);
				}
				break;
			}
			case KEY_ESC:
			{
				return;
			}
			default:
			{
				break;
			}
		}
	}
	return;
}
