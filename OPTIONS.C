#include <myalleg.h>
#include <string.h>
#include <sys/stat.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "tl.h"
#include "totals.h"
#include "options.h"
#include "cl.h"
#include "scr.h"
#include "io.h"
#include "file.h"
#include "list.h"
#include LIBDIR"flags.h"

extern struct TOTALS totals;
extern struct COLOR user_colors;
extern struct LIST tl;

struct OPTIONS options;

struct OPS_INLOCS
{
	B8 x;
	B8 y;
	B8 *color;
	void (*set_value)(B8 *,B8 *);
};

//input locations for optionscr
	#define SCR_OS_CFX 11
	#define SCR_OS_CFY 4
	#define SCR_OS_TITHX 16
	#define SCR_OS_TITHY 9
	#define SCR_OS_FILEX 25
	#define SCR_OS_FILEY 10
	#define SCR_OS_TUTORX 10
	#define SCR_OS_TUTORY 12

B8 *optionscr=
{
	"                           Options                                              "
	"                                                                                "
	"     Colors                                                                     "
	"           F  B                                                                 "
	"General  :                                                                      "
	"Selected :                                                                      "
	"Highlight:                                                                      "
	"                                                                                "
	"     Tithing Method                                                             "
	"Calculate using                                                                 "
	"File to load at startup:                                                        "
	"                                                                                "
	"Tutorial:                                                                       "
	"                                                                                "
	"   Color values                                                                 "
	"                                                                                "
	"    F  B (F color of 15)                                                        "
	"00: X  X                                                                        "
	"01: X  X                                                                        "
	"02: X  X                                                                        "
	"03: X  X                                                                        "
	"04: X  X                                                                        "
	"05: X  X                                                                        "
	"06: X  X                                                                        "
	"07: X  X                                                                        "
	"08: X  X                                                                        "
	"09: X  X                                                                        "
	"10: X  X                                                                        "
	"11: X  X                                                                        "
	"12: X  X                                                                        "
	"13: X  X                                                                        "
	"14: X  X                                                                        "
	"15: X  X "
};

B8 *optionscr_tutorial=
{
	"To move to a different color, press either the 'up' or 'down' keys or the       "
	"'enter' key.                                                                    "
	"                                                                                "
	"To change the tithing calculation method, press the space-bar when the cursor is"
	"at that input location.                                                         "
	"                                                                                "
	"To toggle the tutorial on or off, press space-bar while at that input location. "
	"                                                                                "
	"To accept the changes made and return to the previous screen, hold down the     "
	"'alt' key and press 'q'. To discard the changes made and return to the previous "
	"screen, press the escape key.                                                   "
	"                                                                                "
	"To move to the Tithing Method input location more quickly, hold down 'alt' and  "
	"press 't'.                                                                      "
};

void display_optionscr(void)
{
	B8 color,y;
	B16 empty=0x0720;
	B16 c;

	//we don't want this screen to conform to the user-chosen colors, just in
	//case they chose the same color for foreground and background!
	for(c=0;c<8000;c+=2)
		movedata(_my_ds(),(int)&empty,_dos_ds,0xB8000+c,2);

	write_scr(0,0,optionscr);
	if(options.flags&TUTORIAL)
		write_scr(0,34,optionscr_tutorial);
	HIGHLIGHT(5,8);
	for(color=0x00,y=17;y<33;y++,color++)
	{
		set_text_color(4,y,1,color);
		set_text_color(7,y,1,(color<<4)+0xF);
	}
}

void recalculate_tithowed(void)
{
	struct TRANS *t;

	t=(struct TRANS *)tl.first;
	totals.tith_owed=0;
	while(t)
	{
		if(t->categ->id!=1)
		{
			if(TITH_GROSS&options.flags)
			{
				totals.tith_owed+=t->gross_income*((double)t->per_to_tith/100);
			}
			else 
				totals.tith_owed+=t->value*((double)t->per_to_tith/100);
		}
		else 
		{
			totals.tith_owed+=t->value;
		}
		t=t->next;
	}
}

