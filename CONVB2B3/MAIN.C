#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../../lib/std.h"
#include "../../lib/actsig.h"
#include "../../lib/djgpp.h"

extern B8 disp_actsig_x;
extern B8 disp_actsig_y;

#define INV_CMDLN   1
#define INV_FILE    2
#define CREATE_FAIL 3
#define OUTOFMEM    4
#define WRITE_FAIL  5

struct CATEG
{
	B16 id; //needed to reassociate during loading into memory
	B8 name[15];
	float limit; //if limit==0, then no limit
	float value;
	B8 tmplt;
	B8 sel; //for use by report_categs
	struct CATEG *next;
	struct CATEG *prev;
};

struct DC // debtor/creditor
{
	B16 id;
	B8 name[15];
	float amount;
	B8 tmplt;
	struct DC *next;
	struct DC *prev;
};

struct TRANS
{
	B16 year;
	B8 month;
	B8 day;
	B8 memo[53];
	B8 per_to_tith;
	B8 flags;
	float gross_income;
	float value;
	B16 categ_id;
	B16 dc_id;
	struct CATEG *categ;
	struct DC *dc;
	struct TRANS *next;
	struct TRANS *prev;
};

/* 20b3 info

// same as 20b2, except replace 'float's with 'double's in TRANS, DC, and CATEG

*/

void load_20b2_file(B8 *path,int fhandle);
void write_20b3_file(B8 *file,int fhandle);
void clean_up(void);

struct TRANS *first_trans=0;
struct CATEG *first_categ=0;
struct DC    *first_dc   =0;

int main(int argc,char **argv)
{
	int fdest,fsrc;

	if(argc!=3)
	{
		printf("Usage: conv [PFA data file 2.0b2] [new PFA data file 2.0b3]\n");
		return INV_CMDLN;
	}
	if((fsrc=open(argv[1],O_RDONLY|O_BINARY,S_IRUSR))<0)
	{
		printf("Unable to open '%s': %s\n",argv[1],sys_errlist[errno]);
		return INV_FILE;
	}
	if((fdest=open(argv[2],O_WRONLY|O_EXCL|O_CREAT|O_BINARY,S_IWUSR))<0)
	{
		printf("Unable to create '%s': %s\n",argv[2],sys_errlist[errno]);
		close(fsrc);
		return CREATE_FAIL;
	}
	#ifdef DEBUG
		init_log();
		dlog("PFA: convert 2.0b2 -> 2.0b3\n\n");
	#endif
	load_20b2_file(argv[1],fsrc);
	atexit(clean_up);
	close(fsrc);
	write_20b3_file(argv[2],fdest);
	close(fdest);
	printf("\nconversion complete\n");
	return 0;
}

