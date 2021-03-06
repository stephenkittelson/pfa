#include <time.h>
#include <myalleg.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "tl.h"
#include "scr.h"
#include "options.h"
#include "calc.h"
#include "help.h"
#include "cl.h"
#include "dcl.h"
#include "io.h"
#include "file.h"
#include "search.h"
#include "totals.h"
#include "misc.h"
#include "error.h"
#include LIBDIR"flags.h"

struct LIST tl;

extern struct TOTALS totals;
extern struct OPTIONS options;
extern struct COLOR user_colors;
extern struct LIST cl,dcl;
extern B8 modified;

B8 *tlscr=
{
	/*          0000000000111111111122222222223333333333444444444455
	 2000-12-31 0123456789012345678901234567890123456789012345678901 $-999,999,999.99
				Personal Financial Assistant (Version XXX.XXX)
	*/
	"컴컴컴컴컴쩡컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴컴컴"
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"    -  -  �                                                    �$               "
	"컴컴컴컴컴좔컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컨컴컴컴컴컴컴컴컴 "
};

#define SELECT() set_text_color(0,tl.sel_num+2,80,user_colors.selected);
#define UNSELECT() set_text_color(0,tl.sel_num+2,80,user_colors.general);

//prototypes
B8 edit_trans(struct TRANS *sel);

void tl_refresh(void)
{
	B8 y=3;
	struct TRANS *t;

	//clear trans list
	write_scr(0,2,(tlscr+80));
	if(!tl.first)
		return;
	if(!tl.top)
	{
		#ifdef DEBUG
			write_logs(0,1,"tl_refresh: tl.top wasn't inited\n");
		#endif
		tl.top=tl.first;
	}
	//display trans list
	y=2;
	t=(struct TRANS *)tl.top;
	while(t)
	{
		setcurpos(0,y);
		printf("%04u-%02u-%02u�%s",t->year,t->month,t->day,t->memo);
		setcurpos(65,y);
		printf("%.02f",t->value);
		if((void *)t==tl.bot)
			break;
		y++;
		t=t->next;
	}
}

void tl_display(void)
{
	clear_scr();
	write_scr(0,1,tlscr);
	print_version();
	SELECT();
	tl_refresh();
}

void *tl_next(void *x)
{
	return (void *)(((struct TRANS *)x)->next);
}

void **tl_pnext(void *x)
{
	return (void **)&(((struct TRANS *)x)->next);
}

void *tl_prev(void *x)
{
	return (void *)(((struct TRANS *)x)->prev);
}

void **tl_pprev(void *x)
{
	return (void **)&(((struct TRANS *)x)->prev);
}

void tl_select(void)
{
	SELECT();
}

void tl_unselect(void)
{
	UNSELECT();
}

void *tl_make_template(void)
{
	struct TRANS *t;
	time_t timet;
	struct tm *timetm;

	if(!(t=(struct TRANS *)malloc(sizeof(struct TRANS))))
	{
		print_infoline("Not enough memory to add another transaction");
		return 0;
	}
	time(&timet);
	timetm=gmtime(&timet);

	t->year=timetm->tm_year+1900;
	t->month=timetm->tm_mon+1;
	t->day=timetm->tm_mday;
	strcpy(t->memo,"NEW");
	t->per_to_tith=0;
	t->flags=TEMPLATE;
	t->gross_income=0;
	t->value=0;
	t->categ_id=5;
	t->categ=(struct CATEG *)cl.first;
	t->dc_id=5;
	t->dc=(struct DC *)dcl.first;
	return (void *)t;
}

void tl_activate(B8 display)
{
	if(display)
		tl_display();
	else
		tl_refresh();
}

