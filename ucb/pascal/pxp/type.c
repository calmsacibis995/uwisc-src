/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)type.c	5.1 (Berkeley) 6/5/85";
#endif not lint

/*
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"
#include "tree.h"

STATIC	int typecnt = -1;
/*
 * Type declaration part
 */
typebeg(l, tline)
	int l, tline;
{

	line = l;
	if (nodecl)
		printoff();
	puthedr();
	putcm();
	ppnl();
	indent();
	ppkw("type");
	ppgoin(DECL);
	typecnt = 0;
	setline(tline);
}

type(tline, tid, tdecl)
	int tline;
	char *tid;
	int *tdecl;
{

	if (typecnt)
		putcm();
	setline(tline);
	ppitem();
	ppid(tid);
	ppsep(" =");
	gtype(tdecl);
	ppsep(";");
	setinfo(tline);
	putcml();
	typecnt++;
}

typeend()
{

	if (typecnt == -1)
		return;
	if (typecnt == 0)
		ppid("{type decls}");
	ppgoout(DECL);
	typecnt = -1;
}

/*
 * A single type declaration
 */
gtype(r)
	register int *r;
{

	if (r == NIL) {
		ppid("{type}");
		return;
	}
	if (r[0] != T_ID && r[0] != T_TYPACK)
		setline(r[1]);
	switch (r[0]) {
		default:
			panic("type");
		case T_ID:
			ppspac();
			ppid(r[1]);
			return;
		case T_TYID:
			ppspac();
			ppid(r[2]);
			break;
		case T_TYSCAL:
			ppspac();
			tyscal(r);
			break;
		case T_TYCRANG:
			ppspac();
			tycrang(r);
			break;
		case T_TYRANG:
			ppspac();
			tyrang(r);
			break;
		case T_TYPTR:
			ppspac();
			ppop("^");
			gtype(r[2]);
			break;
		case T_TYPACK:
			ppspac();
			ppkw("packed");
			gtype(r[2]);
			break;
		case T_TYCARY:
		case T_TYARY:
			ppspac();
			tyary(r);
			break;
		case T_TYREC:
			ppspac();
			tyrec(r[2], NIL);
			break;
		case T_TYFILE:
			ppspac();
			ppkw("file");
			ppspac();
			ppkw("of");
			gtype(r[2]);
			break;
		case T_TYSET:
			ppspac();
			ppkw("set");
			ppspac();
			ppkw("of");
			gtype(r[2]);
			break;
	}
	setline(r[1]);
	putcml();
}

/*
 * Scalar type declaration
 */
tyscal(r)
	register int *r;
{
	register int i;

	ppsep("(");
	r = r[2];
	if (r != NIL) {
		i = 0;
		ppgoin(DECL);
		for (;;) {
			ppid(r[1]);
			r = r[2];
			if (r == NIL)
				break;
			ppsep(", ");
			i++;
			if (i == 7) {
				ppitem();
				i = 0;
			}
		}
		ppgoout(DECL);
	} else
		ppid("{constant list}");
	ppsep(")");
}

/*
 * Conformant array subrange.
 */
tycrang(r)
	register int *r;
{

	ppid(r[2]);
	ppsep("..");
	ppid(r[3]);
	ppsep(":");
	gtype(r[4]);
}

/*
 * Subrange type declaration
 */
tyrang(r)
	register int *r;
{

	gconst(r[2]);
	ppsep("..");
	gconst(r[3]);
}

/*
 * Array type declaration
 */
tyary(r)
	register int *r;
{
	register int *tl;

	ppkw("array");
	ppspac();
	ppsep("[");
	tl = r[2];
	if (tl != NIL) {
		ppunspac();
		for (;;) {
			gtype(tl[1]);
			tl = tl[2];
			if (tl == NIL)
				break;
			ppsep(",");
		}
	} else
		ppid("{subscr list}");
	ppsep("]");
	ppspac();
	ppkw("of");
	gtype(r[3]);
}
