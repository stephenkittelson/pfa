#include <myalleg.h>
#include <string.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "io.h"
#include "pfa.h"
#include LIBDIR"flags.h"
#ifdef DEBUG
	#include "scr.h"
	#include <stdio.h>
#endif

#ifdef DEBUG
	extern FILE *dlog_file;
#endif

#define IO_BUFFER_SIZE 30

#ifdef DEBUG
	#define IOCHKSPECKEYS(_Mflags) \
		case KEY_DOWN:\
		{\
			_Mflags|=IODN;\
			goto calculate;\
		}\
		case KEY_UP:\
		{\
			_Mflags|=IOUP;\
			goto calculate;\
		}\
		case KEY_F1:\
		{\
			continue;\
		}
#else
	#define IOCHKSPECKEYS(_Mflags) \
		case KEY_DOWN:\
		{\
			_Mflags|=IODN;\
			goto calculate;\
		}\
		case KEY_UP:\
		{\
			_Mflags|=IOUP;\
			goto calculate;\
		}\
		case KEY_F1:\
		{\
			_Mflags|=IOF1;\
			return FALSE;\
		}
#endif

/* getnulloffset: returns index posistion of null */
B16 getnulloffset(B8 *str)
{
	B16 c;
	for(c=0;str[c]!=0;c++);
	return c;
}

/* shift_string_right: shifts str right one posistion, including str[pos] */
void shift_string_right(B8 *str,B8 pos)
{ 
	B16 c;

	c=getnulloffset(str);
	for(;c>=pos;c--)
	{ 
		str[c+1]=str[c];
		if(c==0)
			break;
	}
}

/* shift_string_left: shifts str left one posistion, over-writing str[pos] */
void shift_string_left(B8 *str,B8 pos)
{
	B16 c;

	for(c=pos;;c++)
	{
		str[c]=str[c+1];
		if(str[c]==0)
			break;
	}
}

/* get_XbYY:
	receives a numerical value from user, not allowing more than max_digits to
	be entered or any chars that are not numbers, stops receiving input when a
	char in end_chars is received. if value is less than min_value, sets flag
	IOLOW to true and returns. if value is more than max_value, sets flag
	IOHIGH to true and returns. shows value in var when first called if flag
	IO_SHOW_INIT_VAL is true, which can be easily done by just passing B16
	pointer that is set to true.

	X: sign
		u - unsigned value
		s - signed value
	Y: storage size
		8
		16
*/

B8 get_ub8(B8 *var,B8 min_value,B8 max_value,B8 max_digits,B8 *end_chars,B16 *flags)
{
	#define REFRESH() \
		setcurpos(x,y);\
		printf("%s ",buf);\
		setcurpos(x+curoff,y);
	B8 x,y;
	B16 code,c,current_size=0;
	B8 curoff=0;
	B8 buf[IO_BUFFER_SIZE]={0};

	x=getcurx();
	y=getcury();
	if(IO_SHOW_INIT_VAL&(*flags))
	{
		sprintf(buf,"%u",*var);
		current_size=curoff=getnulloffset(buf);
		printf("%s",buf);
	}
	_setcursortype(_NORMALCURSOR);
	*flags=FALSE;
	*flags|=IOINS;
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			*flags|=IOALT;
			goto calculate;
		}
		switch(code>>8)
		{
			case KEY_LEFT:
			{
				if(curoff==0)
					break;
				setcurpos(getcurx()-1,y);
				curoff--;
				break;
			}
			case KEY_RIGHT:
			{
				if(buf[curoff]==0)
					break;
				curoff++;
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_INSERT:
			{
				if(*flags&IOINS)
				{
					*flags&=~IOINS;
					_setcursortype(_SOLIDCURSOR);
				}
				else
				{
					*flags|=IOINS;
					_setcursortype(_NORMALCURSOR);
				}
				break;
			}
			case KEY_DEL_PAD:
			case KEY_DEL:
			{
				if(buf[curoff]==0)
					break;
				shift_string_left(buf,curoff);
				current_size--;
				REFRESH();
				break;
			}
			case KEY_HOME:
			{
				setcurpos(x,y);
				curoff=0;
				break;
			}
			case KEY_END:
			{
				curoff=getnulloffset(buf);
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_ESC:
			{
				*flags|=IOESC;
				return FALSE;
			}
			IOCHKSPECKEYS(*flags);
			default:
			{
				break;
			}
		}
		code=code&0xFF;
		switch(code)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				if(current_size!=max_digits && *flags&IOINS)
				{
					shift_string_right(buf,curoff);
					current_size++;
					buf[curoff]=code;
					curoff++;
					REFRESH();
				}
				else if(!(*flags&IOINS))
				{
					if(buf[curoff]==0 && current_size!=max_digits)
					{
						shift_string_right(buf,curoff);
						current_size++;
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
					else if(buf[curoff]!=0)
					{
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
				}
				break;
			}
			case '\b':
			{
				if(curoff!=0)
				{
					curoff--;
					current_size--;
					shift_string_left(buf,curoff);
					REFRESH();
				}
				break;
			}
			default:
			{
				for(c=0;end_chars[c]!=0;c++)
				{
					if(code==end_chars[c])
					{
						goto calculate;
					}
				}
				break;
			}
		}
	}
	calculate:
	_setcursortype(_NORMALCURSOR);
	*var=0;
	for(c=0;buf[c]!=0;c++)
	{
		if(*var>=25)
		{
			if(*var==25)
			{
				if((buf[c]-48)>5)
				{
					*flags|=IOHIGH;
					return FALSE;
				}
			}
			else
			{
				*flags|=IOHIGH;
				return FALSE;
			}
		}
		*var*=10;
		*var+=buf[c]-48;
	}
	if(*var>max_value)
	{
		*flags|=IOHIGH;
		return FALSE;
	}
	if(*var<min_value)
	{
		*flags|=IOLOW;
		return FALSE;
	}
	return TRUE;
}

