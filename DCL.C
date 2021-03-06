#include <myalleg.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "scr.h"
#include "dcl.h"
#include "tl.h"
#include "options.h"
#include "error.h"
#include "misc.h"
#include "io.h"
#include "totals.h"

struct LIST dcl;
extern struct COLOR user_colors;
extern struct LIST cl,tl;
extern B8 modified;
extern struct TOTALS totals;

#define SELECT() set_text_color(0,dcl.sel_num+2,28,user_colors.selected);
#define UNSELECT() set_text_color(0,dcl.sel_num+2,28,user_colors.general);

B8 *dcl_scr=
{
 //00000000001111 
 //01234567890123 $-10000000.00
	"컴컴컴컴컴컴컴쩡컴컴컴컴컴컴                                                    "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"              �                                                                 "
	"컴컴컴컴컴컴컴좔컴컴컴컴컴컴 "
};

void dcl_refresh(void)
{
	struct DC *dc;
	B8 c;

	#ifdef DEBUG
		if(!dcl.top)
			write_logs(0,1,"dcl.c:dcl_refresh:ERROR:dcl.top == NULL\n");
	#endif
	write_scr(0,2,dcl_scr+80);
	dc=(struct DC *)dcl.top;
	for(c=0;c<=dcl.max_sel_num && dc;c++)
	{
		write_scr(0,c+2,dc->name);
		if(dc!=(struct DC *)dcl.first)
		{
			setcurpos(15,c+2);
			printf("$%.02f",dc->amount);
		}
		dc=dc->next;
	}
}

void dcl_display(void)
{
	clear_scr();
	print_version();
	write_scr(0,1,dcl_scr);
	dcl_refresh();
	SELECT();
}

void *dcl_next(void *x)
{
	return (void *)(((struct DC *)x)->next);
}

void **dcl_pnext(void *x)
{
	return (void **)&(((struct DC *)x)->next);
}

void *dcl_prev(void *x)
{
	return (void *)(((struct DC *)x)->prev);
}

void **dcl_pprev(void *x)
{
	return (void **)&(((struct DC *)x)->prev);
}

void dcl_select(void)
{
	SELECT();
}

void dcl_unselect(void)
{
	UNSELECT();
}

void *dcl_make_template(void)
{
	struct DC *tmplt;

	if(!(tmplt=(struct DC *)malloc(sizeof(struct DC))))
	{
		print_infoline("Not enough free memory to add any more debtors/creditors");
		return 0;
	}
	tmplt->id=1;
	strcpy(tmplt->name,"NEW");
	tmplt->tmplt=TRUE;
	tmplt->amount=0;
	return (void *)tmplt;
}

void free_dcl(void)
{
	struct DC *dc,*next;

	dc=(struct DC *)dcl.first;
	while(dc)
	{
		next=dc->next;
		free(dc);
		dc=next;
	}
	dcl.first=dcl.top=dcl.sel=dcl.bot=dcl.last=0;
	dcl.ttb_dist=dcl.sel_num=0;
	dcl.nomore=FALSE;
}

void add_init_dc(void)
{
	struct DC *none;

	if(!(none=(struct DC *)malloc(sizeof(struct DC))))
	{
		error_exit(OUT_OF_MEMORY,EXIT_OUTOFMEM,FALSE);
	}

	none->id=0;
	strcpy(none->name,"no one");
	none->tmplt=FALSE;
	none->amount=0;
	none->prev=0;
	none->next=0;

	dcl.first=dcl.top=dcl.sel=dcl.bot=dcl.last=none;
}

void reset_dcl(void)
{
	struct DC *dc,*next;

	if(dcl.active)
	{
		UNSELECT();
	}
	dcl.top=dcl.sel=dcl.bot=dcl.last=dcl.first;
	dcl.ttb_dist=dcl.sel_num=0;
	dcl.nomore=FALSE;
	dc=((struct DC *)dcl.first)->next;
	if(dc)
	{
		dc->prev->next=0;
		while(dc)
		{
			next=dc->next;
			free(dc);
			dc=next;
		}
	}
	totals.debtbal=0;
}

void dcl_activate(B8 display)
{
	if(display)
		dcl_display();
	else
		dcl_refresh();
}