B8 tl_handle_input(struct LIST **current_list,B16 keycode)
{
	static B8 tutor_num=0;

	switch(keycode>>8)
	{
		case KEY_ENTER:
		case KEY_ENTER_PAD:
		{
			if(tl.sel)
			{
				tl.active=FALSE;
				if(edit_trans((struct TRANS *)(tl.sel)))
					modified=TRUE;
				hidecur();
				tl.active=TRUE;
				tl_display();
			}
			return FALSE;
		}
		//edit categ list
		case KEY_F3:
		{
			*current_list=&cl;
			return TRUE;
		}
		//edit dc list
		case KEY_F4:
		{
			*current_list=&dcl;
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
					" transaction. Press the 't' key to    view the totals. Press the"
					" delete key to delete the selected transaction.");
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
					" device. Press the 'f' key to  clear out all the transaction data."
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
					print_infoline("Press the insert key to insert a new transaction"
					" above the selected one. Press  the 's' key to search the memos of"
					" the transactions.");
					tutor_num++;
					break;
				}
				case 9:
				{
					print_infoline("While in the memo search, hold down 'alt' and press"
					" the 'c' key to search for   transactions with a certain category."
					);
					tutor_num++;
					break;
				}
				case 10:
				{
					print_infoline("While in the memo search, hold down 'alt' and press"
					" the 'f' key to search for   transactions with a certain pattern of"
					" flags (Deposited, Bank Verified, etc.)");
					tutor_num++;
					break;
				}
				case 11:
				{
					print_infoline("While in the memo search, hold down 'alt' and press"
					" the 't' key to search for   transactions with a certain"
					" transactee.");
					tutor_num++;
					break;
				}
				case 12:
				{
					print_infoline("Press the 'F1' key to display the help screen. Press"
					" the 'F2' key to edit the   options for PFA. Press the 'F3' key to"
					" edit the category list.");
					tutor_num++;
					break;
				}
				case 13:
				{
					print_infoline("Press 'F4' to edit the transactee list. Press 'F6'"
					" to receive instructions for  the current screen. End of the"
					" tutorial for transaction screen.");
					tutor_num++;
					break;
				}
				case 14:
				{
					print_infoline("");
					tutor_num=0;
					break;
				}
				default:
				{
					#ifdef DEBUG
						write_logs(0,1,"tl.c:tl_handle_input:default hit on switch(tutor_num)!\n");
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
			reset_tl();
			add_tmplt_to_list(&tl);
			tl_refresh();
			SELECT();
			modified=TRUE;
			print_infoline("Transaction data released");
			return FALSE;
		}
		//delete selected trans
		case KEY_DEL_PAD:
		case KEY_DEL:
		{
			if(tl.sel==tl.last)
			{
				if(TEMPLATE&(((struct TRANS *)tl.sel)->flags))
				{
					print_infoline("Cannot delete this transaction!");
					return FALSE;
				}
			}
			sub_from_totals((struct TRANS *)tl.sel);
			del_sel(&tl);
			modified=TRUE;
			return FALSE;
		}
		//search memos
		case KEY_S:
		{
			search_tl_memo();
			return FALSE;
		}
		//print data
		case KEY_P:
		{
			print_tl();
			return FALSE;
		}
		default:
		{
			handle_globalkeys(&tl,keycode,tl_display,!(TEMPLATE&(((struct TRANS *)tl.last)->flags)));
			return FALSE;
		}
	}
}

void init_tl(B8 limited_run)
{
	tl.first=
	tl.top=
	tl.sel=
	tl.bot=
	tl.last=0;

	if(!limited_run)
	{
		tl.next=tl_next;
		tl.pnext=tl_pnext;
		tl.prev=tl_prev;
		tl.pprev=tl_pprev;

		tl.select=tl_select;
		tl.unselect=tl_unselect;
		tl.refresh=tl_refresh;
		tl.make_template=tl_make_template;
		tl.activate=tl_activate;

		tl.handle_input=tl_handle_input;

		tl.sel_num=0;
		tl.max_sel_num=44;
		tl.ttb_dist=0;
		tl.active=FALSE;
		tl.nomore=FALSE;
	}

	atexit(free_tl);
	if(!limited_run)
	{
		modified=FALSE;
		add_tmplt_to_list(&tl);
	}
	reset_totals();
	if(!limited_run)
	{
		if(options.sfn[0]!=0)
		{
			if(!load_file(options.sfn))
			{
				clear_keybuf();
				readkey();
			}
		}
	}
}

void free_tl(void)
{
	struct TRANS *t,*next;

	t=(struct TRANS *)tl.first;
	while(t)
	{
		next=t->next;
		free(t);
		t=next;
	}
	tl.first=tl.top=tl.sel=tl.bot=tl.last=0;
	tl.ttb_dist=tl.sel_num=0;
	return;
}

