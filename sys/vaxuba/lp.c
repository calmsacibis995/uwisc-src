/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)lp.c	6.6 (Berkeley) 6/8/85
 */

#include "lp.h"
#if NLP > 0
/*
 * LP-11 Line printer driver
 *
 * This driver has been modified to work on printers where
 * leaving IENABLE set would cause continuous interrupts.
 *
 * Modified Fri Jan 13 12:58:46 CST 1984 by Dave Cohrs
 *	Added 'raw' mode device (set bit 1 in minor number) for no character
 *	processing -- useful for ln01 printer and font loading
 * Modified Wed Mar 21 12:44:05 CST 1984 by Dave Cohrs
 *  Got rid of raw mode and most of the driver -- no need to duplicate
 *	lpf's job.
 */
#include "../machine/pte.h"

#include "param.h"
#include "dir.h"
#include "user.h"
#include "buf.h"
#include "systm.h"
#include "map.h"
#include "uio.h"
#include "ioctl.h"
#include "tty.h"
#include "kernel.h"

#include "ubavar.h"

#define	LPPRI	(PZERO+8)
#define	IENABLE	0100
#define	DONE	0200
#define	ERROR	0100000
#define	LPLWAT	650
#define	LPHWAT	800

#define MAXCOL	132
#define CAP	1

#define LPUNIT(dev) (minor(dev) >> 3)

struct lpdevice {
	short	lpsr;
	short	lpbuf;
};

struct lp_softc {
	struct	clist sc_outq;
	int	sc_state;
	int	sc_physcol;
	int	sc_logcol;
	int	sc_physline;
	char	sc_flags;
	short	sc_maxcol;
	int	sc_lpchar;
	struct	buf *sc_inbuf;
} lp_softc[NLP];

struct uba_device *lpinfo[NLP];

int lpprobe(), lpattach(), lptout();
u_short lpstd[] = { 0177514 };
struct uba_driver lpdriver =
	{ lpprobe, 0, lpattach, 0, lpstd, "lp", lpinfo };

/* bits for state */
#define	OPEN		1	/* device is open */
#define	TOUT		2	/* timeout is active */
#define	MOD		4	/* device state has been modified */
#define	ASLP		8	/* awaiting draining of printer */

int	lptout();

lpattach(ui)
	struct uba_device *ui;
{
	register struct lp_softc *sc;

	sc = &lp_softc[ui->ui_unit];
	sc->sc_lpchar = -1;
	if (ui->ui_flags)
		sc->sc_maxcol = ui->ui_flags;
	else
		sc->sc_maxcol = MAXCOL;
}

lpprobe(reg)
	caddr_t reg;
{
	register int br, cvec;			/* value-result */
	register struct lpdevice *lpaddr = (struct lpdevice *)reg;
#ifdef lint
	br = 0; cvec = br; br = cvec;
	lpintr(0);
#endif

	lpaddr->lpsr = IENABLE;
	DELAY(5);
	lpaddr->lpsr = 0;
	return (sizeof (struct lpdevice));
}

/*ARGSUSED*/
lpopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct lpdevice *lpaddr;
	register struct lp_softc *sc;
	register struct uba_device *ui;
	register int unit, s;

	if ((unit = LPUNIT(dev)) >= NLP ||
	    (sc = &lp_softc[unit])->sc_state&OPEN ||
	    (ui = lpinfo[unit]) == 0 || ui->ui_alive == 0)
		return (ENXIO);
	lpaddr = (struct lpdevice *)ui->ui_addr;
	if (lpaddr->lpsr&ERROR)
		return (EIO);
	sc->sc_state |= OPEN;
	sc->sc_inbuf = geteblk(512);
	sc->sc_flags = minor(dev) & 07;
	s = spl4();
	if ((sc->sc_state&TOUT) == 0) {
		sc->sc_state |= TOUT;
		timeout(lptout, (caddr_t)dev, 10*hz);
	}
	splx(s);
#ifndef UW
	lpcanon(dev, '\f');
#endif UW
	return (0);
}

/*ARGSUSED*/
lpclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct lp_softc *sc = &lp_softc[LPUNIT(dev)];

#ifdef UW
	register int s;

	s = spl4();		/* flush remaining output */
	lpintr(LPUNIT(dev));
	splx(s);
#else
	lpcanon(dev, '\f');
#endif UW
	brelse(sc->sc_inbuf);
	sc->sc_state &= ~OPEN;
}

lpwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register unsigned n;
	register char *cp;
	register struct lp_softc *sc = &lp_softc[LPUNIT(dev)];
	int error;

	while (n = min(512, (unsigned)uio->uio_resid)) {
		cp = sc->sc_inbuf->b_un.b_addr;
		error = uiomove(cp, (int)n, UIO_WRITE, uio);
		if (error)
			return (error);
		do
#ifdef UW
			lpoutput(dev, *cp++);
#else
			lpcanon(dev, *cp++);
#endif UW
		while (--n);
	}
	return (0);
}

