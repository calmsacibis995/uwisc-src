/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)maketypes.c	5.1 (Berkeley) 6/6/85";
#endif not lint

/*
 * make symbols
 */

#include "defs.h"
#include "sym.h"
#include "symtab.h"
#include "sym/btypes.h"
#include "sym/classes.h"
#include "sym/sym.rep"

/*
 * point the basic types in the right direction
 */

maketypes()
{
	t_int = st_lookup(symtab, "integer")->type;
	t_real = st_lookup(symtab, "real")->type;
	t_char = st_lookup(symtab, "char")->type;
	t_boolean = st_lookup(symtab, "boolean")->type;
	if (t_int==NIL || t_real==NIL || t_char==NIL || t_boolean==NIL) {
		panic("basic types are missing from namelist");
	}
}

/*
 * enter a keyword in the given table
 */

make_keyword(table, name, tnum)
SYMTAB *table;
char *name;
int tnum;
{
	register SYM *s;

	s = st_insert(table, name);
	s->class = BADUSE;
	s->blkno = 0;
	s->symvalue.token.toknum = tnum;
}
