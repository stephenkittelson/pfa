#include <myalleg.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
//#include "pfa.h"
#include "scr.h"
#include "cl.h"
#include "dcl.h"
#include "tl.h"
#include "totals.h"
#include "search.h"
#include "options.h"
#include "search.h"
#include "list.h"
#include LIBDIR"flags.h"

extern struct LIST tl,cl,dcl;
extern struct COLOR user_colors;
extern B8 *etscr;

static void search_tl_flags(void);
static void search_tl_categs(void);
static void search_tl_dcs(void);

static B8 *search_tl_memo_tocmp(void *x)
{
	return ((struct TRANS *)x)->memo;
}

static B8 search_tl_memo_valid_match(void *x)
{
	return TRUE;
}

void search_tl_memo(void)
{
	B16 keycode;
	B8 sstr[53];
	static struct LIST_SEARCH_STR sd=
	{
		0,0,0,0,0,search_tl_memo_tocmp,0,search_tl_memo_valid_match
	};

	sd.base=
	sd.pos=sstr;
	sd.boundry=&sstr[52];
	sd.base[0]=0;
	sd.match=0;

	clear_keybuf();
	print_infoline("Search for: ");
	normcur();
	while(1)
	{
		keycode=readkey();
		switch(keycode>>8)
		{
			case KEY_UP:
			{
				if(sstr[0]==0)
					continue;
				if(tl.sel==tl.first)
					continue;
				sd.flags=LIST_SEARCH_PREV;
				search_list_str_sel(&tl,&sd,0);
				print_infoline("Search for: %s",sstr);
				break;
			}
			case KEY_DOWN:
			{
				if(sstr[0]==0)
					continue;
				if(tl.sel==tl.last)
					continue;
				sd.flags=LIST_SEARCH_NEXT;
				search_list_str_sel(&tl,&sd,0);
				print_infoline("Search for: %s",sstr);
				break;
			}
			case KEY_ESC:
			{
				print_infoline("");
				hidecur();
				return;
			}
			default:
			{
				if(key_shifts&KB_ALT_FLAG)
				{
					switch(keycode>>8)
					{
						case KEY_F:
						{
							search_tl_flags();
							hidecur();
							return;
						}
						case KEY_C:
						{
							hidecur();
							search_tl_categs();
							return;
						}
						case KEY_T:
						{
							hidecur();
							search_tl_dcs();
							return;
						}
						default:
						{
							break;
						}
					}
				}
				else
				{
					sd.flags=LIST_SEARCH_NEXT;
					search_list_str_sel(&tl,&sd,keycode&0xFF);
					print_infoline("Search for: %s",sstr);
				}
				break;
			}
		}
	}
}

#define BEG_SEARCH 0
#define CANCEL_SEARCH 1
#define CON_SEL 2

#define SCR_FS_GENFLAGSY 1
#define SCR_FS_TAXFLAGSY 6

B8 sflags,chkflags;

static B8 ms_handle_chkbox(B8 x,B8 y,B8 fpattern,B8 *input_locnum)
{
	B16 keycode;

	setcurpos(x,y);
	while(1)
	{
		keycode=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			switch(keycode>>8)
			{
				case KEY_Q:
				{
					print_infoline("");
					return BEG_SEARCH;
				}
				case KEY_E:
				{
					*input_locnum=0;
					return CON_SEL;
				}
				case KEY_B:
				{
					*input_locnum=1;
					return CON_SEL;
				}
				case KEY_R:
				{
					*input_locnum=2;
					return CON_SEL;
				}
				case KEY_U:
				{
					*input_locnum=3;
					return CON_SEL;
				}
				case KEY_N:
				{
					*input_locnum=4;
					return CON_SEL;
				}
				default:
				{
					continue;
				}
			}
		}
		switch(keycode>>8)
		{
			case KEY_SPACE:
			{
				if(chkflags&fpattern)
				{
					sflags^=fpattern;
					if(sflags&fpattern)
					{
						printf("X");
						setcurpos(x,y);
					}
					else
					{
						printf("*");
						setcurpos(x,y);
						chkflags^=fpattern;
					}
				}
				else
				{
					chkflags^=fpattern;
					printf(" ");
					setcurpos(x,y);
				}
				break;
			}
			case KEY_ENTER:
			case KEY_ENTER_PAD:
			{
				(*input_locnum)++;
				if(*input_locnum>4)
					*input_locnum=0;
				return CON_SEL;
			}
			case KEY_ESC:
			{
				return CANCEL_SEARCH;
			}
			default:
			{
				break;
			}
		}
	}
}

