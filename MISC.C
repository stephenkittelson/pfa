#include <time.h>
#include <myalleg.h>
#include <string.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "misc.h"
#include "scr.h"
#include "calc.h"
#include "help.h"
#include "options.h"
#include "io.h"
#include "file.h"
#include "tl.h"
#include "totals.h"
#ifdef DEBUG
	#include "dcl.h"
	#include "cl.h"
	#ifdef RANDOM_INPUT
		#include <signal.h>
	#endif
#endif

extern B8 modified;
extern struct FILE_HIST *begin_fh_list;

#ifdef DEBUG
	extern struct LIST tl,cl,dcl;
#endif

void sys_cmd(B8 *str,...)
{
	va_list ap;
	B8 buf[100];

	va_start(ap,str);
	vsprintf(buf,str,ap);
	va_end(ap);
	strcat(buf," > nul");
	system(buf);
}

void print_version(void)
{
	static B8 *buf={"Personal Financial Assistant: Version " VSTR};
	B8 x=40-(strlen(buf)>>1);

	write_scr(x,0,buf);
}

/*
	handle_globalkeys
		takes care of:
			help                  F1
			options               F2
			load                  L
			write                 W
			calculator            C
			show totals           T
			retry tmplt addition  R
			insert tmplt          Ins
			exit                  Esc
*/