B8 get_sb8(SB8 *var,SB8 min_value,SB8 max_value,B8 max_digits,B8 *end_chars,B16 *flags)
{
	//modificiation for negatives
	#undef REFRESH()
	#define REFRESH() \
		setcurpos(x,y);\
		if(IONEG&(*flags)) printf("-%s ",buf+1);\
		else printf("%s ",buf);\
		setcurpos(x+curoff,y);
	B8 x,y;
	B16 code,c,current_size=0;
	B8 curoff=0;
	B8 buf[IO_BUFFER_SIZE]={0};

	x=getcurx();
	y=getcury();
	if(IO_SHOW_INIT_VAL&(*flags))
	{
		*flags=FALSE;
		sprintf(buf,"%d",*var);
		current_size=curoff=getnulloffset(buf);
		printf("%s",buf);
		//modification for negatives
		if(buf[0]=='-')
		{
			*flags|=IONEG;
			buf[0]='0';
		}
	}
	else
	{
		*flags=FALSE;
	}
	_setcursortype(_NORMALCURSOR);
	*flags|=IOINS;
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			*flags|=IOALT;
			goto calculate;
		}
		switch(code>>8)
		{
			case KEY_LEFT:
			{
				if(curoff==0)
					break;
				setcurpos(getcurx()-1,y);
				curoff--;
				break;
			}
			case KEY_RIGHT:
			{
				if(buf[curoff]==0)
					break;
				curoff++;
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_INSERT:
			{
				if(*flags&IOINS)
				{
					*flags&=~IOINS;
					_setcursortype(_SOLIDCURSOR);
				}
				else
				{
					*flags|=IOINS;
					_setcursortype(_NORMALCURSOR);
				}
				break;
			}
			case KEY_DEL_PAD:
			case KEY_DEL:
			{
				if(buf[curoff]==0)
					break;
				shift_string_left(buf,curoff);
				//modification for negatives
				if(curoff==0 && (*flags))
					*flags&=~IONEG;
				else
					current_size--;
				REFRESH();
				break;
			}
			case KEY_HOME:
			{
				setcurpos(x,y);
				curoff=0;
				break;
			}
			case KEY_END:
			{
				curoff=getnulloffset(buf);
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_ESC:
			{
				*flags|=IOESC;
				return FALSE;
			}
			IOCHKSPECKEYS(*flags);
			default:
			{
				break;
			}
		}
		code=code&0xFF;
		switch(code)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				if(current_size!=max_digits && *flags&IOINS)
				{
					shift_string_right(buf,curoff);
					current_size++;
					buf[curoff]=code;
					curoff++;
					REFRESH();
				}
				else if(!(*flags&IOINS))
				{
					if(buf[curoff]==0 && current_size!=max_digits)
					{
						shift_string_right(buf,curoff);
						current_size++;
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
					else if(buf[curoff]!=0)
					{
						//modification for negatives
						if(IONEG&(*flags) && curoff==0)
						{
							if(current_size!=max_digits)
							{
								*flags&=~IONEG;
								buf[0]=code;
								curoff++;
								REFRESH();
								break;
							}
							break;
						}
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
				}
				break;
			}
			case '\b':
			{
				if(curoff!=0)
				{
					curoff--;
					shift_string_left(buf,curoff);
					//modification for negatives
					if(curoff==0 && (*flags)&IONEG)
						*flags&=~IONEG;
					else
						current_size--;
					REFRESH();
				}
				break;
			}
			//modification for negatives
			case '-':
			{
				if(curoff==0)
				{
					shift_string_right(buf,curoff);
					curoff++;
					buf[0]='0';
					*flags|=IONEG;
					REFRESH();
				}
				break;
			}
			default:
			{
				for(c=0;end_chars[c]!=0;c++)
				{
					if(code==end_chars[c])
					{
						goto calculate;
					}
				}
				break;
			}
		}
	}
	calculate:
	_setcursortype(_NORMALCURSOR);
	*var=0;
	for(c=0;buf[c]!=0;c++)
	{
		if(*var>=12)
		{
			if(*var==12)
			{
				if(IONEG&(*flags))
				{
					if((buf[c]-48)>8)
					{
						*flags|=IOLOW;
						return FALSE;
					}
				}
				else
				{
					if((buf[c]-48)>7)
					{
						*flags|=IOHIGH;
						return FALSE;
					}
				}
			}
			else
			{
				if(IONEG&(*flags))
				{
					*flags|=IOLOW;
					return FALSE;
				}
				else
				{
					*flags|=IOHIGH;
					return FALSE;
				}
			}
		}
		*var*=10;
		*var+=buf[c]-48;
	}
	if(IONEG&(*flags))
	{
		*var*=-1;
	}
	if(*var>max_value)
	{
		*flags|=IOHIGH;
		return FALSE;
	}
	if(*var<min_value)
	{
		*flags|=IOLOW;
		return FALSE;
	}
	return TRUE;
}