static B8 search_flags_cmp(void)
{
	return ( (((struct TRANS *)tl.sel)->flags&chkflags)==sflags );
}

static void search_tl_flags(void)
{
	B8 input_locnum=0;
	B16 keycode;
	static struct LIST_SEARCH sd=
	{
		search_flags_cmp,0
	};

	clear_scr();
	sflags=chkflags=FALSE;
	write_scr(0,1,etscr+480);
	HIGHLIGHT(5 ,1 ); //dEposited
	HIGHLIGHT(4 ,2 ); //Bank has verified
	HIGHLIGHT(6 ,6 ); //eaRned interest
	HIGHLIGHT(11,7 ); //tax dedUctable
	HIGHLIGHT(13,8 ); //taxable iNcome
	write_scr(1,SCR_FS_GENFLAGSY  ,"*");
	write_scr(1,SCR_FS_GENFLAGSY+1,"*");
	write_scr(1,SCR_FS_TAXFLAGSY  ,"*");
	write_scr(1,SCR_FS_TAXFLAGSY+1,"*");
	write_scr(1,SCR_FS_TAXFLAGSY+2,"*");
	while(1)
	{
		switch(input_locnum)
		{
			//DEPED
			case 0:
			{
				switch(ms_handle_chkbox(1,SCR_FS_GENFLAGSY,DEPED,&input_locnum))
				{
					case BEG_SEARCH:
					{
						goto begin_search;
					}
					case CANCEL_SEARCH:
					{
						tl_display();
						return;
					}
					case CON_SEL:
					{
						break;
					}
				}
				break;
			}
			//BANK VERF
			case 1:
			{
				switch(ms_handle_chkbox(1,SCR_FS_GENFLAGSY+1,BANKVERF,&input_locnum))
				{
					case BEG_SEARCH:
					{
						goto begin_search;
					}
					case CANCEL_SEARCH:
					{
						tl_display();
						return;
					}
					case CON_SEL:
					{
						break;
					}
				}
				break;
			}
			//TAXEI
			case 2:
			{
				switch(ms_handle_chkbox(1,SCR_FS_TAXFLAGSY,TAXEI,&input_locnum))
				{
					case BEG_SEARCH:
					{
						goto begin_search;
					}
					case CANCEL_SEARCH:
					{
						tl_display();
						return;
					}
					case CON_SEL:
					{
						break;
					}
				}
				break;
			}
			//TAXD
			case 3:
			{
				switch(ms_handle_chkbox(1,SCR_FS_TAXFLAGSY+1,TAXD,&input_locnum))
				{
					case BEG_SEARCH:
					{
						goto begin_search;
					}
					case CANCEL_SEARCH:
					{
						tl_display();
						return;
					}
					case CON_SEL:
					{
						break;
					}
				}
				break;
			}
			//TAXI
			case 4:
			{
				switch(ms_handle_chkbox(1,SCR_FS_TAXFLAGSY+2,TAXI,&input_locnum))
				{
					case BEG_SEARCH:
					{
						goto begin_search;
					}
					case CANCEL_SEARCH:
					{
						tl_display();
						return;
					}
					case CON_SEL:
					{
						break;
					}
				}
				break;
			}
		}
	}
	begin_search:
	tl_display();

	sd.flags=LIST_SEARCH_FROM_LISTFIRST;
	search_list_sel(&tl,&sd);
	if(!(sd.flags&LIST_SEARCH_MATCH_FOUND))
		print_infoline("No match found");
	else
		print_infoline("Match");
	while(1)
	{
		keycode=readkey();
		switch(keycode>>8)
		{
			case KEY_UP:
			{
				if(tl.sel==tl.first)
					continue;
				sd.flags=LIST_SEARCH_PREV;
				search_list_sel(&tl,&sd);
				if(!(sd.flags&LIST_SEARCH_MATCH_FOUND))
					print_infoline("No match found");
				else
					print_infoline("Match");
				break;
			}
			case KEY_DOWN:
			{
				if(tl.sel==tl.last)
					continue;
				sd.flags=LIST_SEARCH_NEXT;
				search_list_sel(&tl,&sd);
				if(!(sd.flags&LIST_SEARCH_MATCH_FOUND))
					print_infoline("No match found");
				else
					print_infoline("Match");
				break;
			}
			case KEY_ESC:
			{
				print_infoline("");
				return;
			}
			default:
			{
				break;
			}
		}
	}
}

