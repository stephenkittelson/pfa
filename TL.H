#include LIBDIR"std.h"
#include LIBDIR"flags.h"

struct TRANS
{
	// are you going to modify these? copy this to a new conv program first!
	B16 year;
	B8 month;
	B8 day;
	B8 memo[53];
	B8 per_to_tith;
	B8 flags;
	double gross_income;
	double value;
	B16 categ_id;
	B16 dc_id;
	struct CATEG *categ;
	struct DC *dc;
	struct TRANS *next;
	struct TRANS *prev;
};

//flag definitions
	// deposited
	#define DEPED     BIT_0
	// bank verified transaction
	#define BANKVERF  BIT_1
	// tax: earned interest
	#define TAXEI     BIT_2
	// tax deductable
	#define TAXD      BIT_3
	// taxable income
	#define TAXI      BIT_4
	// trans made by program, user edits to make new trans and this flag is set 
	//  to false
	#define TEMPLATE  BIT_5

void tl_display(void);
void init_tl(B8 limited_run);
void free_tl(void);
void reset_tl(void);