B8 get_ub16(B16 *var,B16 min_value,B16 max_value,B8 max_digits,B8 *end_chars,B16 *flags)
{
	#undef REFRESH()
	#define REFRESH() \
		setcurpos(x,y);\
		printf("%s ",buf);\
		setcurpos(x+curoff,y);
	B8 x,y;
	B16 code,c,current_size=0;
	B8 curoff=0;
	B8 buf[IO_BUFFER_SIZE]={0};

	x=getcurx();
	y=getcury();
	if(IO_SHOW_INIT_VAL&(*flags))
	{
		sprintf(buf,"%u",*var);
		current_size=curoff=getnulloffset(buf);
		printf("%s",buf);
	}
	_setcursortype(_NORMALCURSOR);
	*flags=FALSE;
	*flags|=IOINS;
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			*flags|=IOALT;
			goto calculate;
		}
		switch(code>>8)
		{
			case KEY_LEFT:
			{
				if(curoff==0)
					break;
				setcurpos(getcurx()-1,y);
				curoff--;
				break;
			}
			case KEY_RIGHT:
			{
				if(buf[curoff]==0)
					break;
				curoff++;
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_INSERT:
			{
				if(*flags&IOINS)
				{
					*flags&=~IOINS;
					_setcursortype(_SOLIDCURSOR);
				}
				else
				{
					*flags|=IOINS;
					_setcursortype(_NORMALCURSOR);
				}
				break;
			}
			case KEY_DEL_PAD:
			case KEY_DEL:
			{
				if(buf[curoff]==0)
					break;
				shift_string_left(buf,curoff);
				current_size--;
				REFRESH();
				break;
			}
			case KEY_HOME:
			{
				setcurpos(x,y);
				curoff=0;
				break;
			}
			case KEY_END:
			{
				curoff=getnulloffset(buf);
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_ESC:
			{
				*flags|=IOESC;
				return FALSE;
			}
			IOCHKSPECKEYS(*flags);
			default:
			{
				break;
			}
		}
		code=code&0xFF;
		switch(code)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				if(current_size!=max_digits && *flags&IOINS)
				{
					shift_string_right(buf,curoff);
					current_size++;
					buf[curoff]=code;
					curoff++;
					REFRESH();
				}
				else if(!(*flags&IOINS))
				{
					if(buf[curoff]==0 && current_size!=max_digits)
					{
						shift_string_right(buf,curoff);
						current_size++;
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
					else if(buf[curoff]!=0)
					{
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
				}
				break;
			}
			case '\b':
			{
				if(curoff!=0)
				{
					curoff--;
					shift_string_left(buf,curoff);
					current_size--;
					REFRESH();
				}
				break;
			}
			default:
			{
				for(c=0;end_chars[c]!=0;c++)
				{
					if(code==end_chars[c])
					{
						goto calculate;
					}
				}
				break;
			}
		}
	}
	calculate:
	_setcursortype(_NORMALCURSOR);
	*var=0;
	for(c=0;buf[c]!=0;c++)
	{
		if(*var>=6553)
		{
			if(*var==6553)
			{
				if((buf[c]-48)>5)
				{
					*flags|=IOHIGH;
					return FALSE;
				}
			}
			else
			{
				*flags|=IOHIGH;
				return FALSE;
			}
		}
		*var*=10;
		*var+=buf[c]-48;
	}
	if(*var>max_value)
	{
		*flags|=IOHIGH;
		return FALSE;
	}
	if(*var<min_value)
	{
		*flags|=IOLOW;
		return FALSE;
	}
	return TRUE;
}