struct CATEG *categ;

static B8 search_categs_cmp(void)
{
	return ( ((struct TRANS *)tl.sel)->categ==categ );
}

static B8 *search_categs_tocmp(void *x)
{
	return ((struct CATEG *)x)->name;
}

static void search_categs_refresh(void *x)
{
	write_scr(24,48,"               ");
	write_scr(24,48,((struct CATEG *)x)->name);
}

static B8 search_categs_valid_match(void *x)
{
	return !((struct CATEG *)x)->tmplt;
}

static void search_tl_categs(void)
{
	B8 sstr[15];
	B16 keycode;
	struct CATEG *bc;
	static struct LIST_SEARCH_STR sds=
	{
		0,0,0,LIST_CMP_FROM_STR0|LIST_SEARCH_FROM_LISTFIRST,0,
		search_categs_tocmp,search_categs_refresh,search_categs_valid_match
	};
	static struct LIST_SEARCH sd=
	{
		search_categs_cmp,0
	};

	sds.base=
	sds.pos=sstr;
	sds.boundry=&sstr[14];
	sds.base[0]=0;
	sds.match=0;

	categ=(struct CATEG *)cl.first;
	print_infoline("Category to search for: %s",categ->name);
	while(1)
	{
		keycode=readkey();
		switch(keycode>>8)
		{
			case KEY_DOWN:
			{
				if(categ->next)
				{
					for(bc=categ->next;bc->tmplt && bc->next;bc=bc->next);
					if(bc->tmplt)
						continue;
					categ=bc;
					write_scr(24,48,"               ");
					write_scr(24,48,categ->name);
				}
				break;
			}
			case KEY_UP:
			{
				if(categ->prev)
				{
					for(bc=categ->prev;bc->tmplt && bc->prev;bc=bc->prev);
					if(bc->tmplt)
						continue;
					categ=bc;
					write_scr(24,48,"               ");
					write_scr(24,48,categ->name);
				}
				break;
			}
			case KEY_ESC:
			{
				print_infoline("");
				return;
			}
			case KEY_ENTER:
			case KEY_ENTER_PAD:
			{
				sd.flags=LIST_SEARCH_NEXT;
				search_list_sel(&tl,&sd);
				if(sd.flags&LIST_SEARCH_MATCH_FOUND)
					print_infoline("Match");
				else
					print_infoline("No match found for '%s'",categ->name);
				while(1)
				{
					keycode=readkey();
					switch(keycode>>8)
					{
						case KEY_UP:
						{
							if(tl.sel==tl.first)
								continue;
							sd.flags=LIST_SEARCH_PREV;
							search_list_sel(&tl,&sd);
							if(sd.flags&LIST_SEARCH_MATCH_FOUND)
								print_infoline("Match");
							else
								print_infoline("No match found for '%s'",categ->name);
							break;
						}
						case KEY_DOWN:
						{
							if(tl.sel==tl.last)
								continue;
							sd.flags=LIST_SEARCH_NEXT;
							search_list_sel(&tl,&sd);
							if(sd.flags&LIST_SEARCH_MATCH_FOUND)
								print_infoline("Match");
							else
								print_infoline("No match found for '%s'",categ->name);
							break;
						}
						case KEY_ESC:
						{
							print_infoline("");
							return;
						}
						default:
						{
							break;
						}
					}
				}
			}
			default:
			{
				sd.flags=LIST_SEARCH_NEXT;
				search_list_str(&cl,&sds,(B8)(keycode&0xFF));
				if(sds.match)
					categ=(struct CATEG *)sds.match;
				write_scr(0,49,"               ");
				write_scr(0,49,sds.base);
				break;
			}
		}
	}
}

