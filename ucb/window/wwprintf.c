#ifndef lint
static char sccsid[] = "@(#)wwprintf.c	3.5 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"

/*VARARGS2*/
wwprintf(w, fmt, args)
struct ww *w;
char *fmt;
{
#include <stdio.h>
	struct _iobuf _wwbuf;
	char buf[1024];

	/*
	 * A danger is that when buf overflows, _flsbuf() will be
	 * called automatically.  It doesn't check for _IOSTR.
	 * We set the file descriptor to -1 so no actual io will be done.
	 */
	_wwbuf._flag = _IOWRT+_IOSTRG;
	_wwbuf._base = _wwbuf._ptr = buf;
	_wwbuf._cnt = sizeof buf;
	_wwbuf._file = -1;			/* safe */
	_doprnt(fmt, &args, &_wwbuf);
	(void) wwwrite(w, buf, _wwbuf._ptr - buf);
}
