#include <myalleg.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "cl.h"
#include "scr.h"
#include "options.h"
#include "io.h"
#include "help.h"
#include "error.h"
#include "tl.h"
#include "totals.h"
#include "misc.h"
#include LIBDIR"flags.h"

extern struct COLOR user_colors;
extern struct TOTALS totals;
extern B8 modified;

double total;

struct LIST cl;
extern struct LIST dcl,tl;

B8 *categscr=
{
 //000000000011111111112222222222333333333344444444445
 //012345678901234567890123456789012345678901234567890
 //X 12345678901234 $-10000000.00 $-10000000.00 100.00%
	"컫컴컴컴컴컴컴컴쩡컴컴컴컴컴컴쩡컴컴컴컴컴컴쩡컴컴컴                            "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	" �              �             �             �                                   "
	"컨컴컴컴컴컴컴컴좔컴컴컴컴컴컴좔컴컴컴컴컴컴좔컴컴컴"
};

#define SELECT() set_text_color(0,cl.sel_num+2,52,user_colors.selected);
#define UNSELECT() set_text_color(0,cl.sel_num+2,52,user_colors.general);

#define CLSEL ((struct CATEG *)cl.sel)
#define CLFIRST ((struct CATEG *)cl.first)

void cl_refresh(void)
{
	struct CATEG *categ;
	B8 buf[13];
	SB8 c;

	write_scr(0,2,categscr+80);
	categ=(struct CATEG *)cl.top;
	for(c=2;c<=(cl.max_sel_num+2) && categ;c++,categ=categ->next)
	{
		//000000000011111111112222222222333333333344444444445
		//012345678901234567890123456789012345678901234567890
		//X 12345678901234 $-10000000.00 $-10000000.00 100.00%
		write_scr(0,c,(categ->sel)?"X":" ");
		write_scr(2,c,categ->name);
		setcurpos(17,c);
		printf("$%.02f",categ->value);
		if(categ->tmplt || categ->id==1 || categ->id==2)
			continue;
		if(categ->limit>0.005 || categ->limit<-0.005)
		{
			setcurpos(31,c);
			printf("$%.02f",categ->limit);
		}
		if(categ->sel)
			sprintf(buf,"%.02f%%  ",(float)(((categ->value*-1)/total)*100));
		else 
			strcpy(buf,"        ");
		write_scr(45,c,buf);
	}
}

void cl_display(void)
{
	clear_scr();
	print_version();
	write_scr(0,1,categscr);
	cl_refresh();
	SELECT();
}

void cl_select(void)
{
	SELECT();
}

void cl_unselect(void)
{
	UNSELECT();
}

void *cl_next(void *x)
{
	return (void *)((struct CATEG *)x)->next;
}

void **cl_pnext(void *x)
{
	return (void **)&((struct CATEG *)x)->next;
}

void *cl_prev(void *x)
{
	return (void *)((struct CATEG *)x)->prev;
}

void **cl_pprev(void *x)
{
	return (void **)&((struct CATEG *)x)->prev;
}

void *cl_make_template(void)
{
	struct CATEG *categ;

	if(!(categ=(struct CATEG *)malloc(sizeof(struct CATEG))))
	{
		print_infoline("Not enough memory to allow any more categories to be added");
		return 0;
	}
	categ->id=5;
	strcpy(categ->name,"NEW");
	categ->limit=0;
	categ->value=0;
	categ->tmplt=TRUE;
	categ->sel=FALSE;
	return (void *)categ;
}

void refresh_percents(void)
{
	struct CATEG *categ;
	B8 c;
	B8 buf[11];

	categ=(struct CATEG *)cl.top;
	for(c=2;c<=(cl.max_sel_num+2) && categ;c++,categ=categ->next)
	{
		if(categ->sel)
			sprintf(buf,"%.02f%%  ",(float)(((categ->value*-1)/total)*100));
		else 
			strcpy(buf,"        ");
		write_scr(45,c,buf);
	}
}

void cl_activate(B8 display)
{
	struct CATEG *categ;

	//todo when I have a working version again, check to see if this will work
	//total=totals.nt_expense;
	total=0;
	categ=(struct CATEG *)cl.first;
	for(;categ;categ=categ->next)
	{
		if(categ->tmplt || categ->id==1 || categ->id==2 || categ->value>0)
		{
			categ->sel=FALSE;
		}
		else
		{
			categ->sel=TRUE;
			total+=categ->value;
		}
	}
	if(display)
		cl_display();
	else
		cl_refresh();
}