struct DC *dc;

static B8 search_dcs_cmp(void)
{
	return ( ((struct TRANS *)tl.sel)->dc==dc );
}

static B8 *search_dcs_tocmp(void *x)
{
	return ((struct DC *)x)->name;
}

static void search_dcs_refresh(void *x)
{
	write_scr(26,48,"               ");
	write_scr(26,48,((struct DC *)x)->name);
}

static B8 search_dcs_valid_match(void *x)
{
	return !((struct DC *)x)->tmplt;
}

static void search_tl_dcs(void)
{
	B8 sstr[15];
	B16 keycode;
	struct DC *bdc;
	static struct LIST_SEARCH_STR sds=
	{
		0,0,0,LIST_CMP_FROM_STR0|LIST_SEARCH_NEXT,0,
		search_dcs_tocmp,search_dcs_refresh,search_dcs_valid_match
	};
	static struct LIST_SEARCH sd=
	{
		search_dcs_cmp,0
	};

	sds.base=
	sds.pos=sstr;
	sds.boundry=&sstr[14];
	sds.base[0]=0;
	sds.match=0;

	dc=(struct DC *)dcl.first;
	print_infoline("Transactee to search for: %s",dc->name);
	while(1)
	{
		keycode=readkey();
		switch(keycode>>8)
		{
			case KEY_DOWN:
			{
				if(dc->next)
				{
					for(bdc=dc->next;bdc->tmplt && bdc->next;bdc=bdc->next);
					if(bdc->tmplt)
						continue;
					dc=bdc;
					write_scr(26,48,"               ");
					write_scr(26,48,dc->name);
				}
				break;
			}
			case KEY_UP:
			{
				if(dc->prev)
				{
					for(bdc=dc->prev;bdc->tmplt && bdc->prev;bdc=bdc->prev);
					if(bdc->tmplt)
						continue;
					dc=bdc;
					write_scr(26,48,"               ");
					write_scr(26,48,dc->name);
				}
				break;
			}
			case KEY_ESC:
			{
				print_infoline("");
				return;
			}
			case KEY_ENTER:
			case KEY_ENTER_PAD:
			{
				sd.flags=LIST_SEARCH_NEXT;
				search_list_sel(&tl,&sd);
				if(sd.flags&LIST_SEARCH_MATCH_FOUND)
					print_infoline("Match");
				else
					print_infoline("No match found for '%s'",dc->name);
				while(1)
				{
					keycode=readkey();
					switch(keycode>>8)
					{
						case KEY_UP:
						{
							if(tl.sel==tl.first)
								continue;
							sd.flags=LIST_SEARCH_PREV;
							search_list_sel(&tl,&sd);
							if(sd.flags&LIST_SEARCH_MATCH_FOUND)
								print_infoline("Match");
							else
								print_infoline("No match found for '%s'",dc->name);
							break;
						}
						case KEY_DOWN:
						{
							if(tl.sel==tl.last)
								continue;
							sd.flags=LIST_SEARCH_NEXT;
							search_list_sel(&tl,&sd);
							if(sd.flags&LIST_SEARCH_MATCH_FOUND)
								print_infoline("Match");
							else
								print_infoline("No match found for '%s'",dc->name);
							break;
						}
						case KEY_ESC:
						{
							print_infoline("");
							return;
						}
						default:
						{
							break;
						}
					}
				}
			}
			default:
			{
				search_list_str(&dcl,&sds,(B8)(keycode&0xFF));
				if(sds.match)
					dc=(struct DC *)sds.match;
				write_scr(0,49,"               ");
				write_scr(0,49,sds.base);
				break;
			}
		}
	}
}
