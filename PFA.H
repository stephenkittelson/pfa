/*
	Information
	ßßßßßßßßßßß

	Prog name       : Personal Financial Assistant
	Author          : Stephen Kittelson
	Start date      : 1999-12-05 (That is when I started Budget (PFA 1.0))
										2000-05-31 (Estimated start of PFA 2.0)
	Last update     : 2000-12-06 341
	OS              : DOS
	Desc            : Program to make the management of personal finances
										much more efficient than pen & paper
*/

#ifndef _PFA_H_
#define _PFA_H_

#define EMAIL_ADDR "stephen.kittelson@juno.com"

#define EXIT_NORMAL           0
#define EXIT_NOTITHOWED       1
#define EXIT_TITHOWED         2
#define EXIT_INVCMDLN         3
#define EXIT_OUTOFMEM         4
#define EXIT_DEVELOPMENTERROR 5
#define EXIT_INVFILE          6

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bios.h>
#include <go32.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <io.h>
#include <sys/movedata.h>
#include <sys/segments.h>
#include <sys/stat.h>
#include <myalleg.h>
*/

#define FILENAME_SIZE 60
#define MAX_AMOUNT  10000000.00
#define MIN_AMOUNT -10000000.00
#define MAX_BDDIGITS 8
#define MAX_ADDIGITS 2

/*
#include LIBDIR"std.h"
#include LIBDIR"djgpp.h"
*/

#endif //_PFA_H_
