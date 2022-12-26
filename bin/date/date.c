/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)date.c	4.19 (Berkeley) 5/18/86";
#endif not lint

/*
 * Date - print and set date
 */

#include <sys/param.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/file.h>
#include <errno.h>
#include <syslog.h>
#include <utmp.h>

#define WTMP	"/usr/adm/wtmp"

struct	timeval tv, now;
struct	timezone tz;
char	*ap, *ep, *sp;
int	uflag, nflag;
int	retval;

char	*timezone();
static	int dmsize[12] =
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#ifdef UW
static char *usage = "usage: date [-n] [-u] [+format] [yymmddhhmm[.ss]]\n";

#define MONTH   itoa(tp->tm_mon+1,cp,2)
#define DAY     itoa(tp->tm_mday,cp,2)
#define YEAR    itoa(tp->tm_year,cp,2)
#define HOUR    itoa(tp->tm_hour,cp,2)
#define MINUTE  itoa(tp->tm_min,cp,2)
#define SECOND  itoa(tp->tm_sec,cp,2)
#define JULIAN  itoa(tp->tm_yday+1,cp,3)
#define WEEKDAY itoa(tp->tm_wday,cp,1)
#define MODHOUR itoa(h,cp,2)
#define ZONE    itoa(tz.tz_minuteswest,cp,4)
#define ROMAN   roman(tp->tm_mon,cp)
#define dysize(A) (((A)%4)? 365: 366)

char    month[12][3] = {
	"Jan","Feb","Mar","Apr",
	"May","Jun","Jul","Aug",
	"Sep","Oct","Nov","Dec"
};

char	days[7][3] = {
	"Sun","Mon","Tue","Wed",
	"Thu","Fri","Sat"
};

char	*long_days[] = {
	"Sunday","Monday","Tuesday",
	"Wednesday","Thursday","Friday","Saturday"
};

char	*dst_type = "nuawme";
char	*itoa(),*roman();

#else
static	char *usage = "usage: date [-n] [-u] [yymmddhhmm[.ss]]\n";
#endif UW

struct utmp wtmp[2] = {
	{ "|", "", "", 0 },
	{ "{", "", "", 0 }
};

char	*ctime();
char	*asctime();
struct	tm *localtime();
struct	tm *gmtime();
#ifdef UW
struct tm *tp;
#endif UW

char	*strcpy(), *strncpy();
char	*username, *getlogin();
long	time();
uid_t	getuid();

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *tzn;

	openlog("date", LOG_ODELAY, LOG_AUTH);
	(void) gettimeofday(&tv, &tz);
	now = tv;

	while (argc > 1 && argv[1][0] == '-') {
		while (*++argv[1])
		    switch ((int)argv[1][0]) {

		    case 'n':
			nflag++;
			break;

		    case 'u':
			uflag++;
			break;

		    default:
			fprintf(stderr, usage);
			exit(1);
		}
		argc--;
		argv++;
	}
#ifdef UW
	if(*argv[1] == '+') {
		if( uflag )
			tp = gmtime(&tv.tv_sec);
		else
			tp = localtime(&tv.tv_sec);
		prnt_date( argv[1] );
	}
#endif UW
	if (argc > 2) {
		fprintf(stderr, usage);
		exit(1);
	}
	if (argc == 1) 
		goto display;

	if (getuid() != 0) {
		fprintf(stderr, "You are not superuser: date not set\n");
		retval = 1;
		goto display;
	}
	username = getlogin();
	if (username == NULL || *username == '\0')  /* single-user or no tty */
		username = "root";

	ap = argv[1];
	wtmp[0].ut_time = tv.tv_sec;
	if (gtime()) {
		fprintf(stderr, usage);
		retval = 1;
		goto display;
	}
	/* convert to GMT assuming local time */
	if (uflag == 0) {
		tv.tv_sec += (long)tz.tz_minuteswest*60;
		/* now fix up local daylight time */
		if (localtime((time_t *)&tv.tv_sec)->tm_isdst)
			tv.tv_sec -= 60*60;
	}
	if (nflag || !settime(tv)) {
		int wf;

		if (settimeofday(&tv, (struct timezone *)0) < 0) {
			perror("settimeofday");
			retval = 1;
			goto display;
		}
		if ((wf = open(WTMP, O_WRONLY|O_APPEND)) >= 0) {
			(void) time((time_t *)&wtmp[1].ut_time);
			(void) write(wf, (char *)wtmp, sizeof(wtmp));
			(void) close(wf);
		}
	}
	syslog(LOG_NOTICE, "set by %s", username);