void init_options(void)
{
	int fhandle;

	#define SET_DEFAULT_OPS() \
			options.sfn[0]=0;\
			options.flags=TITH_GROSS|TUTORIAL;\
			user_colors.general=0x07;\
			user_colors.selected=0x70;\
			user_colors.highlight=0x0F;
	#define READ_OP(_Mdest,_Msize) \
		if(read(fhandle,_Mdest,_Msize)<1)\
		{\
			SET_DEFAULT_OPS();\
			close(fhandle);\
			return;\
		}
	if((fhandle=open("pfa.dat",O_RDONLY|O_BINARY,S_IWUSR))>=0)
	{
		READ_OP(options.sfn,FILENAME_SIZE);
		READ_OP(&options.flags,1);
		READ_OP(&user_colors.general,1);
		READ_OP(&user_colors.selected,1);
		READ_OP(&user_colors.highlight,1);
		read_filehist(fhandle);
		close(fhandle);
	}
	else
	{
		SET_DEFAULT_OPS();
	}
}

void write_ops(void)
{
	int fhandle;

	#define WRITE_OP(_Msrc,_Msize) \
		if(write(fhandle,_Msrc,_Msize)<1)\
		{\
			print_infoline("Unable to modify 'pfa.dat': %s\nPress any key to continue...",sys_errlist[errno]);\
			close(fhandle);\
			clear_keybuf();\
			while(!keypressed());\
			return;\
		}

	#ifdef RANDOM_INPUT
		print_infoline("Cannot write options data while in random input mode.");
		return;
	#endif
	if((fhandle=open("pfa.dat",O_RDWR|O_CREAT|O_BINARY,S_IWUSR))<0)
	{
		print_infoline("Unable to open/create 'pfa.dat': %s\nPress any key to continue...",sys_errlist[errno]);
		clear_keybuf();
		while(!keypressed());
		return;
	}
	WRITE_OP(options.sfn,FILENAME_SIZE);
	WRITE_OP(&options.flags,1);
	WRITE_OP(&user_colors.general,1);
	WRITE_OP(&user_colors.selected,1);
	WRITE_OP(&user_colors.highlight,1);
	close(fhandle);
}

void set_low_4bits(B8 *src,B8 *dest)
{
	*dest&=0xF0;
	*dest+=*src;
}

void set_high_4bits(B8 *src,B8 *dest)
{
	*dest&=0x0F;
	*dest+=(*src)<<4;
}