void reset_tl(void)
{
	if(tl.active)
	{
		UNSELECT();
	}
	free_tl();
	reset_totals();
}

B8 *etscr=
{
	"Date:            Memo:                                                          "
	"Percent to tithing:       Gross income:               Net amount:               "
	"                                                                                "
	"Category:                                                                       "
	"Transactee:                                                                     "
	"                                                                                "
	"[ ] Deposited                                                                   "
	"[ ] Bank has verified                                                           "
	"                                                                                "
	"TAX                                                                             "
	"                                                                                "
	"[ ] Earned interest                                                             "
	"[ ] Tax deductable                                                              "
	"[ ] Taxable income "
};

B8 *etscr_tutorial=
{
	"To switch between input locations, you can either press the 'enter' key, or hold"
	"down the 'alt' key and press the letter that corresponds to the highlighted     "
	"letter of the input location you wish to go to.                                 "
	"                                                                                "
	"To choose a category/transactee, you can either press the 'up' or 'down' arrows "
	"until the item you want is displayed, or begin typing in the name of the item   "
	"you want until it is displayed (hereafter refered to as 'type-search'). The     "
	"type-search is always case-sensitive.                                           "
	"                                                                                "
	"To mark or unmark one of the flags (Deposited, Bank has Verified, etc.), press  "
	"the space-bar when you are at that input location.                              "
	"                                                                                "
	"To accept the changes you have made to the transaction and return to the trans- "
	"action listing, hold down the 'alt' key and press 'q'.                          "
	"                                                                                "
	"To reject the changes you have made and return to the transaction listing, press"
	"the escape key.                                                                 "
	"                                                                                "
	"To disable the displaying of this tutorial message, turn tutor off on the       "
	"options screen."
};

// misc defs
	#define MAX_INLOCNUM 13
//input locations for etscr
	#define SCR_ET_DATEX 6
	#define SCR_ET_DATEY 0
	#define SCR_ET_MEMOX 23
	#define SCR_ET_MEMOY 0
	#define SCR_ET_PERTOTITHX 20
	#define SCR_ET_PERTOTITHY 1
	#define SCR_ET_GIX 40
	#define SCR_ET_GIY 1
	#define SCR_ET_VALUEX 66
	#define SCR_ET_VALUEY 1
	#define SCR_ET_CATEGX 10
	#define SCR_ET_CATEGY 3
	#define SCR_ET_DCX 12
	#define SCR_ET_DCY 4
	#define SCR_ET_GENFLAGSY 6
	#define SCR_ET_TAXFLAGSY 11

// flag defs
	#define PTT   BIT_0
	#define GI    BIT_1
	#define NV    BIT_2

void display_etscr(struct TRANS *sel,const B8 *dispnotflags)
{
	clear_scr();
	write_scr(0,0,etscr);

	HIGHLIGHT(20,0 ); //memO
	HIGHLIGHT(0 ,1 ); //Percent to tithing
	HIGHLIGHT(26,1 ); //Gross income
	HIGHLIGHT(58,1 ); //net Value
	HIGHLIGHT(0 ,3 ); //Category
	HIGHLIGHT(0 ,4 ); //Transactee
	HIGHLIGHT(5 ,6 ); //dEposited
	HIGHLIGHT(4 ,7 ); //Bank has verified
	HIGHLIGHT(6 ,11); //eaRned interest
	HIGHLIGHT(11,12); //tax dedUctable
	HIGHLIGHT(13,13); //taxable iNcome

	setcurpos(SCR_ET_DATEX,SCR_ET_DATEY);
	printf("%u %u",sel->year,sel->month);
	setcurpos(SCR_ET_DATEX+8,SCR_ET_DATEY);
	printf("%u",sel->day);
	setcurpos(SCR_ET_MEMOX,SCR_ET_MEMOY);
	printf("%s",sel->memo);
	if(!((*dispnotflags)&PTT))
	{
		setcurpos(SCR_ET_PERTOTITHX,SCR_ET_PERTOTITHY);
		printf("%u",sel->per_to_tith);
	}
	if(!((*dispnotflags)&GI))
	{
		setcurpos(SCR_ET_GIX,SCR_ET_GIY);
		printf("%.02f",sel->gross_income);
	}
	if(!((*dispnotflags)&NV))
	{
		setcurpos(SCR_ET_VALUEX,SCR_ET_VALUEY);
		printf("%.02f",sel->value);
	}
	setcurpos(SCR_ET_CATEGX,SCR_ET_CATEGY);
	printf("%s",sel->categ->name);
	setcurpos(SCR_ET_DCX,SCR_ET_DCY);
	printf("%s",sel->dc->name);
	if(DEPED&sel->flags)
	{
		setcurpos(1,SCR_ET_GENFLAGSY);
		printf("X");
	}
	if(BANKVERF&sel->flags)
	{
		setcurpos(1,SCR_ET_GENFLAGSY+1);
		printf("X");
	}
	if(TAXEI&sel->flags)
	{
		setcurpos(1,SCR_ET_TAXFLAGSY);
		printf("X");
	}
	if(TAXD&sel->flags)
	{
		setcurpos(1,SCR_ET_TAXFLAGSY+1);
		printf("X");
	}
	if(TAXI&sel->flags)
	{
		setcurpos(1,SCR_ET_TAXFLAGSY+2);
		printf("X");
	}
	if(options.flags&TUTORIAL)
	{
		write_scr(0,15,etscr_tutorial);
	}
}

