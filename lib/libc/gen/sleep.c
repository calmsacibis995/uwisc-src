#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: sleep.c,v 1.3 86/09/08 14:45:23 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sleep.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/time.h>
#include <signal.h>

#define	setvec(vec, a) \
	vec.sv_handler = a; vec.sv_mask = vec.sv_onstack = 0

static int ringring;

sleep(n)
	unsigned n;
{
	int sleepx(), omask;
	struct itimerval itv, oitv;
	register struct itimerval *itp = &itv;
	struct sigvec vec, ovec;

	if (n == 0)
		return;
	timerclear(&itp->it_interval);
	timerclear(&itp->it_value);
	if (setitimer(ITIMER_REAL, itp, &oitv) < 0)
		return;
	itp->it_value.tv_sec = n;
	if (timerisset(&oitv.it_value)) {
		if (timercmp(&oitv.it_value, &itp->it_value, >))
			oitv.it_value.tv_sec -= itp->it_value.tv_sec;
		else {
			itp->it_value = oitv.it_value;
			/*
			 * This is a hack, but we must have time to
			 * return from the setitimer after the alarm
			 * or else it'll be restarted.  And, anyway,
			 * sleep never did anything more than this before.
			 */
			oitv.it_value.tv_sec = 1;
			oitv.it_value.tv_usec = 0;
		}
	}
	setvec(vec, sleepx);
	(void) sigvec(SIGALRM, &vec, &ovec);
	omask = sigblock(sigmask(SIGALRM));
	ringring = 0;
	(void) setitimer(ITIMER_REAL, itp, (struct itimerval *)0);
	while (!ringring)
		sigpause(omask &~ sigmask(SIGALRM));
	(void) sigvec(SIGALRM, &ovec, (struct sigvec *)0);
	(void) sigsetmask(omask);
	(void) setitimer(ITIMER_REAL, &oitv, (struct itimerval *)0);
}

static
sleepx()
{

	ringring = 1;
}