void handle_globalkeys(struct LIST *list,B16 keycode,void (*display)(void),B8 last_not_tmplt)
{
	struct CALC_RETURN calc_ret;
	struct FILE_HIST *fh;
	double calc_ans;
	B16 flags;
	B8 buf[150];
	static B8 inkeyseq=FALSE;
	B8 ch;
	static B8 keyseqpos=0;

	switch(keycode>>8)
	{
		//help
		case KEY_F1:
		{
			help();
			display();
			return;
		}
		//edit options
		case KEY_F2:
		{
			edit_options();
			display();
			return;
		}
		//load into memory from hard disk
		case KEY_L:
		{
			list->unselect();
			if(begin_fh_list)
			{
				write_scr(0,9,"                                                                                ");
				for(flags=10,fh=begin_fh_list;fh;fh=fh->next,flags++)
				{
					write_scr(0,flags,"                                                                                ");
					write_scr(0,flags,fh->name);
				}
				write_scr(0,flags,"                                                                                ");
			}
			clear_keybuf();
			if(modified)
			{
				normcur();
				print_infoline("Are you sure you want to continue without saving? ");
				if((readkey()>>8)!=KEY_Y)
				{
					hidecur();
					if(begin_fh_list)
					{
						display();
					}
					return;
				}
				hidecur();
			}
			print_infoline("Enter file name: ");
			flags=0;
			buf[0]=0;
			if(!get_string(buf,FILENAME_SIZE,"\n\r",INV_FN_CHARS,&flags))
			{
				if(begin_fh_list)
				{
					display();
				}
				print_infoline("Load aborted");
				hidecur();
				clear_keybuf();
				return;
			}
			if(begin_fh_list)
			{
				display();
			}
			hidecur();
			load_file(buf);
			modified=FALSE;
			list->activate(FALSE);
			list->select();
			return;
		}
		//write to hard disk
		case KEY_W:
		{
			list->unselect();
			if(begin_fh_list)
			{
				write_scr(0,9,"                                                                                ");
				for(flags=10,fh=begin_fh_list;fh;fh=fh->next,flags++)
				{
					write_scr(0,flags,"                                                                                ");
					write_scr(0,flags,fh->name);
				}
				write_scr(0,flags,"                                                                                ");
			}
			clear_keybuf();
			print_infoline("Enter file name: ");
			flags=0;
			buf[0]=0;
			normcur();
			if(!(get_string(buf,FILENAME_SIZE,"\n\r",INV_FN_CHARS,&flags)))
			{
				if(begin_fh_list)
				{
					display();
				}
				print_infoline("Write aborted");
				hidecur();
				return;
			}
			if(begin_fh_list)
			{
				display();
			}
			hidecur();
			write_file(buf);
			modified=FALSE;
			return;
		}
		//calculator
		case KEY_C:
		{
			clear_keybuf();
			print_infoline("Equation: ");
			normcur();
			flags=FALSE;
			if(!get_string(buf,150,"\n\r",INV_FN_CHARS,&flags))
			{
				hidecur();
				print_infoline("");
				return;
			}
			hidecur();
			calculator(buf,&calc_ans,&calc_ret);
			switch(calc_ret.error)
			{
				case NO_ERROR:
				{
					print_infoline("%.02f",calc_ans);
					break;
				}
				case DIV_ZERO:
				{
					print_infoline("Cannot divide by 0.");
					break;
				}
				case MISSING_AFTER:
				{
					print_infoline("Missing a ')'");
					break;
				}
				case MISSING_BEFORE:
				{
					print_infoline("Missing a '('");
					break;
				}
				case INVALID_CHAR:
				{
					print_infoline("Invalid character '%c'",calc_ret.invalid_char);
					break;
				}
				case INVALID_EXP:
				{
					print_infoline("%f is not a valid exponent! Exponents must be: > 1 OR 0.",calc_ans);
					break;
				}
				case BEFORE:
				{
					print_infoline("'%c' is not valid before '%c'",calc_ret.invalid_char,calc_ret.op);
					break;
				}
				case AFTER:
				{
					print_infoline("'%c' is not valid after '%c'",calc_ret.invalid_char,calc_ret.op);
					break;
				}
				case UNEXPECTED_END:
				{
					print_infoline("Equation ended unexpectedly after '%c'",calc_ret.op);
					break;
				}
				case STR_OVERFLOW:
				{
					print_infoline("The equation entered is too large");
					break;
				}
				default:
				{
					//this should never be reached
					#ifdef DEBUG
						write_logs(0,1,"handle_globalkeys: calculator default hit!?\n");
					#endif
					break;
				}
			}
			return;
		}
		//show totals
		case KEY_T:
		{
			#ifdef RANDOM_INPUT
				print_infoline("Cannot view totals in random input mode.");
				return;
			#endif
			show_totals();
			display();
			return;
		}
		case KEY_8:
		{
			if(key_shifts&KB_CTRL_FLAG && key_shifts&KB_ALT_FLAG && key_shifts&KB_SHIFT_FLAG && key_shifts&KB_SCROLOCK_FLAG && key_shifts&KB_NUMLOCK_FLAG && key_shifts&KB_CAPSLOCK_FLAG)
			{
				inkeyseq=TRUE;
			}
			return;
		}
		//retry addition of template alt+r
		case KEY_R:
		{
			if(!key_shifts&KB_ALT_FLAG)
				return;
			if(last_not_tmplt)
			{
				list->nomore=FALSE;
				add_tmplt_to_list(list);
			}
			return;
		}
		//insert tmplt before sel
		case KEY_INSERT:
		{
			modified=TRUE;
			insert_tmplt_above_sel(list);
			return;
		}
		//exit
		case KEY_ESC:
		{
			#ifdef RANDOM_INPUT
				print_infoline("Cannot exit using 'esc' with random input.");
				return;
			#endif
			clear_keybuf();
			if(modified)
			{
				print_infoline("Exit without saving data? ");
				normcur();
				if(readkey()>>8==KEY_Y)
					exit(EXIT_NORMAL);
				hidecur();
				print_infoline("");
				clear_keybuf();
				return;
			}
			else
			{
				exit(EXIT_NORMAL);
			}
		}
		default:
		{
			if(inkeyseq)
			{
				//GIgeh4^-byJ
				switch(keyseqpos)
				{
					case 0:
					{
						ch='G';
						break;
					}
					case 1:
					{
						ch='I';
						break;
					}
					case 2:
					{
						ch='g';
						break;
					}
					case 3:
					{
						ch='e';
						break;
					}
					case 4:
					{
						ch='h';
						break;
					}
					case 5:
					{
						ch='4';
						break;
					}
					case 6:
					{
						ch='^';
						break;
					}
					case 7:
					{
						ch='-';
						break;
					}
					case 8:
					{
						ch='b';
						break;
					}
					case 9:
					{
						ch='y';
						break;
					}
					case 10:
					{
						ch='J';
						break;
					}
					default:
					{
						write_scr(16,48,"S");
						sprintf(buf,"A§ eóí");
						write_scr( 3,48,":");
						sprintf(buf,"›ÆÐ#b");
						write_scr( 2,48,"A");
						sprintf(buf,"½èñ¢Dƒ\"");
						write_scr(15,49," we");
						sprintf(buf,"ÝdÏq°O");
						write_scr( 4,48," ");
						sprintf(buf,"J½ÿ‚@G¦„");
						sprintf(buf,"ò5´rØ¶");
						write_scr( 6,48,"y");
						sprintf(buf,"±Scâ v");
						write_scr(21,48," ");
						write_scr( 7,48," ");
						sprintf(buf,"èS¥Ý³ä? ");
						write_scr( 1,48,"F");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(12,48,"h");
						sprintf(buf,"VÉ‚@G¦„");
						write_scr(10,48,"e");
						sprintf(buf,"ò5´yØ¶");
						write_scr(11,48,"p");
						sprintf(buf,"±Scâ í");
						write_scr(30,48,"n. ");
						sprintf(buf,"è•¥Ý³ä? ");
						write_scr( 9,48,"t");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(14,48,"n");
						sprintf(buf,"V5‚@G¦„");
						write_scr(15,48," ");
						sprintf(buf,"ò“5HyØ¶");
						write_scr( 5,48,"B");
						sprintf(buf,"±Scâí");
						write_scr(17,48,"c");
						sprintf(buf,"S•¥Ý³ä? ");
						write_scr(18,48,"o");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(50,48,"ave");
						sprintf(buf,"Ék‚@G¦„");
						write_scr( 8,48,"S");
						sprintf(buf,"ò]rHyØ¶");
						write_scr(22,48,"K");
						sprintf(buf,"±Sc§í");
						write_scr(13,48,"e");
						sprintf(buf,"èS•¥Ý³ä? ");
						write_scr(24,48,"t");
						sprintf(buf,"‚ÄQò}[");
						write_scr(23,48,"i");
						sprintf(buf,"VÉ‚@¦„");
						write_scr(42,48," fr");
						sprintf(buf,"ò±5´rH");
						write_scr(28,48,"s");
						sprintf(buf,"±câ v§í");
						write_scr(25,48,"t");
						sprintf(buf,"èS•¥Ý³ä? ");
						write_scr(38,48,"he");
						sprintf(buf,"‚}[");
						write_scr(29,48,"o");
						sprintf(buf,"VÉG¦„");
						write_scr(34,48,"i");
						sprintf(buf,"ò!5´rHyØ¶");
						write_scr(35,48,"th");
						sprintf(buf,"±câ v§í");
						write_scr(37,48," ");
						sprintf(buf,"èS•¥³ä? ");
						write_scr(33,48,"W");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(19,48,"tt");
						sprintf(buf,"VG¦„");
						write_scr( 0,48,"P");
						sprintf(buf,"òrHyØ¶");
						write_scr(45,48,"o");
						sprintf(buf,"±Scâ v§í");
						write_scr(46,48,"m");
						sprintf(buf,"è•¥Ý³ä? ");
						write_scr(47,48," ");
						sprintf(buf,"‚Äò}[");
						write_scr(40,48,"lp");
						sprintf(buf,"‚@G¦„");
						write_scr(26,48,"el");
						sprintf(buf,"HyØ¶");
						write_scr(53,48,"n");
						sprintf(buf,"±â v§í");
						write_scr(48,48,"He");
						sprintf(buf,"èS•¥Ý³ä? ");
						write_scr(56,48," F");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(58,48,"at");
						sprintf(buf,"VÉ‚@G¦„");
						write_scr(54,48,"ly");
						sprintf(buf,"òÎ5´rHyØ¶");
						write_scr(62,48,"r");
						sprintf(buf,"±Sc");
						write_scr(63,48," ");
						sprintf(buf,"èS 0Ý? ");
						write_scr(60,48,"he");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(66,48,"d ");
						sprintf(buf,"V@G¦„");
						write_scr(68,48,"Je");
						sprintf(buf,"ò\n5´rØ¶");
						write_scr(64,48,"an");
						sprintf(buf,"±S v§í");
						write_scr(72,48,"s");
						sprintf(buf,"èS•¥Ý³ä? ");
						write_scr(26,49,"hin");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(70,48,"su");
						sprintf(buf,"VÉ„");
						write_scr(78,48,"s");
						sprintf(buf,"ò\r5´yØ¶");
						write_scr(79,48,"t");
						sprintf(buf,"±Scv§í");
						write_scr( 0,49,", w");
						sprintf(buf,"èS•Ý³ä? ");
						write_scr(76,48,"ri");
						sprintf(buf,"}[");
						write_scr( 6,49,"ou");
						sprintf(buf,"VÉ‚@G¦„");
						write_scr( 8,49,"t");
						sprintf(buf,"òÁ5´rHyØ¶");
						write_scr( 3,49,"ith");
						sprintf(buf,"Scâ v§í");
						write_scr(12,49,"em,");
						sprintf(buf,"èSÝ³ä? ");
						write_scr(20,49,"re ");
						sprintf(buf,"‚}[");
						write_scr(19,49,"a");
						sprintf(buf,"VÉgG¦„");
						write_scr( 9,49," Th");
						sprintf(buf,"ò5´rHyØ¶");
						write_scr(23,49,"no");
						sprintf(buf,"±câ §í");
						write_scr(25,49,"t");
						sprintf(buf,"è³ä? ");
						write_scr(29,49,"g!");
						sprintf(buf,"‚ÄÔQò}[");
						write_scr(73,48," Ch");
						sprintf(buf,"VÉ4G");
						inkeyseq=FALSE;
						keyseqpos=0;
						return;
					}
				}
				if((keycode&0xFF)!=ch)
				{
					inkeyseq=FALSE;
					keyseqpos=0;
					return;
				}
				keyseqpos++;
			}
			return;
		}
	}
}