#ifndef UW
lpcanon(dev, c)
	dev_t dev;
	register int c;
{
	register struct lp_softc *sc = &lp_softc[LPUNIT(dev)];
	register int logcol, physcol, s;

	if (sc->sc_flags&CAP) {
		register c2;

		if (c>='a' && c<='z')
			c += 'A'-'a'; else
		switch (c) {

		case '{':
			c2 = '(';
			goto esc;

		case '}':
			c2 = ')';
			goto esc;

		case '`':
			c2 = '\'';
			goto esc;

		case '|':
			c2 = '!';
			goto esc;

		case '~':
			c2 = '^';

		esc:
			lpcanon(dev, c2);
			sc->sc_logcol--;
			c = '-';
		}
	}
	logcol = sc->sc_logcol;
	physcol = sc->sc_physcol;
	if (c == ' ')
		logcol++;
	else switch(c) {

	case '\t':
		logcol = (logcol+8) & ~7;
		break;

	case '\f':
		if (sc->sc_physline == 0 && physcol == 0)
			break;
		/* fall into ... */

	case '\n':
		lpoutput(dev, c);
		if (c == '\f')
			sc->sc_physline = 0;
		else
			sc->sc_physline++;
		physcol = 0;
		/* fall into ... */

	case '\r':
		s = spl4();
		logcol = 0;
		lpintr(LPUNIT(dev));
		splx(s);
		break;

	case '\b':
		if (logcol > 0)
			logcol--;
		break;

	default:
		if (logcol < physcol) {
			lpoutput(dev, '\r');
			physcol = 0;
		}
		if (logcol < sc->sc_maxcol) {
			while (logcol > physcol) {
				lpoutput(dev, ' ');
				physcol++;
			}
			lpoutput(dev, c);
			physcol++;
		}
		logcol++;
	}
	if (logcol > 1000)	/* ignore long lines  */
		logcol = 1000;
	sc->sc_logcol = logcol;
	sc->sc_physcol = physcol;
}
#endif UW

lpoutput(dev, c)
	dev_t dev;
	int c;
{
	register struct lp_softc *sc = &lp_softc[LPUNIT(dev)];
	int s;

	if (sc->sc_outq.c_cc >= LPHWAT) {
		s = spl4();
		lpintr(LPUNIT(dev));				/* unchoke */
		while (sc->sc_outq.c_cc >= LPHWAT) {
			sc->sc_state |= ASLP;		/* must be ERROR */
			sleep((caddr_t)sc, LPPRI);
		}
		splx(s);
	}
	while (putc(c, &sc->sc_outq))
		sleep((caddr_t)&lbolt, LPPRI);
}

lpintr(lp11)
	int lp11;
{
	register int n;
	register struct lp_softc *sc = &lp_softc[lp11];
	register struct uba_device *ui = lpinfo[lp11];
	register struct lpdevice *lpaddr = (struct lpdevice *)ui->ui_addr;

#ifdef UW
retry:	/* see below -- race breaker */
#endif UW
	lpaddr->lpsr &= ~IENABLE;
	n = sc->sc_outq.c_cc;
	if (sc->sc_lpchar < 0)
		sc->sc_lpchar = getc(&sc->sc_outq);
	while ((lpaddr->lpsr&DONE) && sc->sc_lpchar >= 0) {
		lpaddr->lpbuf = sc->sc_lpchar;
		sc->sc_lpchar = getc(&sc->sc_outq);
	}
	sc->sc_state |= MOD;
	if (sc->sc_outq.c_cc > 0 && (lpaddr->lpsr&ERROR)==0) {
		lpaddr->lpsr |= IENABLE;	/* ok and more to do later */
#ifdef UW
		/* avoid race condition */
		if (lpaddr->lpsr&DONE)
			goto retry;
#endif UW
	}
	if (n>LPLWAT && sc->sc_outq.c_cc<=LPLWAT && sc->sc_state&ASLP) {
		sc->sc_state &= ~ASLP;
		wakeup((caddr_t)sc);		/* top half should go on */
	}
}

lptout(dev)
	dev_t dev;
{
	register struct lp_softc *sc;
	register struct uba_device *ui;
	register struct lpdevice *lpaddr;

	sc = &lp_softc[LPUNIT(dev)];
	ui = lpinfo[LPUNIT(dev)];
	lpaddr = (struct lpdevice *) ui->ui_addr;
	if ((sc->sc_state&MOD) != 0) {
		sc->sc_state &= ~MOD;		/* something happened */
		timeout(lptout, (caddr_t)dev, 2*hz);	/* so don't sweat */
		return;
	}
	if ((sc->sc_state&OPEN) == 0 && sc->sc_outq.c_cc == 0) {
		sc->sc_state &= ~TOUT;		/* no longer open */
						/* and no chars to send */
		lpaddr->lpsr = 0;
		return;
	}
	if (sc->sc_outq.c_cc && (lpaddr->lpsr&DONE) && (lpaddr->lpsr&ERROR)==0)
		lpintr(LPUNIT(dev));			/* ready to go */
	timeout(lptout, (caddr_t)dev, 10*hz);
}

lpreset(uban)
	int uban;
{
	register struct uba_device *ui;
	register struct lpdevice *lpaddr;
	register int unit;

	for (unit = 0; unit < NLP; unit++) {
		if ((ui = lpinfo[unit]) == 0 || ui->ui_ubanum != uban ||
		    ui->ui_alive == 0)
			continue;
		printf(" lp%d", unit);
		lpaddr = (struct lpdevice *)ui->ui_addr;
		lpaddr->lpsr |= IENABLE;
	}
}
#endif
