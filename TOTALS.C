#include <myalleg.h>
#include LIBDIR"port.h"
#include "scr.h"
#include "cl.h"
#include "tl.h"
#include "totals.h"
#include "options.h"
#include "dcl.h"
#include "list.h"
#include LIBDIR"flags.h"

struct TOTALS totals;

extern struct LIST cl;
extern struct OPTIONS options;

void show_totals(void)
{
	clear_scr();
	setcurpos(0,0);
	printf( "\n"
					" Balance: $%.02f\n"
					" Bank verified: $%.02f\n"
		                        " Non-verified bank account balance: $%.02f\n"
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
			 ,totals.net,totals.bankverf,totals.deped-totals.expense,totals.gross_income,totals.expense,
			 totals.deped,((struct CATEG *)cl.first)->next->value*-1,
			 totals.tith_owed,totals.debtbal,totals.taxei,totals.taxd,totals.taxi);
	key[KEY_ESC]=FALSE;
	while(!key[KEY_ESC]);
	clear_keybuf();
}

void add_to_totals(struct TRANS *t)
{
	t->categ->value+=t->value;
	t->dc->amount-=t->value;
	if(t->categ->id!=1)
	{
		if(TITH_GROSS&options.flags)
		{
			totals.tith_owed+=t->gross_income*((double)t->per_to_tith/100);
		}
		else 
			totals.tith_owed+=t->value*((double)t->per_to_tith/100);
	}
	else 
	{
		totals.tith_owed+=t->value;
	}
	totals.gross_income+=t->gross_income;
	totals.net+=t->value;
	if(t->value<0)
	{
		totals.expense-=t->value;
		if(t->categ->id!=1)
			totals.nt_expense-=t->value;
	}
	if(t->dc && t->dc->id)
		totals.debtbal-=t->value;
	if(DEPED&t->flags)
		totals.deped+=t->value;
	if(BANKVERF&t->flags)
		totals.bankverf+=t->value;
	if(TAXEI&t->flags)
		totals.taxei+=t->value;
	if(TAXD&t->flags)
		totals.taxd-=t->value;
	if(TAXI&t->flags)
		totals.taxi+=t->value;
}

void sub_from_totals(struct TRANS *t)
{
	t->categ->value-=t->value;
	t->dc->amount+=t->value;
	if(t->categ->id!=1)
	{
		if(TITH_GROSS&options.flags)
			totals.tith_owed-=t->gross_income*((double)t->per_to_tith/100);
		else 
			totals.tith_owed-=t->value*((double)t->per_to_tith/100);
	}
	else 
	{
		totals.tith_owed-=t->value;
	}
	totals.gross_income-=t->gross_income;
	totals.net-=t->value;
	if(t->value<0)
	{
		totals.expense+=t->value;
		if(t->categ->id!=1)
			totals.nt_expense+=t->value;
	}
	if(t->dc && t->dc->id)
		totals.debtbal+=t->value;
	if(DEPED&t->flags)
		totals.deped-=t->value;
	if(BANKVERF&t->flags)
		totals.bankverf-=t->value;
	if(TAXEI&t->flags)
		totals.taxei-=t->value;
	if(TAXD&t->flags)
		totals.taxd+=t->value;
	if(TAXI&t->flags)
		totals.taxi-=t->value;
}
