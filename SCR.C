#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "tl.h"
#include "scr.h"
#include "cl.h"
#include "options.h"
#include "dcl.h"

extern struct OPTIONS options;

struct COLOR user_colors;
B8 scr_cleaned=FALSE;

void write_scr(B8 x,B8 y,B8 *str)
{
	B16 c1,c2;

	for(c1=CALC_SCR_OFFSET(x,y),c2=0;c1<8000 && *(str+c2)!=0;c1+=2,c2++)
	{
		movedata(_my_ds(),(int)(str+c2),_dos_ds,0xB8000+c1,1);
	}
}

void clear_scr(void)
{
	B16 empty; //0x0720 is default value
	B16 c;

	empty=(user_colors.general<<8)+0x20;
	for(c=0;c<8000;c+=2)
		movedata(_my_ds(),(int)&empty,_dos_ds,0xB8000+c,2);
}

void init_scr(void)
{
	__dpmi_regs reg;

	_set_screen_lines(50);
	// taken from FED, sets bright backgrounds instead of blinking
	reg.x.ax = 0x1003;
	reg.x.bx = 0;
	__dpmi_int(0x10, &reg);
	//
	hidecur();
	atexit(clean_scr);
}

void clean_scr(void)
{
	B16 empty=0x0720;
	B16 c;

	if(scr_cleaned)
		return;
	scr_cleaned=TRUE;
	for(c=0;c<8000;c+=2)
		movedata(_my_ds(),(int)&empty,_dos_ds,0xB8000+c,2);
	setcurpos(0,0);
	normcur();
	//set it back to what is was before we started
	normvideo();
}

void print_infoline(B8 *str,...)
{
	va_list ap;

	write_scr(0,48,"                                                                                                                                                                ");
	setcurpos(0,48);
	va_start(ap,str);
	vprintf(str,ap);
	fflush(stdout);
	va_end(ap);
}

void set_text_color(B8 x,B8 y,B16 length,B8 color)
{
	B16 offset,c=0;

	offset=CALC_SCR_OFFSET(x,y)+1;
	for(;c<length && offset<8000;c++,offset+=2)
	{
		movedata(_my_ds(),(int)&color,_dos_ds,0xB8000+offset,1);
	}
}