#define RET_TRUE 0
#define RET_FALSE 1
#define MOVE 2

B8 et_handle_alt(struct TRANS *sel,struct TRANS *tb,B8 *input_locnum)
{
	print_infoline("Alt-");
	while(key[KEY_ALT])
	{
		if(key[KEY_Q])
		{
			sub_from_totals(sel);
			memcpy(sel,tb,sizeof(struct TRANS));
			add_to_totals(sel);
			if(TEMPLATE&sel->flags)
			{
				sel->flags&=~TEMPLATE;
				if(sel==(struct TRANS *)tl.last)
					add_tmplt_to_list(&tl);
			}
			clear_keybuf();
			return RET_TRUE;
		}
		else if(key[KEY_Y])
		{
			*input_locnum=0;
			key[KEY_Y]=FALSE;
			break;
		}
		else if(key[KEY_M])
		{
			*input_locnum=1;
			key[KEY_M]=FALSE;
			break;
		}
		else if(key[KEY_D])
		{
			*input_locnum=2;
			key[KEY_D]=FALSE;
			break;
		}
		else if(key[KEY_O])
		{
			*input_locnum=3;
			key[KEY_O]=FALSE;
			break;
		}
		else if(key[KEY_P])
		{
			*input_locnum=4;
			key[KEY_P]=FALSE;
			break;
		}
		else if(key[KEY_G])
		{
			*input_locnum=5;
			key[KEY_G]=FALSE;
			break;
		}
		else if(key[KEY_V])
		{
			*input_locnum=6;
			key[KEY_V]=FALSE;
			break;
		}
		else if(key[KEY_C])
		{
			*input_locnum=7;
			key[KEY_C]=FALSE;
			break;
		}
		else if(key[KEY_T])
		{
			*input_locnum=8;
			key[KEY_T]=FALSE;
			break;
		}
		else if(key[KEY_E])
		{
			*input_locnum=9;
			key[KEY_E]=FALSE;
			break;
		}
		else if(key[KEY_B])
		{
			*input_locnum=10;
			key[KEY_B]=FALSE;
			break;
		}
		else if(key[KEY_R])
		{
			*input_locnum=11;
			key[KEY_R]=FALSE;
			break;
		}
		else if(key[KEY_U])
		{
			*input_locnum=12;
			key[KEY_U]=FALSE;
			break;
		}
		else if(key[KEY_N])
		{
			*input_locnum=13;
			key[KEY_N]=FALSE;
			break;
		}
	}
	print_infoline("");
	return MOVE;
}

B8 et_handle_chkbox(B8 x,B8 y,B16 fpattern,struct TRANS *tb,struct TRANS *sel,B8 *input_locnum)
{
	B16 code;

	setcurpos(x,y);
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			if(et_handle_alt(sel,tb,input_locnum)==RET_TRUE)
				return RET_TRUE;
			return MOVE;
		}
		switch(code>>8)
		{
			case KEY_SPACE:
			{
				tb->flags^=fpattern;
				if(tb->flags&fpattern)
				{
					write_scr(x,y,"X");
				}
				else
				{
					write_scr(x,y," ");
				}
				break;
			}
			case KEY_ENTER:
			case KEY_ENTER_PAD:
			{
				(*input_locnum)++;
				if(*input_locnum>MAX_INLOCNUM)
					*input_locnum=0;
				return MOVE;
			}
			case KEY_ESC:
			{
				clear_keybuf();
				#ifdef RANDOM_INPUT
					break;
				#else
					return RET_FALSE;
				#endif
			}
			default:
			{
				break;
			}
		}
	}
}

