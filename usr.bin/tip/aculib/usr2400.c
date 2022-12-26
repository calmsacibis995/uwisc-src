#define USR2400
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)usr2400.c	5.1 (Berkeley) 4/30/85";
#endif not lint

/*
 * Routines for calling up on a Hayes Modem
 * (based on the old VenTel driver).
 */
#include "tip.h"
#include <sgtty.h>

#define	min(a,b)	((a < b) ? a : b)

static	int sigALRM();
static	jmp_buf timeoutbuf;
static char gobblebuf[100];

#define	DIALING		1
#define IDLE		2
#define CONNECTED	3
#define	FAILED		4
extern int Debug;

#ifdef USR2400
#define ATDT	"ATD"
#else HAYES
#define ATDT	"ATDT"		/* touch tone */
/*#define ATDT	"ATDP"		/* pulse */
#endif HAYES


static int
gobble(send, match)
char *send;
char *match;
{
	register char *cp;

	if (Debug)
		printf("--->\t%s\n", send);
	write(FD, send, strlen(send));
	signal(SIGALRM, sigALRM);
	if (match == NULL)
		return 0;
	do {
		alarm(45);
		cp = gobblebuf;
		while (read(FD, cp ,1) == 1)
			if (*cp >= ' ')
				break;
		while (++cp < &gobblebuf[100] && read(FD, cp, 1) == 1 && *cp != '\n')
			;
		alarm(0);
		*cp-- = '\0';
		if (*cp == '\r')
			*cp = '\0';
		if (Debug)
			printf("<---\t%s\n", gobblebuf);
	} while (strncmp(gobblebuf, "RING", 4) == 0);
	return  strncmp(gobblebuf, match, strlen(match));
}

usr2400_dialer(num, acu)
	register char *num;
	char *acu;
{
	char tbuffer[BUFSIZ];
	int zero = 0, baud, gobflg;

	if (boolean(value(VERBOSE)))
		printf("\ndialing %s ...", num);
	fflush(stdout);
	ioctl(FD, TIOCHPCL, 0);
	ioctl(FD, TIOCFLUSH, &zero);	/* get rid of garbage */
	write(FD, "\r", 1);
	if (setjmp(timeoutbuf)) {
#ifdef ACULOG
		char line[80];
		sprintf(line, "%d second dial timeout",
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "usr2400", line);
#endif
		usr2400_disconnect();	/* insurance */
		return 0;
	}

	(void) gobble("ATV1E0H\r", "OK");
#ifdef USR2400
	(void) gobble("ATX6S7=44\r", "OK");
#endif
#ifdef	ITTACCT
	/*
	 * N.B. This assumes ALL long distance calls got through the
	 * ITT WATS line...
	 */
	if (strcmp(acu, "/dev/cul1") == 0) {
		sprintf(tbuffer, "%s%s,%4d\r",ATDT, num, getuid()+1000);
	} else
#endif	ITTACCT
		sprintf(tbuffer, "%s%s\r", ATDT, num);
	gobflg = gobble(tbuffer, "CONNECT");
	if (boolean(value(VERBOSE)))
		printf(" (%s) ", gobblebuf);
	if (gobflg != 0) {
		return 0;	/* lets get out of here.. */
	}
	baud = atoi(&gobblebuf[7]);
	if (baud > 0 && baud != BR) { /* reset baud rate */
		ttysetup(baud);
	} else
		ioctl(FD, TIOCFLUSH, &zero);
	return 1;
}


usr2400_disconnect()
{
	char c;
	int len, rlen;

	/* first hang up the modem*/
	if (Debug)
		printf("\rdisconnecting modem....\n\r");
	ioctl(FD, TIOCCDTR, 0);
	sleep(1);
	ioctl(FD, TIOCSDTR, 0);

	sleep(2);
	gobble("ATZ\r", (char *)0);
	sleep(1);
	close(FD);
}

usr2400_abort()
{
	char c;

	write(FD, "\r", 1);	/* send anything to abort the call */
	usr2400_disconnect();
}

static int
sigALRM()
{

	printf("\07timeout waiting for reply\n\r");
	longjmp(timeoutbuf, 1);
}
