#ifndef lint
static char sccsid[] = "@(#)wwinschar.c	3.14 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"
#include "tt.h"

wwinschar(w, row, col, c)
register struct ww *w;
short c;
{
	register i;
	int nvis;

	/*
	 * First, shift the line.
	 */
	{
		register union ww_char *p, *q;

		p = &w->ww_buf[row][w->ww_b.r];
		q = p - 1;
		for (i = w->ww_b.r - col; --i > 0;)
			*--p = *--q;
		q->c_w = c;
	}

	/*
	 * If can't see it, just return.
	 */
	if (row < w->ww_i.t || row >= w->ww_i.b
	    || w->ww_i.r <= 0 || w->ww_i.r <= col)
		return;

	if (col < w->ww_i.l)
		col = w->ww_i.l;

	/*
	 * Now find out how much is actually changed, and fix wwns.
	 */
	{
		register union ww_char *buf;
		register char *win;
		register union ww_char *ns;
		register char *smap;
		char touched;

		nvis = 0;
		smap = &wwsmap[row][col];
		for (i = col; i < w->ww_i.r && *smap++ != w->ww_index; i++)
			;
		if (i >= w->ww_i.r)
			return;
		col = i;
		buf = w->ww_buf[row];
		win = w->ww_win[row];
		ns = wwns[row];
		smap = &wwsmap[row][i];
		touched = wwtouched[row];
		for (; i < w->ww_i.r; i++) {
			if (*smap++ != w->ww_index)
				continue;
			touched |= WWU_TOUCHED;
			if (win[i])
				ns[i].c_w =
					buf[i].c_w ^ win[i] << WWC_MSHIFT;
			else {
				nvis++;
				ns[i] = buf[i];
			}
		}
		wwtouched[row] = touched;
	}

	/*
	 * Can/Should we use delete character?
	 */
	if (tt.tt_hasinsert && nvis > (wwncol - col) / 2) {
		register union ww_char *p, *q;

		tt.tt_ninsert = 1;
		tt.tt_nmodes = c >> WWC_MSHIFT & tt.tt_availmodes;
		(*tt.tt_move)(row, col);
		(*tt.tt_putc)(c & WWC_CMASK);
		tt.tt_ninsert = 0;

		p = &wwos[row][wwncol];
		q = p - 1;
		for (i = wwncol - col; --i > 0;)
			*--p = *--q;
		q->c_w = c;
	}
}