void load_20b2_file(B8 *path,int fhandle)
{
	B32 num;
	B32 c;
	struct TRANS *t,*prev_t=0;
	struct CATEG *categ,*prev_c=0;
	struct DC    *dc,*prev_dc=0;

	#define READ_TRANS(_Mval,_Msize) \
		if(read(fhandle,_Mval,_Msize)<_Msize)\
		{\
			printf("'%s' is not a valid file, it abruptly ended\n%lu of %lu transactions loaded, 0 categories loaded, 0 transactees loaded\n",path,c,num);\
			exit(INV_FILE);\
		}
	#define READ_CATEG(_Mval,_Msize) \
		if(read(fhandle,_Mval,_Msize)<_Msize)\
		{\
			printf("'%s' is not a valid file, it ended too soon\nAll transactions loaded, %lu of %lu categories loaded, 0 transactees loaded\n",path,c,num);\
			exit(INV_FILE);\
		}
	#define READ_DC(_Mval,_Msize) \
		if(read(fhandle,_Mval,_Msize)<_Msize)\
		{\
			printf("'%s' is not a valid file, it ended a little too soon\nAll transactions loaded, all categories loaded, %lu of %lu transactees loaded\n",path,c,num);\
			exit(INV_FILE);\
		}

	printf("loading...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	if(read(fhandle,&num,4)<4)
	{
		printf("'%s' is not a valid file, it ended very abruptly\n0 transactions loaded, 0 categories loaded, 0 transactees loaded\n",path);
		exit(INV_FILE);
	}
	#ifdef DEBUG
		dlog("loading %lu transes\n",num);
	#endif
	for(c=0;c<num;c++)
	{
		disp_actsig();
		if(!(t=(struct TRANS *)malloc(sizeof(struct TRANS))))
		{
			printf("\bFAILED\nNot enough memory to load all of '%s'.\n%lu of %lu transactions loaded, 0 categories loaded, 0 transactees loaded\n",path,c,num);
			exit(OUTOFMEM);
		}
		READ_TRANS(&(t->year),2);
		READ_TRANS(&(t->month),1);
		READ_TRANS(&(t->day),1);
		READ_TRANS(t->memo,53);
		READ_TRANS(&(t->per_to_tith),1);
		READ_TRANS(&(t->flags),1);
		READ_TRANS(&(t->gross_income),sizeof(float));
		READ_TRANS(&(t->value),sizeof(float));
		READ_TRANS(&(t->categ_id),2);
		READ_TRANS(&(t->dc_id),2);
		#ifdef DEBUG
			dlog(" %u %u-%u-%u %s\n",t->categ_id,t->year,t->month,t->day,t->memo);
		#endif
		t->prev=prev_t;
		t->next=0;
		if(!prev_t)
		{
			first_trans=t;
		}
		else
		{
			prev_t->next=t;
		}
		prev_t=t;
	}
	if(read(fhandle,&num,4)<4)
	{
		printf("'%s' is not a valid file, it ended too soon\nAll transactions loaded, 0 categories loaded\n",path);
		exit(INV_FILE);
	}
	#ifdef DEBUG
		dlog("loading %lu categs\n",num);
	#endif
	for(c=0;c<num;c++)
	{
		disp_actsig();
		if(!(categ=(struct CATEG *)malloc(sizeof(struct CATEG))))
		{
			printf("Not enough memory to load all of '%s'. (All transactions loaded, %lu of %lu categories loaded)",path,c,num);
			exit(OUTOFMEM);
		}
		READ_CATEG(&(categ->id),2);
		READ_CATEG(categ->name,15);
		READ_CATEG(&(categ->limit),sizeof(float));
		#ifdef DEBUG
			dlog(" %u:%s\n",categ->id,categ->name);
		#endif
		categ->prev=prev_c;
		categ->next=0;
		if(!prev_c)
		{
			#ifdef DEBUG
				dlog("load: setting first categ\n");
			#endif
			first_categ=categ;
		}
		else
		{
			prev_c->next=categ;
		}
		prev_c=categ;
	}
	if(read(fhandle,&num,4)<4)
	{
		printf("'%s' is not a valid file, it ended too soon\nAll transactions loaded, all categories loaded, 0 transactees loaded\n",path);
		exit(INV_FILE);
	}
	for(c=0;c<num;c++)
	{
		disp_actsig();
		if(!(dc=(struct DC *)malloc(sizeof(struct DC))))
		{
			printf("Not enough memory to load all of '%s'. (All transactions loaded, all categories loaded, %lu of %lu transactees loaded)",path,c,num);
			exit(OUTOFMEM);
		}
		READ_DC(&(dc->id),2);
		READ_DC(dc->name,15);
		dc->next=0;
		dc->prev=prev_dc;
		if(!prev_dc)
		{
			first_dc=dc;
		}
		else
		{
			prev_dc->next=dc;
		}
		prev_dc=dc;
	}
	printf("\bdone\n");
}

void write_20b3_file(B8 *file,int fhandle)
{
	double dbl;
	B32 num;
	struct TRANS *t;
	struct CATEG *categ;
	struct DC *dc;

	#define WRITE(_Mvar,_Msize) \
		if(write(fhandle,_Mvar,_Msize)<=0)\
		{\
			close(fhandle);\
			printf("\bUnable to write to '%s': %s\n",file,sys_errlist[errno]);\
			exit(WRITE_FAIL);\
		}

	if(!first_trans)
	{
		printf("No data to write");
		return;
	}
	//copy to temp file, if it turns out that there isn't enough disk space,
	//we won't lose the file we are saving to
	printf("writing...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	for(num=0,t=first_trans;t;num++,t=t->next,disp_actsig());
	WRITE(&num,4);
	t=first_trans;
	#ifdef DEBUG
		dlog("writing %lu transes\n",num);
	#endif
	while(t)
	{
		disp_actsig();
		#ifdef DEBUG
			dlog(" %u %u-%u-%u %s ",t->categ_id,t->year,t->month,t->day,t->memo);
		#endif
		WRITE(&(t->year),2);
		WRITE(&(t->month),1);
		WRITE(&(t->day),1);
		WRITE(t->memo,53);
		WRITE(&(t->per_to_tith),1);
		WRITE(&(t->flags),1);
		dbl=(double)(t->gross_income);
		WRITE(&(dbl),sizeof(double));
		dbl=(double)(t->value);
		WRITE(&(dbl),sizeof(double));
		WRITE(&(t->categ_id),2);
		WRITE(&(t->dc_id),2);
		t=t->next;
	}
	for(num=0,categ=first_categ;categ;num++,categ=categ->next);
	WRITE(&num,4);
	categ=first_categ;
	#ifdef DEBUG
		dlog("writing %lu categs\n",num);
	#endif
	while(categ)
	{
		disp_actsig();
		WRITE(&(categ->id),2);
		WRITE(categ->name,15);
		dbl=(double)(categ->limit);
		WRITE(&(dbl),sizeof(double));
		categ=categ->next;
	}
	for(num=0,dc=first_dc;dc;num++,dc=dc->next);
	WRITE(&num,4);
	dc=first_dc;
	while(dc)
	{
		disp_actsig();
		WRITE(&(dc->id),2);
		WRITE(&(dc->name),15);
		dc=dc->next;
	}
	close(fhandle);
	printf("\bdone\n");
}

void clean_up(void)
{
	struct TRANS *t,*nt;
	struct CATEG *categ,*ncateg;
	struct DC *dc,*ndc;

	t=first_trans;
	categ=first_categ;
	dc=first_dc;
	while(t || categ || dc)
	{
		if(t)
		{
			nt=t->next;
			free(t);
			t=nt;
		}
		if(categ)
		{
			ncateg=categ->next;
			free(categ);
			categ=ncateg;
		}
		if(dc)
		{
			ndc=dc->next;
			free(dc);
			dc=ndc;
		}
	}
}
