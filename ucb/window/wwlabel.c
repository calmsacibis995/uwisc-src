#ifndef lint
static char sccsid[] = "@(#)wwlabel.c	3.13 4/24/85";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"
#include "char.h"

/*
 * Label window w on f,
 * at 1 line above w and 'where' columns from it's left edge.
 * Gross, but it works.
 */
wwlabel(w, f, where, l, mode)
struct ww *w;
struct ww *f;
char *l;
{
	int row;
	register j;
	int jj;
	register char *win;
	register union ww_char *buf;
	register union ww_char *ns;
	register char *fmap;
	register char *smap;
	char touched;
	char *p;

	if (f->ww_fmap == 0)
		return;

	row = w->ww_w.t - 1;
	if (row < f->ww_i.t || row >= f->ww_i.b)
		return;
	win = f->ww_win[row];
	buf = f->ww_buf[row];
	fmap = f->ww_fmap[row];
	ns = wwns[row];
	smap = wwsmap[row];
	touched = wwtouched[row];
	mode <<= WWC_MSHIFT;

	jj = MIN(w->ww_i.r, f->ww_i.r);
	j = w->ww_i.l + where;
	while (j < jj && *l)
		for (p = unctrl(*l++); j < jj && *p; j++, p++) {
			/* can't label if not already framed */
			if (win[j] & WWM_GLS)
				continue;
			if (smap[j] != f->ww_index)
				buf[j].c_w = mode | *p;
			else {
				ns[j].c_w = (buf[j].c_w = mode | *p)
						^ win[j] << WWC_MSHIFT;
				touched |= WWU_TOUCHED;
			}
			fmap[j] |= WWF_LABEL;
		}
	wwtouched[row] = touched;
}