B8 cl_handle_input(struct LIST **current_list,B16 keycode)
{
	struct TRANS *t;
	static B8 tutor_num=0;

	switch(keycode>>8)
	{
		case KEY_ENTER:
		case KEY_ENTER_PAD:
		{
			if(CLSEL->id<3)
				return FALSE;
			edit_categ();
			clear_keybuf();
			hidecur();
			print_infoline("");
			return FALSE;
		}
		case KEY_DEL_PAD:
		case KEY_DEL:
		{
			#define CLFIRST ((struct CATEG *)cl.first)

			if(cl.sel==cl.first || CLSEL==CLFIRST->next || 
				CLSEL==CLFIRST->next->next)
			{
				print_infoline("Cannot delete this category!");
				return FALSE;
			}
			if(cl.sel==cl.last)
			{
				if( ((struct CATEG *)cl.last)->tmplt )
				{
					print_infoline("Cannot delete this category!");
					return FALSE;
				}
			}
			t=(struct TRANS *)tl.first;
			while(t)
			{
				if((void *)(t->categ)==cl.sel)
					t->categ=(struct CATEG *)cl.first;
				t=t->next;
			}
			del_sel(&cl);
			return FALSE;
		}
		//edit dc list
		case KEY_F4:
		{
			(*current_list)=&dcl;
			return TRUE;
		}
		//edit transaction list
		case KEY_F5:
		{
			(*current_list)=&tl;
			return TRUE;
		}
		//tutorial
		case KEY_F6:
		{
			switch(tutor_num)
			{
				case 0:
				{
					print_infoline("Press either the 'up' or 'down' arrow keys to change"
					" the current selection by 1 item. Press 'F6' to display the next"
					" set of instructions for the current screen");
					tutor_num++;
					break;
				}
				case 1:
				{
					print_infoline("Press the 'enter' key to edit the selected"
					" category. Press the delete key to    delete the selected category.");
					tutor_num++;
					break;
				}
				case 2:
				{
					print_infoline("Press the 'w' key to write all your current data to"
					" a file on the storage device(also known as the \"C: drive\" or"
					" \"hard drive\")");
					tutor_num++;
					break;
				}
				case 3:
				{
					print_infoline("Press the 'l' key to load a file from the storage"
					" device. Press the 'f' key to  clear out all the category data."
					);
					tutor_num++;
					break;
				}
				case 4:
				{
					print_infoline("Hold down the 'alt' key and press the 'up' arrow key"
					" or 'down' arrow key to movethe current selection by 10 items.");
					tutor_num++;
					break;
				}
				case 5:
				{
					print_infoline("Press the 'home' key to select the first item on the"
					" screen. Press the 'end' keyto select the last item on the screen."
					);
					tutor_num++;
					break;
				}
				case 6:
				{
					print_infoline("Hold down 'alt' and press the 'home' key to select"
					" the very first item, or the  'end' key to select the very last"
					" item.");
					tutor_num++;
					break;
				}
				case 7:
				{
					print_infoline("Press the page up key to move up a page. Press the"
					" page down key to move down a page. Press the 'c' key to invoke the"
					" calculator."
					);
					tutor_num++;
					break;
				}
				case 8:
				{
					print_infoline("Press the insert key to insert a new category"
					" above the selected one. Press     spacebar to toggle the"
					" inclusion of the current category in the expense report.");
					tutor_num++;
					break;
				}
				case 9:
				{
					print_infoline("Press the 'F1' key to display the help screen. Press"
					" the 'F2' key to edit the   options for PFA. Press the 'F4' key to"
					" edit the transactee list.");
					tutor_num++;
					break;
				}
				case 10:
				{
					print_infoline("Press 'F5' to edit the transaction list. Press 'F6'"
					" to receive instructions for the current screen. End of the"
					" tutorial for category screen.");
					tutor_num++;
					break;
				}
				case 11:
				{
					print_infoline("");
					tutor_num=0;
					break;
				}
				default:
				{
					#ifdef DEBUG
						write_logs(0,1,"cl.c:cl_handle_input:default hit on switch(tutor_num)!\n");
					#endif
					tutor_num=0;
					break;
				}
			}
			return FALSE;
		}
		//free data without writing to hard disk
		case KEY_F:
		{
			clear_keybuf();
			if(modified)
			{
				print_infoline("Are you sure you want to continue without saving? ");
				normcur();
				if(readkey()>>8!=KEY_Y)
				{
					hidecur();
					return FALSE;
				}
				hidecur();
			}
			t=(struct TRANS *)tl.first;
			while(t)
			{
				if(t->categ->id>2)
					t->categ=(struct CATEG *)cl.first;
				t=t->next;
			}
			reset_cl();
			add_tmplt_to_list(&cl);
			cl_refresh();
			SELECT();
			modified=TRUE;
			print_infoline("Category data released");
			return FALSE;
		}
		//toggle inclusion of selection in percentage report
		case KEY_SPACE:
		{
			if(CLSEL->tmplt || CLSEL->id==1 || CLSEL->id==2 || CLSEL->value>0)
			{
				return FALSE;
			}
			((struct CATEG *)cl.sel)->sel^=TRUE;
			if(((struct CATEG *)cl.sel)->sel)
			{
				write_scr(0,cl.sel_num+2,"X");
				total-=((struct CATEG *)cl.sel)->value;
			}
			else
			{
				write_scr(0,cl.sel_num+2," ");
				total+=((struct CATEG *)cl.sel)->value;
			}
			refresh_percents();
			return FALSE;
		}
		//insert tmplt before sel
		case KEY_INSERT:
		{
			if(cl.sel==cl.first || cl.sel==CLFIRST->next ||
					cl.sel==CLFIRST->next->next)
				return FALSE;
			modified=TRUE;
			insert_tmplt_above_sel(&cl);
			return FALSE;
		}
		default:
		{
			handle_globalkeys(&cl,keycode,cl_display,!((struct CATEG *)cl.last)->tmplt);
			return FALSE;
		}
	}
}