B8 get_sb16(SB16 *var,SB16 min_value,SB16 max_value,B8 max_digits,B8 *end_chars,B16 *flags)
{
	//modificiation for negatives
	#undef REFRESH()
	#define REFRESH() \
		setcurpos(x,y);\
		if(IONEG&(*flags)) printf("-%s ",buf+1);\
		else printf("%s ",buf);\
		setcurpos(x+curoff,y);
	B8 x,y;
	B16 code,c,current_size=0;
	B8 curoff=0;
	B8 buf[IO_BUFFER_SIZE]={0};

	x=getcurx();
	y=getcury();
	_setcursortype(_NORMALCURSOR);
	*flags|=IOINS;
	if(IO_SHOW_INIT_VAL&(*flags))
	{
		*flags=FALSE;
		sprintf(buf,"%d",*var);
		current_size=curoff=getnulloffset(buf);
		printf("%s",buf);
		//modification for negatives
		if(buf[0]=='-')
		{
			*flags|=IONEG;
			buf[0]='0';
		}
	}
	else
	{
		*flags=FALSE;
	}
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			*flags|=IOALT;
			goto calculate;
		}
		switch(code>>8)
		{
			case KEY_LEFT:
			{
				if(curoff==0)
					break;
				setcurpos(getcurx()-1,y);
				curoff--;
				break;
			}
			case KEY_RIGHT:
			{
				if(buf[curoff]==0)
					break;
				curoff++;
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_INSERT:
			{
				if(*flags&IOINS)
				{
					*flags&=~IOINS;
					_setcursortype(_SOLIDCURSOR);
				}
				else
				{
					*flags|=IOINS;
					_setcursortype(_NORMALCURSOR);
				}
				break;
			}
			case KEY_DEL_PAD:
			case KEY_DEL:
			{
				if(buf[curoff]==0)
					break;
				shift_string_left(buf,curoff);
				//modification for negatives
				if(curoff==0 && (*flags)&IONEG)
					*flags&=~IONEG;
				else
					current_size--;
				REFRESH();
				break;
			}
			case KEY_HOME:
			{
				setcurpos(x,y);
				curoff=0;
				break;
			}
			case KEY_END:
			{
				curoff=getnulloffset(buf);
				setcurpos(x+curoff,y);
				break;
			}
			case KEY_ESC:
			{
				*flags|=IOESC;
				return FALSE;
			}
			IOCHKSPECKEYS(*flags);
			default:
			{
				break;
			}
		}
		code=code&0xFF;
		switch(code)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				if(current_size!=max_digits && *flags&IOINS)
				{
					shift_string_right(buf,curoff);
					current_size++;
					buf[curoff]=code;
					curoff++;
					REFRESH();
				}
				else if(!(*flags&IOINS))
				{
					if(buf[curoff]==0 && current_size!=max_digits)
					{
						shift_string_right(buf,curoff);
						current_size++;
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
					else if(buf[curoff]!=0)
					{
						//modification for negatives
						if(IONEG&(*flags) && curoff==0)
						{
							if(current_size!=max_digits)
							{
								*flags&=~IONEG;
								buf[0]=code;
								curoff++;
								REFRESH();
								break;
							}
							break;
						}
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
				}
				break;
			}
			case '\b':
			{
				if(curoff!=0)
				{
					curoff--;
					shift_string_left(buf,curoff);
					//modification for negatives
					if(curoff==0 && (*flags)&IONEG)
						*flags&=~IONEG;
					else
						current_size--;
					REFRESH();
				}
				break;
			}
			//modification for negatives
			case '-':
			{
				if(curoff==0)
				{
					shift_string_right(buf,curoff);
					curoff++;
					buf[0]='0';
					*flags|=IONEG;
					REFRESH();
				}
				break;
			}
			default:
			{
				for(c=0;end_chars[c]!=0;c++)
				{
					if(code==end_chars[c])
					{
						goto calculate;
					}
				}
				break;
			}
		}
	}
	calculate:
	_setcursortype(_NORMALCURSOR);
	*var=0;
	for(c=0;buf[c]!=0;c++)
	{
		if(*var>=3276)
		{
			if(*var==3276)
			{
				if(IONEG&(*flags))
				{
					if((buf[c]-48)>8)
					{
						*flags|=IOLOW;
						return FALSE;
					}
				}
				else
				{
					if((buf[c]-48)>7)
					{
						*flags|=IOHIGH;
						return FALSE;
					}
				}
			}
			else
			{
				if(IONEG&(*flags))
				{
					*flags|=IOLOW;
					return FALSE;
				}
				else
				{
					*flags|=IOHIGH;
					return FALSE;
				}
			}
		}
		*var*=10;
		*var+=buf[c]-48;
	}
	if(IONEG&(*flags))
	{
		*var*=-1;
	}
	if(*var>max_value)
	{
		*flags|=IOHIGH;
		return FALSE;
	}
	if(*var<min_value)
	{
		*flags|=IOLOW;
		return FALSE;
	}
	return TRUE;
}

