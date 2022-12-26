#ifndef lint
static char sccsid[] = "@(#)tth19.c	3.17 5/2/86";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"
#include "tt.h"

/*
kb|h19|heath|h19-b|h19b|heathkit|heath-19|z19|zenith:
	cr=^M:nl=^J:bl=^G:al=1*\EL:am:le=^H:bs:cd=\EJ:ce=\EK:
	cl=\EE:cm=\EY%+ %+ :co#80:dc=\EN:dl=1*\EM:do=\EB:
	ei=\EO:ho=\EH:im=\E@:li#24:mi:nd=\EC:as=\EF:ae=\EG:ms:
	ta=^I:pt:sr=\EI:se=\Eq:so=\Ep:up=\EA:vs=\Ex4:ve=\Ey4:
	kb=^h:ku=\EA:kd=\EB:kl=\ED:kr=\EC:kh=\EH:
	kn#8:k1=\ES:k2=\ET:k3=\EU:k4=\EV:k5=\EW:
	l6=blue:l7=red:l8=white:k6=\EP:k7=\EQ:k8=\ER:
	es:hs:ts=\Ej\Ex5\Ex1\EY8%+ \Eo:fs=\Ek\Ey5:ds=\Ey1:
*/

#define NCOL	80
#define NROW	24

#define G (WWM_GRP << WWC_MSHIFT)
short h19_frame[16] = {
	' ',	'`'|G,	'a'|G,	'e'|G,
	'`'|G,	'`'|G,	'f'|G,	'v'|G,
	'a'|G,	'd'|G,	'a'|G,	'u'|G,
	'c'|G,	't'|G,	's'|G,	'b'|G
};

extern struct tt_str *gen_VS;
extern struct tt_str *gen_VE;
extern struct tt_str *gen_XS;
extern struct tt_str *gen_XE;

int h19_msp10c;

#define pc(c)	ttputc('c')
#define esc()	pc(\033)
#define PAD(ms10) { \
	register i; \
	for (i = ((ms10) + 5) / h19_msp10c; --i >= 0;) \
		pc(\0); \
}
#define ICPAD() PAD((NCOL - tt.tt_col) * 1)	/* 0.1 ms per char */
#define ILPAD() PAD((NROW - tt.tt_row) * 10)	/* 1 ms per char */

#define H19_SETINSERT(m) (esc(), (tt.tt_insert = (m)) ? pc(@) : pc(O))

h19_setinsert(new)
{
	H19_SETINSERT(new);
}

h19_setmodes(new)
register new;
{
	register diff;

	diff = new ^ tt.tt_modes;
	if (diff & WWM_REV) {
		esc();
		if (new & WWM_REV)
			pc(p);
		else
			pc(q);
	}
	if (diff & WWM_GRP) {
		esc();
		if (new & WWM_GRP)
			pc(F);
		else
			pc(G);
	}
	if (diff & WWM_USR) {
		if (new & WWM_USR) {
			if (gen_XS)
				ttxputs(gen_XS);
		} else
			if (gen_XE)
				ttxputs(gen_XE);
	}
	tt.tt_modes = new;
}

h19_insline()
{
	esc();
	pc(L);
	ILPAD();
}

h19_delline()
{
	esc();
	pc(M);
	ILPAD();
}

h19_putc(c)
register char c;
{
	if (tt.tt_nmodes != tt.tt_modes)
		(*tt.tt_setmodes)(tt.tt_nmodes);
	if (tt.tt_ninsert != tt.tt_insert)
		H19_SETINSERT(tt.tt_ninsert);
	ttputc(c);
	if (tt.tt_insert)
		ICPAD();
	if (++tt.tt_col == NCOL)
		tt.tt_col = NCOL - 1;
}

h19_write(p, n)
register char *p;
register n;
{
	if (tt.tt_nmodes != tt.tt_modes)
		(*tt.tt_setmodes)(tt.tt_nmodes);
	if (tt.tt_ninsert != tt.tt_insert)
		H19_SETINSERT(tt.tt_ninsert);
	if (tt.tt_insert) {
		while (--n >= 0) {
			ttputc(*p++);
			ICPAD();
			tt.tt_col++;
		}
	} else {
		tt.tt_col += n;
		ttwrite(p, n);
	}
	if (tt.tt_col == NCOL)
		tt.tt_col = NCOL - 1;
}

h19_move(row, col)
register char row, col;
{
	if (tt.tt_row == row) {
		if (tt.tt_col == col)
			return;
		if (tt.tt_col == col - 1) {
			esc();
			pc(C);
			goto out;
		} else if (tt.tt_col == col + 1) {
			pc(\b);
			goto out;
		}
	}
	if (tt.tt_col == col) {
		if (tt.tt_row == row + 1) {
			esc();
			pc(A);
			goto out;
		} else if (tt.tt_row == row - 1) {
			pc(\n);
			goto out;
		}
	}
	if (col == 0 && row == 0) {
		esc();
		pc(H);
		goto out;
	}
	esc();
	pc(Y);
	ttputc(' ' + row);
	ttputc(' ' + col);
out:
	tt.tt_col = col;
	tt.tt_row = row;
}

h19_init()
{
	if (gen_VS)
		ttxputs(gen_VS);
	esc();
	pc(w);
	esc();
	pc(E);
	tt.tt_col = tt.tt_row = 0;
	tt.tt_ninsert = tt.tt_insert = 0;
	tt.tt_nmodes = tt.tt_modes = 0;
}

h19_end()
{
	if (gen_VE)
		ttxputs(gen_VE);
	esc();
	pc(v);
}

h19_clreol()
{
	esc();
	pc(K);
}

h19_clreos()
{
	esc();
	pc(J);
}

h19_clear()
{
	esc();
	pc(E);
}

h19_delchar()
{
	esc();
	pc(N);
}

tt_h19()
{
	float cpms = (float) wwbaud / 10000;	/* char per ms */

	h19_msp10c = 10 / cpms;			/* ms per 10 char */
	gen_VS = ttxgetstr("vs");
	gen_VE = ttxgetstr("ve");
	gen_XS = ttxgetstr("XS");
	gen_XE = ttxgetstr("XE");

	tt.tt_init = h19_init;
	tt.tt_end = h19_end;

	tt.tt_insline = h19_insline;
	tt.tt_delline = h19_delline;
	tt.tt_delchar = h19_delchar;
	tt.tt_clreol = h19_clreol;
	tt.tt_clreos = h19_clreos;
	tt.tt_clear = h19_clear;
	tt.tt_move = h19_move;
	tt.tt_write = h19_write;
	tt.tt_putc = h19_putc;
	tt.tt_setinsert = h19_setinsert;
	tt.tt_setmodes = h19_setmodes;

	tt.tt_ncol = NCOL;
	tt.tt_nrow = NROW;
	tt.tt_hasinsert = 1;
	tt.tt_availmodes = WWM_REV|WWM_GRP;
	if (gen_XS)
		tt.tt_availmodes |= WWM_USR;
	tt.tt_frame = h19_frame;
	return 0;
}