void edit_dc(void)
{
	struct DC tdc;
	B16 flags;

	memcpy(&tdc,dcl.sel,sizeof(struct DC));
	tdc.tmplt ? (flags=FALSE) : (flags=TRUE);

	normcur();
	print_infoline("Enter name: ");
	while(1)
	{ 
		setcurpos(12,48);
		if(get_string(tdc.name,15,"\n\r","",&flags))
		{
			if(!(IOALT&flags) && !(IODN&flags) && !(IOUP&flags))
			{
				modified=TRUE;
				tdc.tmplt=FALSE;
				memcpy(dcl.sel,&tdc,sizeof(struct DC));
				if(!((struct DC *)dcl.last)->tmplt)
					add_tmplt_to_list(&dcl);
				hidecur();
				clear_keybuf();
				print_infoline("");
				return;
			}
		}
		else
		{
			if(IOESC&flags)
			{
				hidecur();
				clear_keybuf();
				print_infoline("");
				return;
			}
		}
		flags=TRUE;
	}
}

B8 dcl_handle_input(struct LIST **current_list,B16 keycode)
{
	struct TRANS *t;
	static B8 tutor_num=0;

	switch(keycode>>8)
	{
		case KEY_ENTER:
		case KEY_ENTER_PAD:
		{
			if(dcl.sel==dcl.first)
				return FALSE;
			clear_keybuf();
			edit_dc();
			dcl_refresh();
			return FALSE;
		}
		case KEY_DEL_PAD:
		case KEY_DEL:
		{
			if(dcl.sel==dcl.first)
			{
				print_infoline("Cannot delete this transactee!");
				return FALSE;
			}
			if(dcl.sel==dcl.last)
			{
				if( ((struct DC *)dcl.last)->tmplt )
				{
					print_infoline("Cannot delete this transactee!");
					return FALSE;
				}
			}
			t=(struct TRANS *)tl.first;
			while(t)
			{
				if((void *)(t->dc)==dcl.sel)
					t->dc=(struct DC *)dcl.first;
				t=t->next;
			}
			del_sel(&dcl);
			modified=TRUE;
			return FALSE;
		}
		//edit cl list
		case KEY_F3:
		{
			(*current_list)=&cl;
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
					" transactee. Press the delete key to  delete the selected"
					" transactee.");
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
					" device. Press the 'f' key to  clear out all the transactee data."
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
					print_infoline("Press the insert key to insert a new transactee"
					" above the selected one.");
					tutor_num++;
					break;
				}
				case 9:
				{
					print_infoline("Press the 'F1' key to display the help screen. Press"
					" the 'F2' key to edit the   options for PFA. Press the 'F3' key to"
					" edit the category list.");
					tutor_num++;
					break;
				}
				case 10:
				{
					print_infoline("Press 'F5' to edit the transaction list. Press 'F6'"
					" to receive instructions for the current screen. End of the"
					" tutorial for transactee screen.");
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
						write_logs(0,1,"dcl.c:dcl_handle_input:default hit on switch(tutor_num)!\n");
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
				t->dc=(struct DC *)dcl.first;
				t=t->next;
			}
			reset_dcl();
			add_tmplt_to_list(&dcl);
			dcl_refresh();
			SELECT();
			modified=TRUE;
			print_infoline("Transactee data released");
			return FALSE;
		}
		//insert tmplt before sel
		case KEY_INSERT:
		{
			if(dcl.sel==dcl.first)
				return FALSE;
			modified=TRUE;
			insert_tmplt_above_sel(&dcl);
			return FALSE;
		}
		default:
		{
			handle_globalkeys(&dcl,keycode,dcl_display,!((struct DC *)dcl.last)->tmplt);
			return FALSE;
		}
	}
}

void init_dcl(B8 limited_run)
{
	dcl.first=
	dcl.top=
	dcl.sel=
	dcl.bot=
	dcl.last=0;

	if(!limited_run)
	{
		dcl.next=dcl_next;
		dcl.pnext=dcl_pnext;
		dcl.prev=dcl_prev;
		dcl.pprev=dcl_pprev;

		dcl.select=dcl_select;
		dcl.unselect=dcl_unselect;
		dcl.refresh=dcl_refresh;
		dcl.make_template=dcl_make_template;
		dcl.activate=dcl_activate;

		dcl.handle_input=dcl_handle_input;

		dcl.sel_num=0;
		dcl.max_sel_num=44;
		dcl.ttb_dist=0;
		dcl.active=FALSE;
		dcl.nomore=FALSE;
	}
	atexit(free_dcl);
	add_init_dc();
	if(!limited_run)
	{
		add_tmplt_to_list(&dcl);
	}
}
