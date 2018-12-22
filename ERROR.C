#include <myalleg.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "scr.h"
#include "io.h"
#include "file.h"

void error_exit(B8 *err_msg,B8 code,B8 save)
{
	B8 x,y;
	B8 filename[FILENAME_SIZE];
	B16 flags=FALSE;

	clean_scr();
	printf("%s",err_msg);
	if(save)
	{
		printf("\nIf you try, but cannot save or don't want to save, press Ctrl+Break (the Break key is also the Pause key)\nEnter file name to save to: ");
		x=getcurx();
		y=getcury();
		filename[0]=0;
		clear_keybuf();
		do
		{
			while(filename[0]==0)
			{
				setcurpos(x,y);
				get_string(filename,FILENAME_SIZE,"\n\r",INV_FN_CHARS,&flags);
			}
		}while(!write_file(filename));
	}
	exit(code);
}
