#ifndef lint
static char sccsid[] = "@(#)wwdump.c	3.11 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"
#include "tt.h"

static char cmap[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

wwdumpwin(w)
register struct ww *w;
{
	register i, j;

	tt.tt_nmodes = 0;
	(*tt.tt_clear)();
	for (i = w->ww_i.t; i < w->ww_i.b; i++) {
		(*tt.tt_move)(i, w->ww_i.l);
		for (j = w->ww_i.l; j < w->ww_i.r; j++)
			(*tt.tt_putc)(w->ww_win[i][j] & WWM_GLS ? 'G' : ' ');
	}
}

wwdumpnvis(w)
register struct ww *w;
{
	register i;
	char buf[20];

	tt.tt_nmodes = 0;
	(*tt.tt_clear)();
	for (i = w->ww_i.t; i < w->ww_i.b; i++) {
		(*tt.tt_move)(i, w->ww_i.l);
		(void) sprintf(buf, "%d", w->ww_nvis[i]);
		(*tt.tt_write)(buf, strlen(buf));
	}
}

wwdumpsmap()
{
	register i, j;

	tt.tt_nmodes = 0;
	(*tt.tt_clear)();
	for (i = 0; i < wwnrow; i++) {
		(*tt.tt_move)(i, 0);
		for (j = 0; j < wwncol; j++)
			(*tt.tt_putc)(cmap[wwsmap[i][j]]);
	}
}

wwdumpns()
{
	register i, j;

	(*tt.tt_clear)();
	for (i = 0; i < wwnrow; i++) {
		(*tt.tt_move)(i, 0);
		for (j = 0; j < wwncol; j++) {
			tt.tt_nmodes = wwns[i][j].c_m & tt.tt_availmodes;
			(*tt.tt_putc)(wwns[i][j].c_c);
		}
	}
}

wwdumpos()
{
	register i, j;

	(*tt.tt_clear)();
	for (i = 0; i < wwnrow; i++) {
		(*tt.tt_move)(i, 0);
		for (j = 0; j < wwncol; j++) {
			tt.tt_nmodes = wwos[i][j].c_m & tt.tt_availmodes;
			(*tt.tt_putc)(wwns[i][j].c_c);
		}
	}
}