B8 *et_dc_search_tocmp(void *x)
{
	return (void *)((struct DC *)x)->name;
}

void et_dc_search_refresh(void *x)
{
	write_scr(SCR_ET_DCX,SCR_ET_DCY,"              ");
	write_scr(SCR_ET_DCX,SCR_ET_DCY,((struct DC *)x)->name);
}

B8 et_dc_search_valid_match(void *x)
{
	return ( !((struct DC *)x)->tmplt );
}

B8 et_handle_dc(B8 x,B8 y,struct DC **dc,struct TRANS *tb,struct TRANS *sel,B8 *input_locnum)
{
	struct DC *bdc;
	B8 sstr[15];
	B16 keycode;
	static struct LIST_SEARCH_STR sd=
	{
		0,0,0,LIST_CMP_FROM_STR0|LIST_SEARCH_FROM_LISTFIRST,0,
		et_dc_search_tocmp,et_dc_search_refresh,et_dc_search_valid_match
	};

	sd.base=
	sd.pos=sstr;
	sd.boundry=&sstr[14];
	sd.base[0]=0;
	sd.match=0;

	setcurpos(x,y);
	write_scr(x,y,"               ");
	write_scr(x,y,(*dc)->name);
	while(1)
	{
		keycode=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			if(et_handle_alt(sel,tb,input_locnum)==RET_TRUE)
				return RET_TRUE;
			print_infoline("");
			return MOVE;
		}
		switch(keycode>>8)
		{
			case KEY_UP:
			{
				if((*dc)->prev)
				{
					for(bdc=(*dc)->prev;bdc->tmplt && bdc->prev;bdc=bdc->prev);
					if(bdc->tmplt)
						continue;
					*dc=bdc;
					write_scr(x,y,"               ");
					write_scr(x,y,(*dc)->name);
				}
				break;
			}
			case KEY_DOWN:
			{
				if((*dc)->next)
				{
					for(bdc=(*dc)->next;bdc->tmplt && bdc->next;bdc=bdc->next);
					if(bdc->tmplt)
						continue;
					*dc=bdc;
					write_scr(x,y,"               ");
					write_scr(x,y,(*dc)->name);
				}
				break;
			}
			case KEY_ESC:
			{
				clear_keybuf();
				#ifdef RANDOM_INPUT
					break;
				#else
					return RET_FALSE;
				#endif
			}
			case KEY_ENTER:
			case KEY_ENTER_PAD:
			{
				(*input_locnum)++;
				print_infoline("");
				return MOVE;
			}
			default:
			{
				search_list_str(&dcl,&sd,keycode&0xFF);
				if(sd.match)
					(*dc)=(struct DC *)sd.match;
				print_infoline("Search for: %s",sstr);
				setcurpos(x,y);
				break;
			}
		}
	}
}

B8 *et_categ_search_tocmp(void *x)
{
	return ((struct CATEG *)x)->name;
}

void et_categ_search_refresh(void *x)
{
	write_scr(SCR_ET_CATEGX,SCR_ET_CATEGY,"               ");
	write_scr(SCR_ET_CATEGX,SCR_ET_CATEGY,((struct CATEG *)x)->name);
}

B8 et_categ_search_valid_match(void *x)
{
	return ( !((struct CATEG *)x)->tmplt );
}

