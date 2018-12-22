#include <myalleg.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "options.h"
#include "scr.h"
#include "tl.h"
#include "cl.h"
#include "dcl.h"
#include "misc.h"
#ifdef F_CL_TITHCHK
	#include "file.h"
	#include "totals.h"
#endif
#ifdef DEBUG
	#include "io.h"
#endif

B8 modified;

extern struct LIST tl;
#ifdef F_CL_TITHCHK
	extern struct TOTALS totals;
#endif

B8 *_PFA_idstr={"[ PFA by Stephen Kittelson. Version " VSTR ". Built on " __DATE__ " ]"};

#ifdef F_CL_TITHCHK
	int main(int argc,char *argv[])
#else
	int main(void)
#endif
{
	//#define DEBUG_GETFLOAT
	struct LIST *current_list,*last_list;
	B16 keycode;
	#ifdef DEBUG
		B8 valid_ttbdist=TRUE;
		#ifdef DEBUG_GETFLOAT
			double val=0;
			B16 flags=0;
		#endif
	#endif

	#ifdef DEBUG
			printf("We are in debug mode\n");
		init_logs(2);
		write_logs(0,1,"Personal Financial Assistant " VSTR "\n\n");
	#endif
	#ifdef F_CL_TITHCHK
		if(argc==1)
		{
		}
		else if(argc==3)
		{
			if(argv[1][0]=='-')
			{
				if(argv[1][1]=='t')
				{
					init_options();
					init_dcl(TRUE);
					init_cl(TRUE);
					init_tl(TRUE);
					if(!_load_file(argv[2],TRUE))
					{
						return EXIT_INVFILE;
					}
					printf("\nAmount of tithing owed: %.02f",totals.tith_owed);
					if(totals.tith_owed>0.004)
					{
						#ifdef DEBUG
							printf("\nReturning tithing owed (%u)\n",EXIT_TITHOWED);
						#endif
						return EXIT_TITHOWED;
					}
					else
					{
						#ifdef DEBUG
							printf("\nReturning NO tithing owed (%u)\n",EXIT_NOTITHOWED);
						#endif
						return EXIT_NOTITHOWED;
					}
				}
				else
				{
					goto invcmdln;
				}
			}
			else
			{
				goto invcmdln;
			}
		}
		else
		{
			invcmdln:
			printf("Invalid command line.\n"
				"Usage: pfa.exe [-t filename]\n"
				" t - check to see if there is tithing to be paid in the PFA data file\n"
				"     'filename'. If so, returns %u, else %u.\n"
				,EXIT_TITHOWED,EXIT_NORMAL);
			return EXIT_INVCMDLN;
		}
	#endif
	allegro_init();
	install_keyboard();

	hidecur();
	init_scr();
	init_options();
	init_misc();
	init_dcl(FALSE);
	init_cl(FALSE);
	init_tl(FALSE);

	#ifdef DEBUG_GETFLOAT
		write_scr(0,1,"-12345678.12");
		get_floatlbl:
		write_scr(0,0,"            ");
		setcurpos(0,0);
		get_float(&val,MIN_AMOUNT,MAX_AMOUNT,MAX_BDDIGITS,MAX_ADDIGITS,"\n\r",&flags,"%.02f");
		goto get_floatlbl;
		exit(0);
	#endif
	last_list=current_list=&tl;
	current_list->active=TRUE;
	current_list->activate(TRUE);
	#ifdef BETA
		print_infoline("BETA VERSION: DO NOT DISTRIBUTE!! Press the 'F6' key to activate the tutorial.");
	#else
		print_infoline("Press the 'F6' key to activate the tutorial.");
	#endif
	while(1)
	{
		#ifdef DEBUG
			if(valid_ttbdist && current_list->ttb_dist<current_list->max_sel_num && current_list->bot!=current_list->last)
			{
				write_logs(0,1,"main.c:ttb_dist not valid\n");
				valid_ttbdist=FALSE;
			}
		#endif
		keycode=readkey();
		handle_geninput(current_list,keycode);
		if(current_list->handle_input(&current_list,keycode))
		{
			last_list->active=FALSE;
			last_list=current_list;
			current_list->active=TRUE;
			current_list->activate(TRUE);
		}
	}
	//this will never be reached, just here to avoid a warning
	return EXIT_NORMAL;
}
