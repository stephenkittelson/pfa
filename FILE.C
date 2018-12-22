#include <bios.h>
#include <unistd.h>
#include <string.h>
#include <myalleg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include LIBDIR"std.h"
#include LIBDIR"port.h"
#include "pfa.h"
#include "tl.h"
#include "scr.h"
#include "cl.h"
#include "dcl.h"
#include "options.h"
#include "totals.h"
#include "misc.h"
#include "file.h"
#include "error.h"
#include LIBDIR"actsig.h"
#include LIBDIR"flags.h"

struct FILE_HIST *begin_fh_list=0;

extern B8 disp_actsig_x;
extern B8 disp_actsig_y;
extern B8 disp_actsig_c;
extern struct COLOR user_colors;
extern struct TOTALS totals;
extern struct OPTIONS options;
extern struct LIST tl,cl,dcl;

#define NOERROR   0
#define CHKSUM    1
#define ENDOFFILE 2

#define FBUF_SIZE 100 //(4+10*sizeof(struct TRANS))

static B8 fbuf[FBUF_SIZE];

B8 cache_read(int fhandle,void *buf,ssize_t sizetoread)
{
  static ssize_t bytes_left=0;
  B8 cache_index,size_read;

  if(sizetoread>256)
  {
    sprintf(fbuf,"FBUF_SIZE: %u sizetoread: %u\n",FBUF_SIZE,sizetoread);
    error_exit(fbuf,EXIT_DEVELOPMENTERROR,0);
  }
  for(size_read=0,cache_index=0;size_read<sizetoread;size_read++,cache_index++,bytes_left--)
  {
    if(bytes_left==0)
    {
      // refill buffer
      if((bytes_left=read(fhandle,fbuf,FBUF_SIZE))<1)
      {
	return bytes_left;
      }
      cache_index=0;
    }
    (*((B8 *)buf+size_read))=fbuf[cache_index];
  }
  memcpy(&(fbuf[0]),&(fbuf[cache_index]),bytes_left);
  return size_read;
}