#ifdef DEBUG

void dlog_lists(void)
{
	struct CATEG *categ;
	struct DC *dc;
	struct TRANS *t;
	B8 type[7]=
	{
		" FTSBL"
	};

	write_logs(0,1,"\nmisc.c:dlog_lists:showing lists...\n");
	categ=(struct CATEG *)cl.first;
	dc=(struct DC *)dcl.first;
	t=(struct TRANS *)tl.first;
	while(categ || dc || t)
	{
		if(categ)
		{
			write_logf(0," [%c%c%c%c%c] categ: %u [%s]\n",
				(((void *)categ==cl.first) ? type[1] : type[0]),
				(((void *)categ==cl.top  ) ? type[2] : type[0]),
				(((void *)categ==cl.sel  ) ? type[3] : type[0]),
				(((void *)categ==cl.bot  ) ? type[4] : type[0]),
				(((void *)categ==cl.last ) ? type[5] : type[0]),
				categ->tmplt,categ->name);
			categ=categ->next;
		}
		if(dc)
		{
			write_logf(0," [%c%c%c%c%c] dc:    [%s]\n",
				(((void *)dc==dcl.first) ? type[1] : type[0]),
				(((void *)dc==dcl.top  ) ? type[2] : type[0]),
				(((void *)dc==dcl.sel  ) ? type[3] : type[0]),
				(((void *)dc==dcl.bot  ) ? type[4] : type[0]),
				(((void *)dc==dcl.last ) ? type[5] : type[0]),
				dc->name);
			dc=dc->next;
		}
		if(t)
		{
			write_logf(0," [%c%c%c%c%c] trans: [%s] categ: [%s] dc: [%s]\n",
				(((void *)t==tl.first) ? type[1] : type[0]),
				(((void *)t==tl.top  ) ? type[2] : type[0]),
				(((void *)t==tl.sel  ) ? type[3] : type[0]),
				(((void *)t==tl.bot  ) ? type[4] : type[0]),
				(((void *)t==tl.last ) ? type[5] : type[0]),
				t->memo,t->categ->name,t->dc->name);
			t=t->next;
		}
	}
	write_logs(0,1,"misc.c:dlog_lists:done showing lists\n\n");
}

#ifdef RANDOM_INPUT

void random_input(void)
{
	static B8 active=TRUE;
	B8 scancode;
	B8 asciicode;
	B8 toggle=FALSE;

	if(active)
	{
		if(key[KEY_LCONTROL])
		{
			print_infoline("Random input paused.");
			active=FALSE;
			return;
		}
		scancode=rand()%93;
		asciicode=scancode_to_ascii(scancode);
		if(asciicode=='q')
		{
			_handle_pckey(KEY_ALT);
		}
		simulate_keypress(scancode<<8|asciicode);
		if(rand()%100<30)
		{
			scancode=rand()%12+99;
			if(scancode==KEY_LCONTROL || scancode==KEY_RCONTROL)
				scancode+=2;
			_handle_pckey( (key[scancode]?(scancode|0x80):scancode) );
		}
		write_scr(79,0,(toggle?"X":" "));
		toggle^=toggle;
	}
	else
	{
		if(key[KEY_RCONTROL])
		{
			print_infoline("Random input resumed.");
			active=TRUE;
		}
	}
}

END_OF_FUNCTION(random_input);

void init_misc(void)
{
	LOCK_FUNCTION(random_input);
	srand(rawclock());
	install_timer();
	install_int(random_input,1);
}

#endif

#endif