B8 get_float(double *var,double min_value,double max_value,B8 max_bdsize,B8 max_adsize,B8 *end_chars,B16 *flags,const B8 *format_str)
{
	//modificiation for negatives
	#undef REFRESH()
	#define REFRESH() \
		setcurpos(x,y);\
		if(IONEG&(*flags)) printf("-%s ",buf+1);\
		else printf("%s ",buf);\
		setcurpos(x+curoff,y);
	B8 x,y;
	B16 code,c,current_adsize=0,current_bdsize=0;
	B8 curoff=0;
	B8 buf[IO_BUFFER_SIZE]={0};
	float mod=0;
	float tolerance;
	B8 *max_size;
	B16 *current_size;

	#ifdef DEBUG
		//#define DEBUG_GETFLOAT
		//#define DEBUG_GETFLOAT_EXTENDED
		//#define DEBUG_TRAP
	#endif

	#ifdef DEBUG_GETFLOAT
		B8 ads,bds;
		B8 *dec_loc;
		static B8 dlog_c=0;
		static B32 dlog_rewindc=0;
		static B32 dlog_rcwraps=0;
	#endif

	max_size=&max_bdsize;
	current_size=&current_bdsize;
	x=getcurx();
	y=getcury();
	if(IO_SHOW_INIT_VAL&(*flags))
	{
		#ifndef DEBUG_GETFLOAT
		show_init_val:
		#endif
		//modification for both
		*flags=IOINS;
		//modification for decimals
		sprintf(buf,format_str,*var);
		*current_size=curoff=getnulloffset(buf);
		printf("%s",buf);
		//modification for negatives
		if(buf[0]=='-')
		{
			current_bdsize--;
			buf[0]='0';
			*flags|=IONEG;
		}
		//modification for decimals
		if(strchr(buf,'.'))
		{
			current_bdsize--;
			*flags|=IODECHIT;
			current_adsize=strlen(strchr(buf,'.')+1);
			current_bdsize-=current_adsize;
			current_size=&current_adsize;
			max_size=&max_adsize;
		}
		#ifndef DEBUG_GETFLOAT
		if(current_bdsize>max_bdsize || current_adsize>max_adsize)
		{
			*var=0;
			#ifdef DEBUG
				write_logs(0,1,"io.c:get_float:variable passed does not meet its own size requirements! nullifying var...\n");
			#endif
			goto show_init_val;
		}
		#endif
	}
	else
	{
		//modification for both
		*flags=IOINS;
	}
	_setcursortype(_NORMALCURSOR);
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			*flags|=IOALT;
			goto calculate;
		}
		switch(code>>8)
		{
			case KEY_LEFT:
			{
				if(curoff==0)
					continue;
				#ifdef DEBUG_GETFLOAT
					write_logs(0,1,"io.c:get_float:last_op: key left\n");
				#endif
				setcurpos(getcurx()-1,y);
				curoff--;
				if(buf[curoff]=='.')
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1," key left, decimal hop\n");
					#endif
					current_size=&current_bdsize;
					max_size=&max_bdsize;
				}
				#ifndef DEBUG_GETFLOAT
					continue;
				#else
					goto dlog_entry;
				#endif
			}
			case KEY_RIGHT:
			{
				if(buf[curoff]==0)
					continue;
				if(buf[curoff]=='.')
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: key right, decimal hop\n");
					#endif
					current_size=&current_adsize;
					max_size=&max_adsize;
				}
				#ifdef DEBUG_GETFLOAT
					write_logs(0,1,"io.c:get_float:last_op: key right\n");
				#endif
				curoff++;
				setcurpos(x+curoff,y);
				#ifndef DEBUG_GETFLOAT
					continue;
				#else
					goto dlog_entry;
				#endif
			}
			case KEY_INSERT:
			{
				if(*flags&IOINS)
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: switch to overwrite mode\n");
					#endif
					*flags&=~IOINS;
					_setcursortype(_SOLIDCURSOR);
				}
				else
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: switch to insert mode\n");
					#endif
					*flags|=IOINS;
					_setcursortype(_NORMALCURSOR);
				}
				#ifndef DEBUG_GETFLOAT
					continue;
				#else
					goto dlog_entry;
				#endif
			}
			case KEY_BACKSPACE:
			{
				if(curoff==0)
					continue;
				#ifdef DEBUG_GETFLOAT
					write_logs(0,1,"io.c:get_float:last_op: backspace\n");
				#endif
				curoff--;
				if(buf[curoff]=='.')
				{
					current_size=&current_bdsize;
					max_size=&max_bdsize;
				}
			}
			case KEY_DEL_PAD:
			case KEY_DEL:
			{
				if(buf[curoff]==0)
					continue;
				//modification for decimals
				if(buf[curoff]=='.')
				{
					if(current_bdsize+current_adsize<=max_bdsize)
					{
						#ifdef DEBUG_GETFLOAT
							write_logs(0,1,"io.c:get_float:last_op: rm decimal\n");
						#endif
						*flags&=~IODECHIT;
						max_size=&max_bdsize;
						current_size=&current_bdsize;
						current_bdsize+=current_adsize;
						current_adsize=0;
					}
					else
					{
						#ifdef DEBUG_GETFLOAT
							write_logs(0,1," rm failed\n");
						#endif
						continue;
					}
				}
				//modification for negatives
				else if(curoff==0 && (*flags)&IONEG)
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: rm '-'\n");
					#endif
					*flags&=~IONEG;
				}
				else
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: general rm\n");
					#endif
					(*current_size)--;
				}
				#ifdef DEBUG_GETFLOAT
					write_logs(0,1," rm: removal\n");
				#endif
				shift_string_left(buf,curoff);
				REFRESH();
				#ifndef DEBUG_GETFLOAT
					continue;
				#else
					goto dlog_entry;
				#endif
			}
			case KEY_HOME:
			{
				#ifdef DEBUG_GETFLOAT
					write_logs(0,1,"io.c:get_float:last_op: home\n");
				#endif
				setcurpos(x,y);
				curoff=0;
				current_size=&current_bdsize;
				max_size=&max_bdsize;
				#ifndef DEBUG_GETFLOAT
					continue;
				#else
					goto dlog_entry;
				#endif
			}
			case KEY_END:
			{
				#ifdef DEBUG_GETFLOAT
					write_logs(0,1,"io.c:get_float:last_op: end\n");
				#endif
				curoff=getnulloffset(buf);
				setcurpos(x+curoff,y);
				if(strchr(buf,'.'))
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1," end: decimal hop\n");
					#endif
					current_size=&current_adsize;
					max_size=&max_adsize;
				}
				#ifndef DEBUG_GETFLOAT
					continue;
				#else
					goto dlog_entry;
				#endif
			}
			case KEY_ESC:
			{
				#ifdef DEBUG_TRAP
					continue;
				#else
					*flags|=IOESC;
					return FALSE;
				#endif
			}
			IOCHKSPECKEYS(*flags);
			default:
			{
				break;
			}
		}
		code=code&0xFF;
		switch(code)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				if(((*flags)&IOINS) || (buf[curoff]==0))
				{
					if(curoff==0 && IONEG&(*flags))
						break;
					if(*current_size!=*max_size)
					{
						#ifdef DEBUG_GETFLOAT
							write_logs(0,1,"io.c:get_float:last_op: general insert\n");
						#endif
						shift_string_right(buf,curoff);
						(*current_size)++;
						buf[curoff]=code;
						curoff++;
						REFRESH();
					}
				}
				else
				{
					//modification for negatives
					if(curoff==0 && IONEG&(*flags))
					{
						if(*current_size!=*max_size)
						{
							#ifdef DEBUG_GETFLOAT
								write_logs(0,1,"io.c:get_float:last_op: !IOINS, curoff==0, !IONEG\n");
							#endif
							*flags&=~IONEG;
							buf[0]=code;
							(*current_size)++;
							curoff++;
							REFRESH();
						}
						break;
					}
					if(buf[curoff]=='.')
					{
						if(current_adsize+current_bdsize+1<=max_bdsize)
						{
							#ifdef DEBUG_GETFLOAT
								write_logs(0,1,"io.c:get_float:last_op: overwrite decimal, !IOINS\n");
							#endif
							current_bdsize+=current_adsize+1;
							current_adsize=0;
							*flags&=~IODECHIT;
						}
						else
						{
							break;
						}
					}
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: actual overwrite\n");
					#endif
					buf[curoff]=code;
					curoff++;
					REFRESH();
				}
				break;
			}
			//modification for negatives
			case '-':
			{
				if(min_value>-0.004)
					continue;
				if(curoff==0 && !((*flags)&IONEG))
				{
					#ifdef DEBUG_GETFLOAT
						write_logs(0,1,"io.c:get_float:last_op: '-'\n");
					#endif
					shift_string_right(buf,curoff);
					curoff++;
					buf[0]='0';
					*flags|=IONEG;
					REFRESH();
				}
				break;
			}
			//modification for decimals
			case '.':
			{
				if(!(IODECHIT&(*flags)))
				{
					if(IOINS&(*flags))
					{
						if(curoff==0 && IONEG&(*flags))
							break;
						if(strlen(buf+curoff)<=max_adsize)
						{
							#ifdef DEBUG_GETFLOAT
								write_logs(0,1,"io.c:get_float:last_op: (insert) '.'\n");
							#endif
							*flags|=IODECHIT;
							current_size=&current_adsize;
							max_size=&max_adsize;
							shift_string_right(buf,curoff);
							buf[curoff]='.';
							current_adsize=strlen(buf+curoff+1);
							current_bdsize-=current_adsize;
							curoff++;
							REFRESH();
						}
					}
					else
					{
						if(strlen(buf+curoff+1)<=max_adsize)
						{
							#ifdef DEBUG_GETFLOAT
								write_logs(0,1,"io.c:get_float:last_op: (overwrite) '.'\n");
							#endif
							*flags|=IODECHIT;
							if(curoff==0 && IONEG&(*flags))
								*flags&=~IONEG;
							current_size=&current_adsize;
							max_size=&max_adsize;
							buf[curoff]='.';
							current_adsize=strlen(buf+curoff+1);
							current_bdsize-=current_adsize;
							curoff++;
							REFRESH();
						}
					}
				}
				else if(buf[curoff]=='.')
				{
					setcurpos(x+(++curoff),y);
					max_size=&max_adsize;
					current_size=&current_adsize;
				}
				break;
			}
			default:
			{
				for(c=0;end_chars[c]!=0;c++)
				{
					if(code==end_chars[c])
					{
						goto calculate;
					}
				}
				continue;
			}
		}
		#ifdef DEBUG_GETFLOAT
			dlog_entry:
			#define PANIC(_Mstr) \
				write_logs(0,1,"io.c:get_float: PANIC!! " _Mstr "\n\n\n");\
				exit(EXIT_DEVELOPMENTERROR);

			dlog_c++;
			#ifdef DEBUG_GETFLOAT_EXTENDED
				write_logf(0," max_bds: %u max_ads: %u curoff: %u cur_bds: %u cur_ads: %u\n"
						 " buf: [%s]\n"
						 " flags: %X\n\n",
					max_bdsize,max_adsize,curoff,current_bdsize,current_adsize,
					buf,*flags);
			#endif
			if(current_adsize>max_adsize || current_bdsize>max_bdsize)
			{
				PANIC("cur > max");
			}
			if(current_size==&current_bdsize)
			{
				dec_loc=(B8 *)strchr(buf,'.');
				if(dec_loc && dec_loc<(&buf[curoff]) )
				{
					PANIC("bd -> ad");
				}
			}
			else
			{
				dec_loc=(B8 *)strchr(buf,'.');
				if(dec_loc && dec_loc>(&buf[curoff]) )
				{
					PANIC("ad ->bd");
				}
			}
			dec_loc=strchr(buf,'.');
			if(dec_loc)
				ads=strlen(dec_loc+1);
			else
				ads=0;
			bds=strlen(buf)-ads;
			if(dec_loc)
				bds--;
			if(IONEG&(*flags))
				bds--;
			if(bds!=current_bdsize || ads!=current_adsize)
			{
				PANIC("double chk, bds|ads!=cur");
			}
			if(dlog_c==100)
			{
				dlog_c=0;
				write_logs(0,1,"\n\n\n\n");
				rewind(dlog_file);
				if(dlog_rewindc==0xFFFFFFFFL)
				{
					dlog_rcwraps++;
				}
				dlog_rewindc++;
				write_logf(0,"PFA " VSTR "\nio.c:get_float: %u dlog rewinds %u dlog rewind count wraps\n\n",dlog_rewindc,dlog_rcwraps);
			}
			#ifdef DEBUG_TRAP
				calculate:
			#endif
		#endif
	}
	#ifndef DEBUG_TRAP
		calculate:
	#endif
	*var=0;
	*flags&=~IODECHIT;
	for(c=0;buf[c]!=0;c++)
	{
		if(buf[c]=='.')
		{
			*flags|=IODECHIT;
			mod=0.1;
		}
		else
		{
			if(!(IODECHIT&(*flags)))
			{
				*var*=10;
				*var+=buf[c]-48;
			}
			else
			{
				*var+=(buf[c]-48)*mod;
				mod*=0.1;
			}
		}
	}
	tolerance=0.5;
	for(c=0;c<max_adsize;c++)
		tolerance*=0.1;
	if(IONEG&(*flags))
		*var*=-1;
	if((*var)<(min_value-tolerance))
	{
		*flags|=IOLOW;
		return FALSE;
	}
	if(*var>=max_value+tolerance)
	{
		*flags|=IOHIGH;
		return FALSE;
	}
	return TRUE;
}