B8 edit_trans(struct TRANS *sel)
{
	B8 input_locnum=0;
	B16 flags;
	B16 keycode;
	B8 notdispflags;
	B8 sstr[15];
	struct TRANS tb;
	struct CATEG *categ;
	static struct LIST_SEARCH_STR sd=
	{
		0,0,0,LIST_CMP_FROM_STR0|LIST_SEARCH_FROM_LISTFIRST,0,
		et_categ_search_tocmp,et_categ_search_refresh,et_categ_search_valid_match
	};

	sd.base=
	sd.pos=sstr;
	sd.boundry=&sstr[14];

	memcpy(&tb,sel,sizeof(struct TRANS));
	if(TEMPLATE&sel->flags)
	{
		notdispflags=PTT|GI|NV;
		tb.memo[0]=0;
	}
	else
	{
		notdispflags=FALSE;
	}
	display_etscr(&tb,&notdispflags);
	while(1)
	{
		clear_keybuf();
		switch(input_locnum)
		{
			//year
			case 0:
			{
				while(1)
				{
					setcurpos(SCR_ET_DATEX,SCR_ET_DATEY);
					flags=1;
					if(!get_ub16(&(tb.year),1980,9999,4,"\n\r",&flags))
					{
						if(IOESC&flags)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(IOF1&flags)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(IOALT&flags)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//month
			case 1:
			{
				while(1)
				{
					setcurpos(SCR_ET_DATEX+5,SCR_ET_DATEY);
					flags=1;
					if(!get_ub8(&(tb.month),1,12,2,"\n\r",&flags))
					{
						if(IOESC&flags)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(IOF1&flags)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(IOALT&flags)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//day
			case 2:
			{
				while(1)
				{
					setcurpos(SCR_ET_DATEX+8,SCR_ET_DATEY);
					flags=1;
					if(!get_ub8(&(tb.day),1,31,2,"\n\r",&flags))
					{
						if(IOESC&flags)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(IOF1&flags)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(IOALT&flags)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//memo
			case 3:
			{
				while(1)
				{
					setcurpos(SCR_ET_MEMOX,SCR_ET_MEMOY);
					flags=1;
					if(!get_string(tb.memo,53,"\n\r","",&flags))
					{
						if(flags&IOESC)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(flags&IOF1)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(flags&IOALT)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//per_to_tith
			case 4:
			{
				while(1)
				{
					setcurpos(SCR_ET_PERTOTITHX,SCR_ET_PERTOTITHY);
					flags=1;
					if(notdispflags&PTT)
					{
						notdispflags&=~PTT;
						flags=0;
					}
					if(!get_ub8(&(tb.per_to_tith),0,100,3,"\n\r",&flags))
					{
						if(flags&IOESC)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(IOF1&flags)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(IOALT&flags)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//gross income
			case 5:
			{
				while(1)
				{
					setcurpos(SCR_ET_GIX,SCR_ET_GIY);
					flags=1;
					if(notdispflags&GI)
					{
						notdispflags&=~GI;
						flags=0;
					}
					if(!(get_float(&(tb.gross_income),0,MAX_AMOUNT,MAX_BDDIGITS,MAX_ADDIGITS,"\n\r",&flags,"%.02f")))
					{
						if(flags&IOESC)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(IOF1&flags)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(IOALT&flags)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//value
			case 6:
			{
				while(1)
				{
					setcurpos(SCR_ET_VALUEX,SCR_ET_VALUEY);
					flags=1;
					if(notdispflags&NV)
					{
						notdispflags&=~NV;
						flags=0;
					}
					if(!(get_float(&(tb.value),MIN_AMOUNT,MAX_AMOUNT,MAX_BDDIGITS,MAX_ADDIGITS,"\n\r",&flags,"%.02f")))
					{
						if(flags&IOESC)
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						else if(IOF1&flags)
						{
							help();
							display_etscr(&tb,&notdispflags);
							break;
						}
					}
					else
					{
						if(IOALT&flags)
						{
							if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
								return TRUE;
						}
						else
						{
							input_locnum++;
						}
						break;
					}
				}
				break;
			}
			//category
			case 7:
			{
				setcurpos(SCR_ET_CATEGX,SCR_ET_CATEGY);
				sd.base[0]=0;
				sd.pos=sstr;
				sd.match=0;
				while(1)
				{
					keycode=readkey();
					if(key_shifts&KB_ALT_FLAG)
					{
						if(et_handle_alt(sel,&tb,&input_locnum)==RET_TRUE)
							return TRUE;
						break;
					}
					switch(keycode>>8)
					{
						case KEY_UP:
						{
							if(tb.categ->prev)
							{
								for(categ=tb.categ->prev;categ->tmplt && categ->prev;categ=categ->prev);
								if(categ->tmplt)
									continue;
								tb.categ=categ;
								write_scr(SCR_ET_CATEGX,SCR_ET_CATEGY,"               ");
								write_scr(SCR_ET_CATEGX,SCR_ET_CATEGY,tb.categ->name);
							}
							break;
						}
						case KEY_DOWN:
						{
							if(tb.categ->next)
							{
								for(categ=tb.categ->next;categ->tmplt && categ->next;categ=categ->next);
								if(categ->tmplt)
									continue;
								tb.categ=categ;
								write_scr(SCR_ET_CATEGX,SCR_ET_CATEGY,"               ");
								write_scr(SCR_ET_CATEGX,SCR_ET_CATEGY,tb.categ->name);
							}
							break;
						}
						case KEY_ESC:
						{
							clear_keybuf();
							#ifdef RANDOM_INPUT
								goto exit_save;
							#else
								return FALSE;
							#endif
						}
						case KEY_ENTER:
						case KEY_ENTER_PAD:
						{
							input_locnum++;
							goto continue_input;
						}
						default:
						{
							search_list_str(&cl,&sd,keycode&0xFF);
							if(sd.match)
								tb.categ=(struct CATEG *)sd.match;
							print_infoline("Searching for: %s",sd.base);
							setcurpos(SCR_ET_CATEGX,SCR_ET_CATEGY);
							break;
						}
					}
				}
				continue_input:
				print_infoline("");
				break;
			}
			//debtor/creditor
			case 8:
			{
				flags=(B16)et_handle_dc(SCR_ET_DCX,SCR_ET_DCY,&(tb.dc),&tb,sel,&input_locnum);
				if(flags==RET_TRUE)
					return TRUE;
				else if(flags==RET_FALSE)
					return FALSE;
				break;
			}
			//deposited
			case 9:
			{
				switch(et_handle_chkbox(1,SCR_ET_GENFLAGSY,DEPED,&tb,sel,&input_locnum))
				{
					case RET_TRUE:
					{
						return TRUE;
					}
					case RET_FALSE:
					{
						return FALSE;
					}
					default:
					{
						break;
					}
				}
				break;
			}
			//bank verified
			case 10:
			{
				switch(et_handle_chkbox(1,SCR_ET_GENFLAGSY+1,BANKVERF,&tb,sel,&input_locnum))
				{
					case RET_TRUE:
					{
						return TRUE;
					}
					case RET_FALSE:
					{
						return FALSE;
					}
					default:
					{
						break;
					}
				}
				break;
			}
			//tax: earned interest
			case 11:
			{
				switch(et_handle_chkbox(1,SCR_ET_TAXFLAGSY,TAXEI,&tb,sel,&input_locnum))
				{
					case RET_TRUE:
					{
						return TRUE;
					}
					case RET_FALSE:
					{
						return FALSE;
					}
					default:
					{
						break;
					}
				}
				break;
			}
			//tax deductable
			case 12:
			{
				switch(et_handle_chkbox(1,SCR_ET_TAXFLAGSY+1,TAXD,&tb,sel,&input_locnum))
				{
					case RET_TRUE:
					{
						return TRUE;
					}
					case RET_FALSE:
					{
						return FALSE;
					}
					default:
					{
						break;
					}
				}
				break;
			}
			//taxable income
			case 13:
			{
				switch(et_handle_chkbox(1,SCR_ET_TAXFLAGSY+2,TAXI,&tb,sel,&input_locnum))
				{
					case RET_TRUE:
					{
						return TRUE;
					}
					case RET_FALSE:
					{
						return FALSE;
					}
					default:
					{
						break;
					}
				}
				break;
			}
			default:
			{
				#ifdef DEBUG
					write_logf(0,"tl.c:edit_trans:input_locnum is <13!:%u\n",input_locnum);
					exit(EXIT_DEVELOPMENTERROR);
				#endif
				break;
			}
		}
	}
	#ifdef RANDOM_INPUT
		exit_save:
		sub_from_totals(sel);
		memcpy(sel,&tb,sizeof(struct TRANS));
		add_to_totals(sel);
		if(TEMPLATE&sel->flags)
		{
			sel->flags&=~TEMPLATE;
			if(sel==(struct TRANS *)tl.last)
				add_tmplt_to_list(&tl);
		}
		clear_keybuf();
		return RET_TRUE;
	#endif
}