B8 _load_file(B8 *path,B8 limited_run)
{
	int fhandle;
	B32 num;
	B32 c;
	B8 c2=0;
	ssize_t retval;
	B8 fileformat32=FALSE;
	void (*lf_print)(B8 *str,...)=print_infoline;
	struct TRANS *t,*prev_t;
	struct CATEG *categ,*prev_c;
	struct DC *dc,*prev_dc;

	#define TRANS_MSG "Able to load %lu of %lu transactions, 0 categories, and 0 debtors/creditors."
	#define TRANS_MSG_NONE "Unable to load any transactions, categories, or debtors/creditors."
	#define CATEG_MSG "Able to load all transactions, %lu of %lu categories, and 0 debtors/creditors."
	#define CATEG_MSG_NONE "Able to load all transactions, 0 categories, and 0 debtors/creditors."
	#define DC_MSG "Able to load all transactions, all categories, and %lu of %lu debtors/creditors."
	#define DC_MSG_NONE "Able to load all transactions, all categories, and 0 debtors/creditors."
	#define ABORT() \
		close(fhandle);\
		return FALSE;
	#define READ_TRANS(_Mbuffer,_Msize) \
		if((retval=cache_read(fhandle,_Mbuffer,_Msize))<1)\
		{\
			if(retval==0)\
                        {\
				lf_print("'%s' is not a valid file, it abruptly ended. " TRANS_MSG,path,c,num);\
                        }\
			else if(retval<0)\
			{\
				lf_print("Unable to read transactions from '%s': %s",path,sys_errlist[errno]);\
			}\
			if(!limited_run)\
			{\
				reset_tl();\
				add_tmplt_to_list(&tl);\
			}\
			ABORT();\
		}
	#define READ_CATEG(_Mbuffer,_Msize) \
		if((retval=cache_read(fhandle,_Mbuffer,_Msize)<_Msize)<1)\
		{\
			if(retval==0)\
			{\
				lf_print("'%s' is not a valid file, it abruptly ended. " CATEG_MSG,path,c,num);\
			}\
			else if(retval<0)\
			{\
				lf_print("Unable to read categories from '%s': %s",path,sys_errlist[errno]);\
			}\
			if(!limited_run)\
			{\
				reset_cl();\
				reset_tl();\
				add_tmplt_to_list(&tl);\
				add_tmplt_to_list(&cl);\
			}\
			ABORT();\
		}
	#define READ_DC(_Mbuffer,_Msize) \
		if((retval=cache_read(fhandle,_Mbuffer,_Msize)<_Msize)<1)\
		{\
			if(retval==0)\
			{\
				lf_print("'%s' is not a valid file, it abruptly ended. " DC_MSG,path,c,num);\
			}\
			else if(retval<0)\
			{\
				lf_print("Unable to read transactees from '%s': %s",path,sys_errlist[errno]);\
			}\
			if(!limited_run)\
			{\
				reset_cl();\
				reset_dcl();\
				reset_tl();\
				add_tmplt_to_list(&tl);\
				add_tmplt_to_list(&cl);\
				add_tmplt_to_list(&dcl);\
			}\
			ABORT();\
		}

	if(limited_run)
	{
		(int *)lf_print=printf;
	}
	if((fhandle=open(path,O_BINARY,S_IREAD))<0)
	{
		lf_print("Error opening '%s': %s",path,sys_errlist[errno]);
		return FALSE;
	}
	lf_print("loading...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	if(cache_read(fhandle,&num,4)<4)
	{
		lf_print("'%s' is not a valid file, it ended very abruptly. " TRANS_MSG_NONE,path);
		ABORT();
	}
	if(num==0)
	{
		fileformat32=TRUE;
	}
	if(!limited_run)
	{
		reset_all();
	}
	prev_t=0;
	for(c=0;c<num;c++)
	{
		disp_actsig();
		if(!(t=(struct TRANS *)malloc(sizeof(struct TRANS))))
		{
			lf_print("Not enough memory to load all of '%s'. " TRANS_MSG,path,c,num);
			if(!limited_run)
			{
				reset_tl();
				add_tmplt_to_list(&tl);
			}
			ABORT();
		}
		READ_TRANS(&(t->year),2);
		#ifdef DEBUG
		write_logf(1,"%04u-%02u-%02u\n",t->year,t->month,t->day);
		write_logf(0,"%04u-%02u-%02u\n",t->year,t->month,t->day);
		#endif
		READ_TRANS(&(t->month),1);
		READ_TRANS(&(t->day),1);
		READ_TRANS(t->memo,53);
		READ_TRANS(&(t->per_to_tith),1);
		READ_TRANS(&(t->flags),1);
		READ_TRANS(&(t->gross_income),sizeof(double));
		READ_TRANS(&(t->value),sizeof(double));
		READ_TRANS(&(t->categ_id),2);
		READ_TRANS(&(t->dc_id),2);
		t->categ=0;
		t->dc=0;
		t->next=0;
		t->prev=prev_t;
		if(prev_t)
			prev_t->next=t;
		prev_t=t;
		if(!limited_run)
		{
			tl.last=(void *)t;
			if(c2<=tl.max_sel_num)
			{
				if(c2==0)
				{
					tl.top=(void *)t;
					tl.first=(void *)t;
				}
				else
					tl.ttb_dist++;
				tl.bot=(void *)t;
				c2++;
			}
		}
		else
		{
			if(c2==0)
			{
				tl.first=(void *)t;
				c2++;
			}
		}
	}
	if(cache_read(fhandle,&num,4)<4)
	{
		lf_print("'%s' is not a valid file, it ended too soon. " CATEG_MSG_NONE,path);
		if(!limited_run)
		{
			reset_tl();
			add_tmplt_to_list(&tl);
		}
		ABORT();
	}
	prev_c=((struct CATEG *)cl.first)->next->next;
	for(c2=3,c=0;c<num;c++)
	{
		disp_actsig();
		if(!(categ=(struct CATEG *)malloc(sizeof(struct CATEG))))
		{
			lf_print("Not enough memory to load all of '%s'. " CATEG_MSG,path,c,num);
			if(!limited_run)
			{
				reset_cl();
				reset_tl();
				add_tmplt_to_list(&tl);
				add_tmplt_to_list(&cl);
			}
			ABORT();
		}
		READ_CATEG(&(categ->id),2);
		READ_CATEG(categ->name,15);
		READ_CATEG(&(categ->limit),sizeof(double));
		categ->value=0;
		categ->tmplt=FALSE;
		categ->next=0;
		categ->prev=prev_c;
		prev_c->next=categ;
		prev_c=categ;
		if(!limited_run)
		{
			cl.last=(void *)categ;
			if(c2<=cl.max_sel_num)
			{
				cl.ttb_dist++;
				cl.bot=(void *)categ;
				c2++;
			}
		}
	}
	if(cache_read(fhandle,&num,4)<4)
	{
		lf_print("'%s' is not a valid file, it ended abruptly. " DC_MSG_NONE,path);
		if(!limited_run)
		{
			reset_cl();
			reset_tl();
			add_tmplt_to_list(&tl);
			add_tmplt_to_list(&cl);
		}
		ABORT();
	}
	prev_dc=(struct DC *)dcl.first;
	for(c2=1,c=0;c<num;c++)
	{
		disp_actsig();
		if(!(dc=(struct DC *)malloc(sizeof(struct DC))))
		{
			lf_print("Not enough memory to load all of '%s'. " DC_MSG,path,c,num);
			if(!limited_run)
			{
				reset_cl();
				reset_dcl();
				reset_tl();
				add_tmplt_to_list(&tl);
				add_tmplt_to_list(&cl);
				add_tmplt_to_list(&dcl);
			}
			ABORT();
		}
		READ_DC(&(dc->id),2);
		READ_DC(dc->name,15);
		dc->amount=0;
		dc->tmplt=FALSE;
		dc->next=0;
		dc->prev=prev_dc;
		prev_dc->next=dc;
		prev_dc=dc;
		if(!limited_run)
		{
			dcl.last=(void *)dc;
			if(c2<=dcl.max_sel_num)
			{
				dcl.ttb_dist++;
				dcl.bot=(void *)dc;
				c2++;
			}
		}
	}
	close(fhandle);
	t=(struct TRANS *)tl.first;
	// reassociate transactions with...
	while(t)
	{
		disp_actsig();
		// categories
		if(t->categ_id!=2)
		{
			categ=((struct CATEG *)cl.first);
			while(categ)
			{
				if(categ->id==t->categ_id)
				{
					t->categ=categ;
					break;
				}
				categ=categ->next;
			}
			if(!categ)
			{
				t->categ_id=0;
				t->categ=(struct CATEG *)cl.first;
			}
		}
		else
		{
			t->categ=((struct CATEG *)cl.first)->next->next;
		}
		// debtors/creditors
		if(t->dc_id!=0)
		{
			dc=((struct DC *)dcl.first)->next;
			while(dc)
			{
				if(dc->id==t->dc_id)
				{
					t->dc=dc;
					break;
				}
				dc=dc->next;
			}
			if(!dc)
			{
				t->dc_id=0;
				t->dc=(struct DC *)dcl.first;
			}
		}
		else
		{
			t->dc=(struct DC *)dcl.first;
		}
		add_to_totals(t);
		t=t->next;
	}
	if(!limited_run)
	{
		cl.sel=cl.first;
		tl.sel=tl.first;
		dcl.sel=dcl.first;
		add_tmplt_to_list(&tl);
		add_tmplt_to_list(&cl);
		add_tmplt_to_list(&dcl);
	}
	printf("\bcomplete");
	return TRUE;
}

B8 write_file(B8 *path)
{
	int fhandle;
	B32 num;
	struct TRANS *t;
	struct CATEG *categ;
	struct DC *dc;
	struct FILE_HIST *fh,*prev_fh;

	#define WRITE(_Mvar,_Msize) \
		if(write(fhandle,_Mvar,_Msize)<=0)\
		{\
			close(fhandle);\
			sys_cmd("move /Y _temp_#.tmp %s",path);\
			print_infoline("Unable to write to file: %s",sys_errlist[errno]);\
			return FALSE;\
		}

	if( (tl.first==tl.last && ((struct TRANS *)tl.first)->flags&TEMPLATE)
		&&
		 (((struct CATEG *)cl.first)->next->next->next->tmplt &&
			((struct CATEG *)cl.first)->next->next->next==(struct CATEG *)cl.last)
		&&
		 (((struct DC *)dcl.first)->next->tmplt &&
			((struct DC *)dcl.first)->next==(struct DC *)dcl.last)
		)
	{
		print_infoline("No data to save");
		return FALSE;
	}
	//redo the id's
	print_infoline("processing data...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	categ=(struct CATEG *)cl.first;
	dc=(struct DC *)dcl.first;
	num=0;
	while(dc || categ)
	{
		disp_actsig();
		if(num>65535)
		{
			setcurpos(0,48);
			fputs("Over 65,535:",stdout);
			if(dc && categ)
				fputs(" transactees & categories",stdout);
			if(dc)
				fputs(" transactees",stdout);
			if(categ)
				fputs(" categories",stdout);
			fputs(". Unable to save data.",stdout);
			fflush(stdout);
			return FALSE;
		}
		if(dc)
		{
			dc->id=num;
			dc=dc->next;
		}
		if(categ)
		{
			categ->id=num;
			categ=categ->next;
		}
		num++;
	}
	#ifndef RANDOM_INPUT
		//copy to temp file, if we end up not being able to write the whole file,
		//we won't lose the file we are saving to
		sys_cmd("copy /Y %s _temp_#.tmp",path);
		if((fhandle=open(path,O_WRONLY|O_TRUNC|O_CREAT|O_BINARY,S_IWRITE))<0)
		{
			print_infoline("Error saving '%s': %s",path,sys_errlist[errno]);
			return FALSE;
		}
	#endif
	print_infoline("writing...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	for(num=0,t=(struct TRANS *)tl.first;t;num++)
	{
		while(t && (TEMPLATE&t->flags))
		{
			t=t->next;
		}
		if(!t)
			break;
		t=t->next;
		disp_actsig();
	}
	#ifndef RANDOM_INPUT
		WRITE(&num,4);
	#endif
	t=(struct TRANS *)tl.first;
	//we are not going to write the template trans(es)
	while(t)
	{
		disp_actsig();
		if(TEMPLATE&t->flags)
		{
			t=t->next;
			continue;
		}
		#ifndef RANDOM_INPUT
			WRITE(&(t->year),2);
			WRITE(&(t->month),1);
			WRITE(&(t->day),1);
			WRITE(t->memo,53);
			WRITE(&(t->per_to_tith),1);
			WRITE(&(t->flags),1);
			WRITE(&(t->gross_income),sizeof(double));
			WRITE(&(t->value),sizeof(double));
			WRITE(&(t->categ->id),2);
			WRITE(&(t->dc->id),2);
		#endif
		t=t->next;
	}
	for(num=0,categ=((struct CATEG *)cl.first)->next->next->next;categ;num++)
	{
		while(categ && categ->tmplt)
		{
			categ=categ->next;
		}
		if(!categ)
			break;
		categ=categ->next;
		disp_actsig();
	}
	#ifndef RANDOM_INPUT
		WRITE(&num,4);
	#endif
	//we are not going to save the 'Other' & 'Tithing' & 'Income' categories
	categ=((struct CATEG *)cl.first)->next->next->next;
	while(categ)
	{
		disp_actsig();
		if(categ->tmplt)
		{
			categ=categ->next;
			continue;
		}
		#ifndef RANDOM_INPUT
			WRITE(&(categ->id),2);
			WRITE(categ->name,15);
			WRITE(&(categ->limit),sizeof(double));
		#endif
		categ=categ->next;
	}
	for(num=0,dc=((struct DC *)dcl.first)->next;dc;num++)
	{
		while(dc && dc->tmplt)
		{
			dc=dc->next;
		}
		if(!dc)
			break;
		dc=dc->next;
		disp_actsig();
	}
	#ifndef RANDOM_INPUT
		WRITE(&num,4);
	#endif
	// we are not going to save the 'no one' entry
	dc=((struct DC *)dcl.first)->next;
	while(dc)
	{
		disp_actsig();
		if(dc->tmplt)
		{
			dc=dc->next;
			continue;
		}
		#ifndef RANDOM_INPUT
			WRITE(&(dc->id),2);
			WRITE(dc->name,15);
		#endif
		dc=dc->next;
	}
	#ifndef RANDOM_INPUT
		close(fhandle);
	#endif
	print_infoline("deleting temp file...");
	#ifndef RANDOM_INPUT
		sys_cmd("del _temp_#.tmp");
	#endif
	print_infoline("adding file to history...");
	for(num=0,fhandle=TRUE,fh=begin_fh_list;fh;fh=fh->next,num++)
	{
		if(fh->name[0]==path[0])
		{
			if(strncmp(fh->name,path,100)==0)
			{
				fhandle=FALSE;
				break;
			}
		}
	}
	if(fhandle)
	{
		if(num==30)
		{
			for(fh=begin_fh_list,prev_fh=0;fh->next;prev_fh=fh,fh=fh->next);
			prev_fh->next=0;
			free(fh);
		}
		if(!(fh=(struct FILE_HIST *)malloc(sizeof(struct FILE_HIST))))
		{
			print_infoline("write semi-successful...Not enough memory to add file to history");
			return TRUE;
		}
		strncpy(fh->name,path,100);
		fh->next=begin_fh_list;
		begin_fh_list=fh;
	}
	#ifdef RANDOM_INPUT
		print_infoline("Cannot write while random input is going.");
	#else
		print_infoline("writing complete");
	#endif
	return TRUE;
}

void print_tl(void)
{
	B8 retval;
	struct TRANS *t;
	B8 buf[81];
	B8 c;

	#ifdef RANDOM_INPUT
		print_infoline("Cannot print data while on random input.");
		return;
	#endif
	retval=biosprint(2,NULL,0);
	if(retval&0x20)
	{
		print_infoline("Printer: no paper");
		return;
	}
	if(retval&0x08)
	{
		print_infoline("IO error! Cannot communicate with printer");
		return;
	}
	if(!(retval&0x80))
	{
		print_infoline("Printer is not ready for printing");
		return;
	}
	print_infoline("sending transaction data to printer...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	t=(struct TRANS *)tl.first;
	while(t)
	{
		disp_actsig();
		if(TEMPLATE&t->flags)
		{
			t=t->next;
			continue;
		}
		memset(buf,32,80);
		sprintf(buf,"%u-%02u-%02u %s",t->year,t->month,t->day,t->memo);
		sprintf(buf+64,"$%.02f",t->value);
		for(c=11;c<80;c++)
		{
			if(buf[c]==0)
				buf[c]=' ';
		}
		buf[80]=0;
		fprintf(stdprn,"%s",buf);
		t=t->next;
	}
	fprintf(stdprn,
					"\n"
					" Balance: $%.02f\n"
					" Bank verified: $%.02f\n"
					"\n"
					" Gross income: $%.02f\n"
					" Total expense: $%.02f\n"
					" Total deposited: $%.02f\n"
					"\n"
					" Tithing payed: $%.02f\n"
					" Tithing owed: $%.02f\n"
					"\n"
					" Debt balance: $%.02f\n"
					"\n"
					"   TAXES\n"
					"\n"
					" Earned interest: $%.02f\n"
					" Deductable: $%.02f\n"
					" Taxable income: $%.02f\n"
			 ,totals.net,totals.bankverf,totals.gross_income,totals.expense,
			 totals.deped,((struct CATEG *)cl.first)->next->value*-1,
			 totals.tith_owed,totals.debtbal,totals.taxei,totals.taxd,totals.taxi);
	fprintf(stdprn,"\f");
	printf("\bdone");
}

void read_filehist(int fhandle)
{
	B8 c,num;
	struct FILE_HIST *fh;

	//we do not close the file in this function because it is already done in
	// init_options
	atexit(clean_file);
	if(read(fhandle,&num,1)<1)
	{
		return;
	}
	for(c=0;c<num;c++)
	{
		if(!(fh=(struct FILE_HIST *)malloc(sizeof(struct FILE_HIST))))
		{
			clear_keybuf();
			print_infoline("Not enough memory to load all of the file history! Press any key to continue...");
			readkey();
			return;
		}
		if(read(fhandle,fh->name,100)<100)
		{
			free(fh);
			return;
		}
		fh->next=begin_fh_list;
		begin_fh_list=fh;
	}
}

void clean_file(void)
{
	int fhandle;
	B8 c;
	struct FILE_HIST *fh,*nfh;

	if(!begin_fh_list)
		return;
	if((fhandle=open("pfa.dat",O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,S_IWUSR))<0)
	{
		clear_keybuf();
		print_infoline("Unable to create 'pfa.dat', thus, unable to store file"
			"history: %s",sys_errlist[errno]);
		readkey();
		return;
	}
	#define WRITE_OP(_Msrc,_Msize) \
		if(write(fhandle,_Msrc,_Msize)<1)\
		{\
			close(fhandle);\
			clear_keybuf();\
			print_infoline("Unable to modify 'pfa.dat': %s\nPress any key to continue...",sys_errlist[errno]);\
			readkey();\
			return;\
		}
	WRITE_OP(options.sfn,FILENAME_SIZE);
	WRITE_OP(&options.flags,1);
	WRITE_OP(&user_colors.general,1);
	WRITE_OP(&user_colors.selected,1);
	WRITE_OP(&user_colors.highlight,1);
	for(c=0,fh=begin_fh_list;fh;c++,fh=fh->next);
	if(write(fhandle,&c,1)<=0)
	{ 
		clear_keybuf();
		close(fhandle);
		print_infoline("Unable to write any file history data to 'pfa.dat': %s\nPress any key to continue...",sys_errlist[errno]);
		readkey();
		return;
	}
	for(fh=begin_fh_list;fh;)
	{
		if(write(fhandle,fh->name,100)<100)
		{ 
			clear_keybuf();
			close(fhandle);
			print_infoline("Unable to write all file history data to 'pfa.dat': %s\nPress any key to continue...",sys_errlist[errno]);
			readkey();
			return;
		}
		nfh=fh->next;
		free(fh);
		fh=nfh;
	}
	close(fhandle);
}
