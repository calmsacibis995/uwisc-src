#ifndef lint
static char sccsid[] = "@(#)context.c	3.7 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include <stdio.h>
#include "value.h"
#include "string.h"
#include "context.h"

/*
 * Context push/pop for nested command files.
 */

char *malloc();

cx_alloc()
{
	register struct context *xp;

	if (cx.x_type != 0) {
		xp = (struct context *)
			malloc((unsigned) sizeof (struct context));
		if (xp == 0)
			return -1;
		*xp = cx;
		cx.x_link = xp;
		cx.x_type = 0;
	}
	cx.x_erred = 0;
	cx.x_synerred = 0;
	cx.x_abort = 0;
	return 0;
}

cx_free()
{
	struct context *xp;

	if ((xp = cx.x_link) != 0) {
		cx = *xp;
		free((char *)xp);
	} else
		cx.x_type = 0;
}

cx_beginfile(filename)
char *filename;
{
	if (cx_alloc() < 0)
		return -1;
	cx.x_type = X_FILE;
	if ((cx.x_filename = str_cpy(filename)) == 0)
		goto bad;
	cx.x_fp = fopen(filename, "r");
	if (cx.x_fp == 0)
		goto bad;
	cx.x_bol = 1;
	cx.x_lineno = 0;
	cx.x_errwin = 0;
	cx.x_noerr = 0;
	return 0;
bad:
	if (cx.x_filename != 0)
		str_free(cx.x_filename);
	cx_free();
	return -1;
}

cx_beginbuf(buf, arg, narg)
char *buf;
struct value *arg;
int narg;
{
	if (cx_alloc() < 0)
		return -1;
	cx.x_type = X_BUF;
	cx.x_bufp = cx.x_buf = buf;
	cx.x_arg = arg;
	cx.x_narg = narg;
	return 0;
}

cx_end()
{
	switch (cx.x_type) {
	case X_BUF:
		break;
	case X_FILE:
		(void) fclose(cx.x_fp);
		str_free(cx.x_filename);
		break;
	}
	cx_free();
}
