/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)assign.c	5.1 (Berkeley) 6/6/85";
#endif not lint

/*
 * assign the value of an expression to a variable (or term)
 */

#include "defs.h"
#include "tree.h"
#include "sym.h"
#include "process.h"
#include "tree.rep"

assign(var, exp)
NODE *var;
NODE *exp;
{
	ADDRESS addr;
	int varsize;
	char cvalue;
	short svalue;
	long lvalue;

	if (!compatible(var->nodetype, exp->nodetype)) {
		error("incompatible types");
	}
	addr = lval(var);
	eval(exp);
	varsize = size(var->nodetype);
	if (varsize < sizeof(long)) {
		lvalue = pop(long);
		switch (varsize) {
			case sizeof(char):
				cvalue = lvalue;
				dwrite(&cvalue, addr, varsize);
				break;

			case sizeof(short):
				svalue = lvalue;
				dwrite(&svalue, addr, varsize);
				break;

			default:
				panic("bad size %d", varsize);
		}
	} else {
		sp -= varsize;
		dwrite(sp, addr, varsize);
	}
}