void edit_categ(void)
{
	B8 buf[13];
	B16 flags;
	struct CATEG tc;

	#define REFRESH() \
		write_scr(INLOC_NAMEX,INLOC_NAMEY,tc.name);\
		sprintf(buf,"%.02f",tc.limit);\
		write_scr(INLOC_LIMITX,INLOC_LIMITY,buf);

	memcpy(&tc,cl.sel,sizeof(struct CATEG));
	if(tc.tmplt)
		tc.name[0]=0;
	//                   000000000011111
	//                   012345678901234        -10000000.00
	//             000000000011111111112222222222
	//             012345678901234567890123456789
	print_infoline("Name:                 Limit:");
	#define INLOC_NAMEX 6
	#define INLOC_NAMEY 48
	#define INLOC_LIMITX 29
	#define INLOC_LIMITY 48
	write_scr(INLOC_NAMEX,INLOC_NAMEY,tc.name);
	setcurpos(INLOC_LIMITX,INLOC_LIMITY);
	printf("%.02f",tc.limit);

	clear_keybuf();
	normcur();
	while(1)
	{
		//name
		while(1)
		{ 
			setcurpos(INLOC_NAMEX,INLOC_NAMEY);
			flags=TRUE;
			if(get_string(tc.name,15,"\n\r","",&flags))
			{
				if(IOALT&flags)
				{
					write_scr(0,49,"Alt-");
					while(key[KEY_ALT])
					{
						if(key[KEY_Q])
						{
							clear_keybuf();
							tc.tmplt=FALSE;
							memcpy(cl.sel,&tc,sizeof(struct CATEG));
							if(cl.sel==cl.last)
							{
								add_tmplt_to_list(&cl);
							}
							cl_refresh();
							print_infoline("");
							modified=TRUE;
							return;
						}
					}
					write_scr(0,49,"    ");
				}
				break;
			}
			else
			{
				if(IOF1&flags)
				{
					help();
					cl_display();
					REFRESH();
					print_infoline("Name:                 Limit:");
				}
				else if(IOESC&flags)
				{
					return;
				}
			}
		}
		//limit
		while(1)
		{
			setcurpos(INLOC_LIMITX,INLOC_LIMITY);
			flags=TRUE;
			if(get_float(&(tc.limit),MIN_AMOUNT,MAX_AMOUNT,MAX_BDDIGITS,MAX_ADDIGITS,"\n\r",&flags,"%.02f"))
			{
				if(IOALT&flags)
				{
					write_scr(0,49,"Alt-");
					while(key[KEY_ALT])
					{
						if(key[KEY_Q])
						{
							clear_keybuf();
							tc.tmplt=FALSE;
							memcpy(cl.sel,&tc,sizeof(struct CATEG));
							if(cl.sel==cl.last)
							{
								add_tmplt_to_list(&cl);
							}
							cl_refresh();
							print_infoline("");
							modified=TRUE;
							return;
						}
					}
					write_scr(0,49,"    ");
				}
				break;
			}
			else
			{
				if(IOF1&flags)
				{
					help();
					cl_display();
					REFRESH();
					print_infoline("Name:                 Limit:");
				}
				else if(IOESC&flags)
				{
					return;
				}
			}
		}
	}
}

