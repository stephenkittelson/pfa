#include LIBDIR"std.h"

struct CATEG
{
	// are you going to modify these? copy this to a new conv program first!
	B16 id; //needed to reassociate during loading into memory
	B8 name[15];
	double limit; //if limit==0, then no limit
	double value;
	B8 tmplt;
	B8 sel; //for use by report_categs
	struct CATEG *next;
	struct CATEG *prev;
};

void edit_categ(void);
void add_init_categs(void);
void free_cl(void);
void init_cl(B8 limited_run);
void reset_cl(void);
