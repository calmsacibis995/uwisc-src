#ifndef lint
static char sccsid[] = "@(#)USRob.c	4.4 (Berkeley) 6/25/83";
#endif

#include "tip.h"

static	int sigALRM();
static	int timeout = 0;
static	jmp_buf timeoutbuf;

/*
 * Dial up on a USRobotics Auto Dialer 
 *	ALways pulse dial (ours phones are thus limited)	
 */

USRob_dialer(num, mod)
	char *num, *mod;
{
	register int connected = 0, i, num_len ;
	char cbuf[40] , c ;

	if (boolean(value(VERBOSE)))
		printf("\nstarting call...");

/* Need something here to eat garbage on the line */

	eat_junk();	/* Eat junk on the line */

	strcpy(cbuf, "AT\r");
	write(FD,cbuf,strlen(cbuf));
	if( ! detect( "\r\nOK\r\n")) {
		printf("Auto-dialer won't respond, giving up .....");
		return(0);
	}

	num_len = strlen(num);
	for(i=0; i < num_len ; i++ ){
	    if ( num[i] == '=' )  num[i] = ',' ;
        }	
	strcpy(cbuf, "ATDP");
	strcat(cbuf, num);
	strcat(cbuf, "\r");
	write(FD, cbuf, strlen(cbuf)); 

	if (boolean(value(VERBOSE)))
		printf("ringing...");
	/*
	 * The reply from the USRobotics should be:
	 *	\r\nNO CARRIER\r\n	failure
	 *	\r\nCONNECT\r\n		success
	 */
	connected = detect("\r\nCONNECT\r\n");
#ifdef ACULOG
	if (timeout) {
		char line[80];

		sprintf(line, "%d dial timeout",
		logent(value(HOST), num, "USRob", line);
	}
#endif
	if (timeout)
		USRob_disconnect();	/* insurance */
	return (connected);
}


USRob_disconnect()
{
	int rw = 2;

	ioctl(FD, TIOCFLUSH, &rw);
}

USRob_abort()
{
	USRob_disconnect();
}

static int
sigALRM()
{

	timeout = 1;
	longjmp(timeoutbuf, 1);
}


static int
detect(s)
	register char *s;
{
	char c;
	int (*f)();

	f = signal(SIGALRM, sigALRM);
	timeout = 0;
	while (*s) {
		if (setjmp(timeoutbuf)) {
			USRob_abort();
			break;
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
		if (c != *s++)
			return (0);
	}
	signal(SIGALRM, f);
	return (timeout == 0);
}
eat_junk()
{
	char c;
	int (*f)();

	f = signal(SIGALRM, sigALRM);
	timeout = 0;
	for(;;) {
		if (setjmp(timeoutbuf)) {
			break;
		}
		alarm(5);
		read(FD, &c, 1);
		alarm(0);
	}
	signal(SIGALRM, f);
	return (timeout == 0);
}