display:
	(void) gettimeofday(&tv, (struct timezone *)0);
	if (uflag) {
		ap = asctime(gmtime((time_t *)&tv.tv_sec));
		tzn = "GMT";
	} else {
		struct tm *tp;
		tp = localtime((time_t *)&tv.tv_sec);
		ap = asctime(tp);
		tzn = timezone(tz.tz_minuteswest, tp->tm_isdst);
	}
	printf("%.20s", ap);
	if (tzn)
		printf("%s", tzn);
	printf("%s", ap+19);
	exit(retval);
}

gtime()
{
	register int i, year, month;
	int day, hour, mins, secs;
	struct tm *L;
	char x;

	ep = ap;
	while(*ep) ep++;
	sp = ap;
	while(sp < ep) {
		x = *sp;
		*sp++ = *--ep;
		*ep = x;
	}
	sp = ap;
	(void) gettimeofday(&tv, (struct timezone *)0);
	L = localtime((time_t *)&tv.tv_sec);
	secs = gp(-1);
	if (*sp != '.') {
		mins = secs;
		secs = 0;
	} else {
		sp++;
		mins = gp(-1);
	}
	hour = gp(-1);
	day = gp(L->tm_mday);
	month = gp(L->tm_mon+1);
	year = gp(L->tm_year);
	if (*sp)
		return (1);
	if (month < 1 || month > 12 ||
	    day < 1 || day > 31 ||
	    mins < 0 || mins > 59 ||
	    secs < 0 || secs > 59)
		return (1);
	if (hour == 24) {
		hour = 0;
		day++;
	}
	if (hour < 0 || hour > 23)
		return (1);
	tv.tv_sec = 0;
	year += 1900;
	for (i = 1970; i < year; i++)
		tv.tv_sec += dysize(i);
	/* Leap year */
	if (dysize(year) == 366 && month >= 3)
		tv.tv_sec++;
	while (--month)
		tv.tv_sec += dmsize[month-1];
	tv.tv_sec += day-1;
	tv.tv_sec = 24*tv.tv_sec + hour;
	tv.tv_sec = 60*tv.tv_sec + mins;
	tv.tv_sec = 60*tv.tv_sec + secs;
	return (0);
}