B8 get_string(B8 *str,B16 max_size,B8 *end_chars,B8 *inv_chars,B16 *flags)
{
	#undef REFRESH()
	#define REFRESH() \
		setcurpos(x,y);\
		printf("%s ",str);\
		setcurpos(x+curoff,y);
	B8 x,y;
	B16 code,c,current_size=0;
	B8 curoff=0;
	B8 *pos;

	x=getcurx();
	y=getcury();
	if(IO_SHOW_INIT_VAL&(*flags))
	{
		current_size=curoff=getnulloffset(str);
		printf("%s",str);
	}
	else
	{
		str[0]=0;
	}
	_setcursortype(_NORMALCURSOR);
	*flags=FALSE;
	*flags|=IOINS;
	while(1)
	{
		code=readkey();
		if(key_shifts&KB_ALT_FLAG)
		{
			*flags|=IOALT;
			goto calculate;
		}
		switch(code>>8)
		{
			case KEY_LEFT:
			{
				if(curoff==0)
					continue;
				setcurpos(getcurx()-1,y);
				curoff--;
				continue;
			}
			case KEY_RIGHT:
			{
				if(str[curoff]==0)
					continue;
				curoff++;
				setcurpos(x+curoff,y);
				continue;
			}
			case KEY_INSERT:
			{
				if(*flags&IOINS)
				{
					*flags&=~IOINS;
					_setcursortype(_SOLIDCURSOR);
				}
				else
				{
					*flags|=IOINS;
					_setcursortype(_NORMALCURSOR);
				}
				continue;
			}
			case KEY_DEL_PAD:
			case KEY_DEL:
			{
				if(str[curoff]==0)
					continue;
				shift_string_left(str,curoff);
				current_size--;
				REFRESH();
				continue;
			}
			case KEY_HOME:
			{
				setcurpos(x,y);
				curoff=0;
				continue;
			}
			case KEY_END:
			{
				curoff=getnulloffset(str);
				setcurpos(x+curoff,y);
				continue;
			}
			case KEY_ESC:
			{
				*flags|=IOESC;
				return FALSE;
			}
			IOCHKSPECKEYS(*flags);
			default:
			{
				break;
			}
		}
		code&=0xFF;
		switch(code)
		{
			case '\t':
			{
				//todo add support for 'tab' key
				break;
			}
			case '\b':
			{
				if(curoff!=0)
				{
					curoff--;
					current_size--;
					shift_string_left(str,curoff);
					REFRESH();
				}
				break;
			}
			default:
			{
				if(code==0)
					break;
				for(c=0;end_chars[c]!=0;c++)
				{
					if(code==end_chars[c])
					{
						return TRUE;
					}
				}
				pos=inv_chars;
				while(*pos)
				{
					if(*pos==code)
						break;
					pos++;
				}
				if(*pos)
					break;
				if(current_size!=max_size-1 && *flags&IOINS)
				{
					shift_string_right(str,curoff);
					current_size++;
					str[curoff]=code;
					curoff++;
					REFRESH();
				}
				else if(!(*flags&IOINS))
				{
					if(str[curoff]==0 && current_size!=max_size-1)
					{
						current_size++;
						str[curoff]=code;
						curoff++;
						str[curoff]=0;
						REFRESH();
					}
					else if(str[curoff]!=0)
					{
						str[curoff]=code;
						curoff++;
						REFRESH();
					}
				}
				break;
			}
		}
	}
	calculate:
	return TRUE;
}

void fget_string(B8 *str,B16 max_size,B8 *end_chars,FILE *file)
{
	B16 c=0;
	B16 c2;

	while(1)
	{
		str[c]=fgetc(file);
		switch(str[c])
		{
			case '\b':
			{
				#ifdef DEBUG
					write_logs(0,1,"fget_string: error: '\\b' in file\n");
				#endif
				str[c]=0;
				if(c==0)
					break;
				str[--c]=0;
				break;
			}
			default:
			{
				for(c2=0;end_chars[c2]!=0;c2++)
				{
					if(str[c]==end_chars[c2])
					{
						str[c]=0;
						return;
					}
				}
				if(c==max_size-1)
					break;
				c++;
				break;
			}
		}
	}
}