void edit_options(void)
{
	SB8 input_locnum=0;
	B16 flags;
	B8 ttith_gross=TITH_GROSS&options.flags;
	B8 tv;
	B8 tf=options.flags;
	B16 keycode;
	B8 tsfn[FILENAME_SIZE];
	struct COLOR tc;
	struct OPS_INLOCS inloc[6]=
	{
		{SCR_OS_CFX  ,SCR_OS_CFY  ,&(tc.general)  ,set_low_4bits },
		{SCR_OS_CFX+3,SCR_OS_CFY  ,&(tc.general)  ,set_high_4bits},
		{SCR_OS_CFX  ,SCR_OS_CFY+1,&(tc.selected) ,set_low_4bits },
		{SCR_OS_CFX+3,SCR_OS_CFY+1,&(tc.selected) ,set_high_4bits},
		{SCR_OS_CFX  ,SCR_OS_CFY+2,&(tc.highlight),set_low_4bits },
		{SCR_OS_CFX+3,SCR_OS_CFY+2,&(tc.highlight),set_high_4bits}
	};

	display_optionscr();
	strncpy(tsfn,options.sfn,FILENAME_SIZE);
	COPY_COLOR(tc.,user_colors.);
	for(tv=0;tv<6;tv++)
	{
		setcurpos(inloc[tv].x,inloc[tv].y);
		if(inloc[tv].x==SCR_OS_CFX)
			printf("%u",(B8)(*(inloc[tv].color)&0x0F));
		else
			printf("%u",(B8)(*(inloc[tv].color)>>4));
	}
	if(TITH_GROSS&tf)
		write_scr(SCR_OS_TITHX,SCR_OS_TITHY,"gross income");
	else
		write_scr(SCR_OS_TITHX,SCR_OS_TITHY,"net income");
	write_scr(SCR_OS_FILEX,SCR_OS_FILEY,tsfn);
	write_scr(SCR_OS_TUTORX,SCR_OS_TUTORY,((tf&TUTORIAL)?"ON":"OFF"));
	while(1)
	{
		clear_keybuf();
		if(input_locnum<6)
		{
			while(1)
			{
				flags=1;
				setcurpos(inloc[input_locnum].x,inloc[input_locnum].y);
				write_scr(inloc[input_locnum].x,inloc[input_locnum].y,"  ");
				if(inloc[input_locnum].x==SCR_OS_CFX)
					tv=*(inloc[input_locnum].color)&0x0F;
				else
					tv=*(inloc[input_locnum].color)>>4;
				if(get_ub8(&tv,0,15,2,"\n\r",&flags))
				{ 
					inloc[input_locnum].set_value(&tv,inloc[input_locnum].color);
					if(IOALT&flags)
					{
						print_infoline("Alt-");
						while(key[KEY_ALT])
						{
							if(key[KEY_Q])
							{
								goto alt_q_exit;
							}
							if(key[KEY_T])
							{
								input_locnum=6;
								break;
							}
						}
						print_infoline("");
					}
					else if(IOUP&flags)
					{
						input_locnum-=2;
						if(input_locnum<0)
							input_locnum=8;
					}
					else if(IODN&flags)
					{
						if(input_locnum<5)
							input_locnum+=2;
						else //input_locnum==5
							input_locnum++;
					}
					else
					{
						input_locnum++;
					}
					if(input_locnum>5)
						break;
				}
				else
				{
					if(IOESC&flags)
					{
						clear_keybuf();
						hidecur();
						return;
					}
				}
			}
		}
		else if(input_locnum==6)
		{
			setcurpos(SCR_OS_TITHX,SCR_OS_TITHY);
			while(1)
			{
				keycode=readkey();
				if(key_shifts&KB_ALT_FLAG)
				{
					if(keycode>>8==KEY_Q)
					{
						goto alt_q_exit;
					}
					else if(keycode>>8==KEY_T)
					{
						input_locnum=6;
						break;
					}
				}
				else
				{
					switch(keycode>>8)
					{
						case KEY_SPACE:
						{
							tf^=TITH_GROSS;
							if(TITH_GROSS&tf)
								write_scr(SCR_OS_TITHX,SCR_OS_TITHY,"gross income");
							else
								write_scr(SCR_OS_TITHX,SCR_OS_TITHY,"net income  ");
							continue;
						}
						case KEY_ENTER:
						{
							input_locnum++;
							break;
						}
						case KEY_ESC:
						{
							clear_keybuf();
							hidecur();
							return;
						}
						case KEY_UP:
						{
							input_locnum--;
							break;
						}
						case KEY_DOWN:
						{
							input_locnum++;
							break;
						}
						default:
						{
							continue;
						}
					}
					break;
				}
			}
		}
		else if(input_locnum==7)
		{
			setcurpos(SCR_OS_FILEX,SCR_OS_FILEY);
			flags=1;
			if(get_string(tsfn,FILENAME_SIZE,"\n\r",INV_FN_CHARS,&flags))
			{ 
				if(IOALT&flags)
				{
					print_infoline("Alt-");
					while(key[KEY_ALT])
					{
						if(key[KEY_Q])
						{
							goto alt_q_exit;
						}
						if(key[KEY_T])
						{
							input_locnum=6;
							break;
						}
					}
					print_infoline("");
				}
				else if(IOUP&flags)
				{
					input_locnum--;
				}
				else
				{
					input_locnum++;
				}
			}
			else
			{
				clear_keybuf();
				hidecur();
				return;
			}
		}
		else if(input_locnum==8)
		{
			setcurpos(SCR_OS_TUTORX,SCR_OS_TUTORY);
			while(1)
			{
				keycode=readkey();
				if(key_shifts&KB_ALT_FLAG)
				{
					if(keycode>>8==KEY_Q)
					{
						goto alt_q_exit;
					}
					else if(keycode>>8==KEY_T)
					{
						input_locnum=6;
						break;
					}
				}
				else
				{
					switch(keycode>>8)
					{
						case KEY_SPACE:
						{
							tf^=TUTORIAL;
							write_scr(SCR_OS_TUTORX,SCR_OS_TUTORY,((tf&TUTORIAL)?"ON ":"OFF"));
							continue;
						}
						case KEY_ENTER:
						{
							input_locnum=0;
							break;
						}
						case KEY_ESC:
						{
							clear_keybuf();
							hidecur();
							return;
						}
						case KEY_UP:
						{
							input_locnum--;
							break;
						}
						case KEY_DOWN:
						{
							input_locnum=0;
							break;
						}
						default:
						{
							continue;
						}
					}
					break;
				}
			}
		}
		else
		{
			#ifdef DEBUG
				write_logs(0,1,"options.c:edit_options:else hit for input_locnum\n");
			#endif
			input_locnum=0;
		}
	}
	alt_q_exit:
	if((TITH_GROSS&tf)!=ttith_gross)
		recalculate_tithowed();
	strncpy(options.sfn,tsfn,FILENAME_SIZE);
	COPY_COLOR(user_colors.,tc.);
	options.flags=tf;
	clear_keybuf();
	write_ops();
	hidecur();
	return;
}
