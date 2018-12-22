#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include LIBDIR"actsig.h"

#define NUM_TRANS 0xFFFF
#define NUM_CATEG 0xFFFF-3
#define NUM_DC    0xFFFF-1

#define SIZE_FN "test.size"

extern B8 disp_actsig_x,disp_actsig_y;

//todo random files?

int main(void)
{
	int fhandle;
	B16 c;
	B32 num=NUM_TRANS;
	B8 trans_data[]=
	{
		0xD0,0x07, //these have to be flipped, i guess that is how read + write work, strange, it doesn't do that to strings...
		0x01,
		0x01,
		't','e','s','t',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','-',
			'-','-','-','e',0x00,
		0x00,
		0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,
		0x00,0x00
	};
	B8 categ_data[]=
	{
		0x03,0x00,
		't','e','s',
			't','-','-',
			'-','-','-',
			'-','-','-',
			'-','e',0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};
	B8 dc_data[]=
	{
		0x03,0x00,
		't','e','s',
			't','-','-',
			'-','-','-',
			'-','-','-',
			'-','e',0x00
	};

	if((fhandle=open(SIZE_FN,O_WRONLY|O_TRUNC|O_CREAT|O_BINARY,S_IWUSR))<0)
	{
		printf("Unable to open '" SIZE_FN "': %s\n",sys_errlist[errno]);
		return 1;
	}
	#define WRITE(_Mdata,_Msize) \
		if(write(fhandle,_Mdata,_Msize)<_Msize) \
		{ \
			printf("Unable to write to '" SIZE_FN "': %s\n",sys_errlist[errno]); \
			return 1; \
		}
	printf("Writing...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	WRITE(&num,4);
	for(c=0;c<NUM_TRANS;c++)
	{
		WRITE(trans_data,sizeof(trans_data));
		if(trans_data[3]==0xFF)
			trans_data[2]++;
		trans_data[3]++;
		disp_actsig();
	}
	num=NUM_CATEG;
	WRITE(&num,4);
	for(c=0;c<NUM_CATEG;c++)
	{
		WRITE(categ_data,sizeof(categ_data));
		disp_actsig();
	}
	num=NUM_DC;
	WRITE(&num,4);
	for(c=0;c<NUM_DC;c++)
	{
		WRITE(dc_data,sizeof(dc_data));
		disp_actsig();
	}
	close(fhandle);
	printf("\bdone");
	return 0;
}
