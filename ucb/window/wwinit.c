#ifndef lint
static char sccsid[] = "@(#)wwinit.c	3.26 5/1/86";
#endif

/*
 * Copyright (c) 1983 Regents of the University of California,
 * All rights reserved.  Redistribution permitted subject to
 * the terms of the Berkeley Software License Agreement.
 */

#include "ww.h"
#include "tt.h"
#include <sys/signal.h>
#include <fcntl.h>
#include "char.h"

wwinit()
{
	register i, j;
	char *kp;
	register char **p, **q;
	char **env, **termcap = 0;
	extern char **environ;
	int s;

	wwdtablesize = getdtablesize();
	wwhead.ww_forw = &wwhead;
	wwhead.ww_back = &wwhead;

	s = sigblock(sigmask(SIGIO));
	if (signal(SIGIO, wwrint) == BADSIG)
		return -1;

	if (wwgettty(0, &wwoldtty) < 0)
		return -1;
	wwwintty = wwoldtty;
	wwwintty.ww_sgttyb.sg_flags &= ~XTABS;
	wwnewtty.ww_sgttyb = wwoldtty.ww_sgttyb;
	wwnewtty.ww_sgttyb.sg_erase = -1;
	wwnewtty.ww_sgttyb.sg_kill = -1;
	wwnewtty.ww_sgttyb.sg_flags |= CBREAK;
	wwnewtty.ww_sgttyb.sg_flags &= ~(ECHO|CRMOD);
	wwnewtty.ww_tchars.t_intrc = -1;
	wwnewtty.ww_tchars.t_quitc = -1;
	wwnewtty.ww_tchars.t_startc = -1;
	wwnewtty.ww_tchars.t_stopc = -1;
	wwnewtty.ww_tchars.t_eofc = -1;
	wwnewtty.ww_tchars.t_brkc = -1;
	wwnewtty.ww_ltchars.t_suspc = -1;
	wwnewtty.ww_ltchars.t_dsuspc = -1;
	wwnewtty.ww_ltchars.t_rprntc = -1;
	wwnewtty.ww_ltchars.t_flushc = -1;
	wwnewtty.ww_ltchars.t_werasc = -1;
	wwnewtty.ww_ltchars.t_lnextc = -1;
	wwnewtty.ww_lmode = wwoldtty.ww_lmode | LLITOUT;
	wwnewtty.ww_ldisc = wwoldtty.ww_ldisc;
	wwnewtty.ww_fflags = wwoldtty.ww_fflags | FASYNC;
	if (wwsettty(0, &wwnewtty, &wwoldtty) < 0)
		goto bad;

	if ((wwterm = getenv("TERM")) == 0) {
		wwerrno = WWE_BADTERM;
		goto bad;
	}
	if (tgetent(wwtermcap, wwterm) != 1) {
		wwerrno = WWE_BADTERM;
		goto bad;
	}
	wwbaud = wwbaudmap[wwoldtty.ww_sgttyb.sg_ospeed];

	if (ttinit() < 0)
		goto bad;
	wwnrow = tt.tt_nrow;
	wwncol = tt.tt_ncol;
	wwavailmodes = tt.tt_availmodes;
	wwwrap = tt.tt_wrap;
	(*tt.tt_init)();

	if (wwavailmodes & WWM_REV)
		wwcursormodes = WWM_REV | wwavailmodes & WWM_BLK;
	else if (wwavailmodes & WWM_UL)
		wwcursormodes = WWM_UL;

	if ((wwib = malloc((unsigned) 512)) == 0)
		goto bad;
	wwibe = wwib + 512;
	wwibq = wwibp = wwib;

	if ((wwsmap = wwalloc(0, 0, wwnrow, wwncol, sizeof (char))) == 0)
		goto bad;
	for (i = 0; i < wwnrow; i++)
		for (j = 0; j < wwncol; j++)
			wwsmap[i][j] = WWX_NOBODY;

	wwos = (union ww_char **)
		wwalloc(0, 0, wwnrow, wwncol, sizeof (union ww_char));
	if (wwos == 0)
		goto bad;
	for (i = 0; i < wwnrow; i++)
		for (j = 0; j < wwncol; j++)
			wwos[i][j].c_w = ' ';
	wwns = (union ww_char **)
		wwalloc(0, 0, wwnrow, wwncol, sizeof (union ww_char));
	if (wwns == 0)
		goto bad;
	for (i = 0; i < wwnrow; i++)
		for (j = 0; j < wwncol; j++)
			wwns[i][j].c_w = ' ';

	wwtouched = malloc((unsigned) wwnrow);
	if (wwtouched == 0) {
		wwerrno = WWE_NOMEM;
		goto bad;
	}
	for (i = 0; i < wwnrow; i++)
		wwtouched[i] = 0;

	wwindex[WWX_NOBODY] = &wwnobody;
	wwnobody.ww_order = NWW;

	kp = wwkeys;
	wwaddcap("kb", &kp);
	wwaddcap("ku", &kp);
	wwaddcap("kd", &kp);
	wwaddcap("kl", &kp);
	wwaddcap("kr", &kp);
	wwaddcap("kh", &kp);
	if ((j = tgetnum("kn")) >= 0) {
		char cap[32];

		(void) sprintf(kp, "kn#%d:", j);
		for (; *kp; kp++)
			;
		for (i = 1; i <= j; i++) {
			(void) sprintf(cap, "k%d", i);
			wwaddcap(cap, &kp);
			cap[0] = 'l';
			wwaddcap(cap, &kp);
		}
	}
	for (i = 0, p = environ; *p++; i++)
		;
	if ((env = (char **)malloc((unsigned)(i + 3) * sizeof (char *))) == 0)
		goto bad;
	for (p = environ, q = env; *p; p++, q++) {
		if (strncmp(*p, "TERM=", 5) == 0)
			*q = WWT_TERM;
		else if (strncmp(*p, "TERMCAP=", 8) == 0)
			termcap = q;
		else
			*q = *p;
	}
	*(termcap ? termcap : q++) = wwwintermcap;
	*q = 0;
	environ = env;

	(void) signal(SIGPIPE, SIG_IGN);
	(void) sigsetmask(s);
	return 0;
bad:
	/*
	 * Don't bother to free storage.  We're supposed
	 * to exit when wwinit fails anyway.
	 */
	(void) wwsettty(0, &wwoldtty, &wwnewtty);
	(void) signal(SIGIO, SIG_DFL);
	(void) sigsetmask(s);
	return -1;
}

wwaddcap(cap, kp)
register char *cap;
register char **kp;
{
	char tbuf[512];
	char *tp = tbuf;
	register char *str, *p;

	if ((str = tgetstr(cap, &tp)) != 0) {
		while (*(*kp)++ = *cap++)
			;
		(*kp)[-1] = '=';
		while (*str) {
			for (p = unctrl(*str++); *(*kp)++ = *p++;)
				;
			(*kp)--;
		}
		*(*kp)++ = ':';
		**kp = 0;
	}
}
