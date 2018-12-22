#include LIBDIR"std.h"
#include "pfa.h"

struct COLOR
{
	B8 general;
	B8 selected;
	B8 highlight;
};

struct OPTIONS
{
	B8 flags;
	//startup file name
	B8 sfn[FILENAME_SIZE];
};

#define COPY_COLOR(_Mdest,_Msrc) \
	_Mdest general=_Msrc general;\
	_Mdest selected=_Msrc selected;\
	_Mdest highlight=_Msrc highlight;

//flag defs
	//tith method
	#define TITH_GROSS  BIT_0
	#define TUTORIAL    BIT_1

void init_options(void);
void write_ops(void);
void edit_options(void);