void reset_cl(void)
{
	struct CATEG *categ,*next;

	if(cl.active)
	{
		UNSELECT();
	}
	cl.top=cl.sel=cl.first;
	cl.bot=cl.last=((struct CATEG *)cl.first)->next->next;
	cl.ttb_dist=2;
	cl.sel_num=0;
	cl.nomore=FALSE;
	categ=(struct CATEG *)cl.first;
	//it is faster not to put this in a loop that only loops 3 times anyway
	categ->value=0;
	categ=categ->next;
	categ->value=0;
	categ=categ->next;
	categ->value=0;
	categ=categ->next;
	if(categ)
	{
		categ->prev->next=0;
		while(categ)
		{
			next=categ->next;
			free(categ);
			categ=next;
		}
	}
}

void add_init_categs(void)
{
	struct CATEG *other,*tith,*income;

	/*
		these malloc's should never fail except at startup, and we won't have any 
		data to save then
	*/
	if(!(other=(struct CATEG *)malloc(sizeof(struct CATEG))))
	{
		error_exit(OUT_OF_MEMORY,EXIT_OUTOFMEM,FALSE);
	}
	if(!(tith=(struct CATEG *)malloc(sizeof(struct CATEG))))
	{
		error_exit(OUT_OF_MEMORY,EXIT_OUTOFMEM,FALSE);
	}
	if(!(income=(struct CATEG *)malloc(sizeof(struct CATEG))))
	{
		error_exit(OUT_OF_MEMORY,EXIT_OUTOFMEM,FALSE);
	}
	cl.sel=cl.first=cl.top=(void *)other;

	other->id=0;
	tith->id=1;
	income->id=2;

	strcpy(other->name,"Other");
	strcpy(tith->name,"Tithing");
	strcpy(income->name,"Income");

	other->limit=
	tith->limit=
	income->limit=0;

	other->value=
	tith->value=
	income->value=0;

	other->tmplt=
	tith->tmplt=
	income->tmplt=FALSE;

	other->sel=TRUE;
	tith->sel=
	income->sel=FALSE;

	other->next=tith;
	tith->next=income;
	income->next=0;

	other->prev=0;
	tith->prev=other;
	income->prev=tith;

	cl.last=cl.bot=(void *)income;
	cl.ttb_dist=2;
}

void free_cl(void)
{
	struct CATEG *c,*next;

	c=(struct CATEG *)cl.first;
	while(c)
	{
		next=c->next;
		free(c);
		c=next;
	}
	cl.first=cl.top=cl.sel=cl.bot=cl.last=0;
	cl.ttb_dist=cl.sel_num=0;
	cl.nomore=FALSE;
}

void init_cl(B8 limited_run)
{
	cl.first=
	cl.top=
	cl.sel=
	cl.bot=
	cl.last=0;

	if(!limited_run)
	{
		cl.next=cl_next;
		cl.pnext=cl_pnext;
		cl.prev=cl_prev;
		cl.pprev=cl_pprev;

		cl.select=cl_select;
		cl.unselect=cl_unselect;
		cl.refresh=cl_refresh;
		cl.make_template=cl_make_template;
		cl.activate=cl_activate;

		cl.handle_input=cl_handle_input;

		cl.sel_num=0;
		cl.max_sel_num=44;
		cl.ttb_dist=0;
		cl.active=FALSE;
		cl.nomore=FALSE;
	}

	atexit(free_cl);
	add_init_categs();
	if(!limited_run)
	{
		add_tmplt_to_list(&cl);
	}
}
