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
	B8 id; //needed to reassociate during loading into memory
	B8 name[15];
	float limit; //if limit==0, then no limit
	float value;
	B8 sel; //for use by report_categs
	struct CATEG *next;
	struct CATEG *prev;
};

struct TRANS20b1
{
	B8 day;
	B8 month;
	B16 year;
	B8 memo[53];
	B8 per_to_tith;
	B16 flags;
	float gross_income;
	float value;
	B8 categ_id;
	struct CATEG *categ;
	struct TRANS20b1 *next;
	struct TRANS20b1 *prev;
};

/* 20b2 info

struct TRANS20b2
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

struct DC // debtor/creditor
{
	B16 id;
	B8 name[15];
	float amount;
	struct DC *next;
	struct DC *prev;
};

*/

void load_20b1_file(B8 *path,int fhandle);
void write_20b2_file(B8 *file,int fhandle);
void clean_up(void);

struct TRANS20b1 *first_t20b1=0;
struct CATEG *first_categ=0;

int main(int argc,char **argv)
{
	int fdest,fsrc;

	if(argc!=3)
	{
		printf("Usage: conv [PFA data file 2.0b1] [new PFA data file 2.0b2]\n");
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
		dlog("PFA: convert 2.0b1 -> 2.0b2\n\n");
	#endif
	load_20b1_file(argv[1],fsrc);
	atexit(clean_up);
	close(fsrc);
	write_20b2_file(argv[2],fdest);
	close(fdest);
	printf("\nconversion complete\n");
	return 0;
}

void load_20b1_file(B8 *path,int fhandle)
{
	B32 num;
	B32 c;
	struct TRANS20b1 *t,*prev_t=0;
	struct CATEG *categ,*prev_c=0;

	#define READ_TRANS(_Mval,_Msize) \
		if(read(fhandle,_Mval,_Msize)<_Msize)\
		{\
			printf("'%s' is not a valid file, it abruptly ended\n%lu of %lu transactions loaded, 0 categories loaded\n",path,c,num);\
			exit(INV_FILE);\
		}
	#define READ_CATEG(_Mval,_Msize) \
		if(read(fhandle,_Mval,_Msize)<_Msize)\
		{\
			printf("'%s' is not a valid file, it ended a little too soon\nAll transactions loaded, %lu of %lu categories loaded\n",path,c,num);\
			exit(INV_FILE);\
		}

	printf("loading...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	if(read(fhandle,&num,sizeof(B32))<sizeof(B32))
	{
		printf("'%s' is not a valid file, it ended very abruptly\n0 transactions loaded, 0 categories loaded\n",path);
		exit(INV_FILE);
	}
	#ifdef DEBUG
		dlog("loading %lu transes\n",num);
	#endif
	for(c=0;c<num;c++)
	{
		disp_actsig();
		if(!(t=(struct TRANS20b1 *)malloc(sizeof(struct TRANS20b1))))
		{
			printf("\bFAILED\nNot enough memory to load all of '%s'.\n%lu of %lu transactions loaded, 0 categories loaded\n",path,c,num);
			exit(OUTOFMEM);
		}
		READ_TRANS(&(t->day),1);
		READ_TRANS(&(t->month),1);
		READ_TRANS(&(t->year),2);
		READ_TRANS(t->memo,53);
		READ_TRANS(&(t->per_to_tith),1);
		READ_TRANS(&(t->flags),2);
		READ_TRANS(&(t->gross_income),4);
		READ_TRANS(&(t->value),4);
		READ_TRANS(&(t->categ_id),1);
		#ifdef DEBUG
			dlog(" %u %u-%u-%u %s\n",t->categ_id,t->year,t->month,t->day,t->memo);
		#endif
		t->categ=0;
		t->prev=prev_t;
		t->next=0;
		if(!prev_t)
		{
			first_t20b1=t;
		}
		else
		{
			prev_t->next=t;
		}
		prev_t=t;
	}
	if(read(fhandle,&num,sizeof(B32))<sizeof(B32))
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
		READ_CATEG(&(categ->id),1);
		READ_CATEG(categ->name,15);
		READ_CATEG(&(categ->limit),4);
		#ifdef DEBUG
			dlog(" %u:%s\n",categ->id,categ->name);
		#endif
		categ->value=0;
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
		if(categ->id>48)
		{
			printf("Category limit reached. %lu of %lu categories loaded.",c,num);
			break;
		}
	}
	printf("\bdone\n");
}

void write_20b2_file(B8 *file,int fhandle)
{
	B8 zero=0;
	B16 dc_id=0;
	B32 num;
	struct TRANS20b1 *t;
	struct CATEG *categ;

	#define WRITE(_Mvar,_Msize) \
		if(write(fhandle,_Mvar,_Msize)<=0)\
		{\
			close(fhandle);\
			printf("\bUnable to write to '%s': %s\n",file,sys_errlist[errno]);\
			exit(WRITE_FAIL);\
		}

	if(!first_t20b1)
	{
		printf("No data to write");
		return;
	}
	//copy to temp file, if it turns out that there isn't enough disk space,
	//we won't lose the file we are saving to
	printf("writing...");
	disp_actsig_x=getcurx();
	disp_actsig_y=getcury();
	for(num=0,t=first_t20b1;t;num++,t=t->next,disp_actsig());
	WRITE(&num,4);
	t=first_t20b1;
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
		t->flags>>=4;
		WRITE(&(t->flags),1);
		WRITE(&(t->gross_income),sizeof(float));
		WRITE(&(t->value),sizeof(float));
		WRITE(&(t->categ_id),1); WRITE(&zero,1);
		WRITE(&(dc_id),2);
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
		WRITE(&(categ->id),1); WRITE(&zero,1);
		WRITE(categ->name,15);
		WRITE(&(categ->limit),sizeof(float));
		categ=categ->next;
	}
	num=0;
	WRITE(&num,4);
	close(fhandle);
	printf("\bdone\n");
}

void clean_up(void)
{
	struct TRANS20b1 *t20b1,*nt20b1;
	struct CATEG *categ,*ncateg;

	t20b1=first_t20b1;
	categ=first_categ;
	while(t20b1 || categ)
	{
		if(t20b1)
		{
			nt20b1=t20b1->next;
			free(t20b1);
			t20b1=nt20b1;
		}
		if(categ)
		{
			ncateg=categ->next;
			free(categ);
			categ=ncateg;
		}
	}
}
