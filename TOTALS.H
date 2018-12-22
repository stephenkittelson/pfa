struct TOTALS
{
	double tith_owed;
	double gross_income;
	double net;
	double expense;
	double nt_expense; // total expenses excluding tithing, used for category report
	double debtbal;
	double deped;
	double bankverf;
	double taxei;
	double taxd;
	double taxi;
};

void show_totals(void);
void add_to_totals(struct TRANS *t);
void sub_from_totals(struct TRANS *t);
#define reset_totals() memset(&totals,0,sizeof(struct TOTALS));