gp(dfault)
{
	register int c, d;

	if (*sp == 0)
		return (dfault);
	c = (*sp++) - '0';
	d = (*sp ? (*sp++) - '0' : 0);
	if (c < 0 || c > 9 || d < 0 || d > 9)
		return (-1);
	return (c+10*d);
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#define WAITACK		2	/* seconds */
#define WAITDATEACK	5	/* seconds */

extern	int errno;
/*
 * Set the date in the machines controlled by timedaemons
 * by communicating the new date to the local timedaemon. 
 * If the timedaemon is in the master state, it performs the
 * correction on all slaves.  If it is in the slave state, it
 * notifies the master that a correction is needed.
 * Returns 1 on success, 0 on failure.
 */
settime(tv)
	struct timeval tv;
{
	int s, length, port, timed_ack, found, err;
	long waittime;
	fd_set ready;
	char hostname[MAXHOSTNAMELEN];
	struct timeval tout;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in sin, dest, from;

	sp = getservbyname("timed", "udp");
	if (sp == 0) {
		fprintf(stderr, "udp/timed: unknown service\n");
		retval = 2;
		return (0);
	}	
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EPROTONOSUPPORT)
			perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		sin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
		fprintf(stderr, "date: All ports in use\n");
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	(void) gethostname(hostname, sizeof (hostname));
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)tv.tv_sec);
	msg.tsp_time.tv_usec = htonl((u_long)tv.tv_usec);
	length = sizeof (struct sockaddr_in);
	if (connect(s, &dest, length) < 0) {
		perror("date: connect");
		goto bad;
	}
	if (send(s, (char *)&msg, sizeof (struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			perror("date: send");
		goto bad;
	}
	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;
	FD_ZERO(&ready);
	FD_SET(s, &ready);
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &length) == 0
	    && err) {
		errno = err;
		if (errno != ECONNREFUSED)
			perror("date: send (delayed error)");
		goto bad;
	}
	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof (struct sockaddr_in);
		if (recvfrom(s, (char *)&msg, sizeof (struct tsp), 0, &from,
		    &length) < 0) {
			if (errno != ECONNREFUSED)
				perror("date: recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);
		switch (msg.tsp_type) {

		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;

		case TSP_DATEACK:
			(void)close(s);
			return (1);

		default:
			fprintf(stderr,
			    "date: Wrong ack received from timed: %s\n", 
			    tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1)
		fprintf(stderr,
		    "date: Can't reach time daemon, time set locally.\n");
bad:
	(void)close(s);
	retval = 2;
	return (0);
}

#ifdef UW
/*
 * Special version of integer to ascii conversion for prnt_format.
 */
char *
itoa(i,ptr,dig)
register  int	i;
register  int	dig;
register  char	*ptr;
{
	switch(dig)	{
		case 4: 
			*ptr++ = i/1000 + '0';
			i = i - i / 1000 * 1000;
		case 3:
			*ptr++ = i/100 + '0';
			i = i - i / 100 * 100;
		case 2:
			*ptr++ = i / 10 + '0';
		case 1:	
			*ptr++ = i % 10 + '0';
	}
	return(ptr);
}

static char *
rm[12] = {
	"I","II","III",
	"IV","V","VI",
	"VII","VIII","IX",
	"X","XI","XII" };

char *
roman(m,ptr) 
	char *ptr;
{
	register len = strlen(rm[m]);
	bcopy(rm[m],ptr,len);
	ptr += len;
	return ptr;
}
/*
 * Print the date using format strings
 * This routine uses a format similar to printf, however there is
 * no attempt at compatibility.
 *
 * Note: This routine exits when done.
 */
prnt_date( ap )
char *ap;
{
	char c;
	register char *cp;
	char buf[200];
	int i, h;
	int hflag = 0;

	ap++;
	cp = buf;
	while(c = *ap++) {
		if(c == '%')
		switch(*ap++) {
		case '%':
			*cp++ = '%';
			continue;
		case 'n':
			*cp++ = '\n';
			continue;
		case 't':
			*cp++ = '\t';
			continue;
		case 'm' :
			cp = MONTH;
			continue;
		case 'd':
			cp = DAY;
			continue;
		case 'y' :
			cp = YEAR;
			continue;
		case 'D':
			cp = MONTH;
			*cp++ = '/';
			cp = DAY;
			*cp++ = '/';
			cp = YEAR;
			continue;
		case 'H':
			cp = HOUR;
			continue;
		case 'M':
			cp = MINUTE;
			continue;
		case 'S':
			cp = SECOND;
			continue;
		case 'R':
			cp = ROMAN;
			continue;
		case 'T':
			cp = HOUR;
			*cp++ = ':';
			cp = MINUTE;
			*cp++ = ':';
			cp = SECOND;
			continue;
		case 'j':
			cp = JULIAN;
			continue;
		case 'w':
			cp = WEEKDAY;
			continue;
		case 'r':
			if((h = tp->tm_hour) >= 12)
				hflag++;
			if((h %= 12) == 0)
				h = 12;
			cp = MODHOUR;
			*cp++ = ':';
			cp = MINUTE;
			*cp++ = ':';
			cp = SECOND;
			*cp++ = ' ';
			if(hflag)
				*cp++ = 'P';
			else *cp++ = 'A';
			*cp++ = 'M';
			continue;
		case 'h':
			for(i=0; i<3; i++)
				*cp++ = month[tp->tm_mon][i];
			continue;
		case 'A':
			for(i=0;long_days[tp->tm_wday][i];i++)
				*cp++ = long_days[tp->tm_wday][i];
			continue;	
		case 'a':
			for(i=0; i<3; i++)
				*cp++ = days[tp->tm_wday][i];
			continue;
		case 'z':
			cp = ZONE;
			continue;
		case 's':
			*cp++ = dst_type[ tz.tz_dsttime ];
			continue;
		default:
			(void) fprintf(stderr, "date: bad format character - %c\n", *--ap);
			exit(2);
		}	/* endsw */
		*cp++ = c;
	}	/* endwh */

	*cp = '\n';
	(void) write(1,buf,(cp - &buf[0]) + 1);
	exit(0);
}
#endif UW
