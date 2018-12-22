#include LIBDIR"std.h"

struct DC // debtor/creditor
{
	// are you going to modify these? copy this to a new conv program first!
	B16 id;
	B8 name[15];
	double amount;
	B8 tmplt;
	struct DC *next;
	struct DC *prev;
};

void free_dcl(void);
void add_init_dc(void);
void init_dcl(B8 limited_run);
void reset_dcl(void);
