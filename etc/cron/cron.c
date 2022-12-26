#ifndef lint
static char *sccsid = "@(#)cron.c	4.12 (Berkeley) 5/27/86";
#endif

#ifdef UW
/*
 * Modified Sun Apr 28 16:14:01 CDT 1985 by Dave Cohrs
 *  allow escaping of '%' char with a backslash like sysV cron.
 * Above modification added to 4.3 src Thu Oct 24 02:10:17 CDT 1985
 *  by David Parter.
 * Modified Thu Dec 19 09:43:28 CST 1985 by Dave Cohrs
 *  to allow only root to start cron.
 * Modified Mon Mar 10 03:02:55 CST 1986 by David Parter
 *  to log errors to syslog.
 * Modified Fri Oct  3 13:43:47 CDT 1986 by Dave Cohrs
 *  to allow commenting lines with a '#' character
 */
#endif UW

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <pwd.h>
#ifdef UW
#include <syslog.h>
#endif UW
#include <fcntl.h>

#define	LISTS	(2*BUFSIZ)
#define	MAXLIN	BUFSIZ

#ifndef CRONTAB
#define CRONTAB "/usr/lib/crontab"
#endif

#ifndef CRONTABLOC
#define CRONTABLOC  "/usr/lib/crontab.local"
#endif

#define	EXACT	100
#define	ANY	101
#define	LIST	102
#define	RANGE	103
#define	EOS	104

char	crontab[]	= CRONTAB;
char	loc_crontab[]   = CRONTABLOC;
time_t	itime;
struct	tm *loct;
struct	tm *localtime();
char	*malloc();
char	*realloc();
int	reapchild();
int	flag;
char	*list;
char	*listend;
unsigned listsize;

FILE	*debug;
#define dprintf if (debug) fprintf

main(argc, argv)
	int argc;
	char **argv;
{
	register char *cp;
	char *cmp();
	time_t filetime = 0;
	time_t lfiletime = 0;
	char c;
	extern char *optarg;

#ifdef UW
    openlog("cron", 0, LOG_DAEMON);
    if (getuid() != 0) {
        syslog(LOG_ERR, "attempted start by uid %d", getuid());
        fprintf(stderr,"cron: attempted start by non-root, refused\n");
        exit(1);
	}
	syslog(LOG_INFO, "start");
#endif UW

	if (fork())
		exit(0);
	c = getopt(argc, argv, "d:");
	if (c == 'd') {
		debug = fopen(optarg, "w");
		if (debug == NULL)
			exit(1);
		fcntl(fileno(debug), F_SETFL, FAPPEND);
	}
	chdir("/");
	freopen("/", "r", stdout);
	freopen("/", "r", stderr);
	untty();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGCHLD, reapchild);
	time(&itime);
	itime -= localtime(&itime)->tm_sec;

	for (;; itime+=60, slp()) {
		struct stat cstat, lcstat;
		int newcron, newloc;
		
		newcron = 0;
		if (stat(crontab, &cstat) < 0)
		    cstat.st_mtime = 1;
		if (cstat.st_mtime != filetime) {
			filetime = cstat.st_mtime;
			newcron++;
		}

		newloc  = 0;
		if (stat(loc_crontab, &lcstat) < 0)
		    lcstat.st_mtime = 1;
		if (lcstat.st_mtime != lfiletime) {
			lfiletime = lcstat.st_mtime;
			newloc++;
		}

		if (newcron || newloc) {
			init();
			append(crontab);
			append(loc_crontab);
			*listend++ = EOS;
			*listend++ = EOS;
		}

		loct = localtime(&itime);
		loct->tm_mon++;		 /* 1-12 for month */
		if (loct->tm_wday == 0)
			loct->tm_wday = 7;	/* sunday is 7, not 0 */
		for(cp = list; *cp != EOS;) {
			flag = 0;
			cp = cmp(cp, loct->tm_min);
			cp = cmp(cp, loct->tm_hour);
			cp = cmp(cp, loct->tm_mday);
			cp = cmp(cp, loct->tm_mon);
			cp = cmp(cp, loct->tm_wday);
			if(flag == 0)
				ex(cp);
			while(*cp++ != 0)
				;
		}
	}
}

char *
cmp(p, v)
char *p;
{
	register char *cp;

	cp = p;
	switch(*cp++) {

	case EXACT:
		if (*cp++ != v)
			flag++;
		return(cp);

	case ANY:
		return(cp);

	case LIST:
		while(*cp != LIST)
			if(*cp++ == v) {
				while(*cp++ != LIST)
					;
				return(cp);
			}
		flag++;
		return(cp+1);

	case RANGE:
		if(*cp > v || cp[1] < v)
			flag++;
		return(cp+2);
	}
	if(cp[-1] != v)
		flag++;
	return(cp);
}

slp()
{
	register i;
	time_t t;

	time(&t);
	i = itime - t;
	if(i < -60 * 60 || i > 60 * 60) {
		itime = t;
		i = 60 - localtime(&itime)->tm_sec;
		itime += i;
	}
	if(i > 0)
		sleep(i);
}

ex(s)
char *s;
{
	int st;
	register struct passwd *pwd;
	char user[BUFSIZ];
	char *c = user;
	int pid;

	if (fork()) {
		return;
	}

	pid = getpid();
	while(*s != ' ' && *s != '\t')
		*c++ = *s++;
	*c = '\0';
	s++;
	if ((pwd = getpwnam(user)) == NULL) {
#ifdef UW
		syslog(LOG_ERR, "unknown user: %s (%s)", user, ++s);
#endif UW
		dprintf(debug, "%d: cannot find %s\n", pid, user),
			fflush(debug);
		exit(1);
	}
	(void) setgid(pwd->pw_gid);
	initgroups(pwd->pw_name, pwd->pw_gid);
	(void) setuid(pwd->pw_uid);
	freopen("/", "r", stdin);
	dprintf(debug, "%d: executing %s", pid, s), fflush (debug);
	execl("/bin/sh", "sh", "-c", s, 0);
#ifdef UW
    syslog(LOG_ERR, "command failed: %s [%m]", s);
#endif UW
	dprintf(debug, "%d: cannot execute sh\n", pid), fflush (debug);
	exit(0);
}

init()
{
	/*
	 * Don't free in case was longer than LISTS.  Trades off
	 * the rare case of crontab shrinking vs. the common case of
	 * extra realloc's needed in append() for a large crontab.
	 */
	if (list == 0) {
		list = malloc(LISTS);
		listsize = LISTS;
	}
	listend = list;
}

append(fn)
char *fn;
{
	register i, c;
	register char *cp;
	register char *ocp;
	register int n;

	if (freopen(fn, "r", stdin) == NULL)
		return;
	cp = listend;
loop:
	if(cp > list+listsize-MAXLIN) {
		int length = cp - list;

		listsize += LISTS;
		list = realloc(list, listsize);
		cp = list + length;
	}
	ocp = cp;
	for(i=0;; i++) {
		do
			c = getchar();
		while(c == ' ' || c == '\t')
			;
#ifdef UW
		if(i == 0 && c == '#')	/* ignore commented lines */
			goto ignore;
#endif UW
		if(c == EOF || c == '\n')
			goto ignore;
		if(i == 5)
			break;
		if(c == '*') {
			*cp++ = ANY;
			continue;
		}
		if ((n = number(c)) < 0)
			goto ignore;
		c = getchar();
		if(c == ',')
			goto mlist;
		if(c == '-')
			goto mrange;
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = EXACT;
		*cp++ = n;
		continue;

	mlist:
		*cp++ = LIST;
		*cp++ = n;
		do {
			if ((n = number(getchar())) < 0)
				goto ignore;
			*cp++ = n;
			c = getchar();
		} while (c==',');
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = LIST;
		continue;

	mrange:
		*cp++ = RANGE;
		*cp++ = n;
		if ((n = number(getchar())) < 0)
			goto ignore;
		c = getchar();
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = n;
	}
	while(c != '\n') {
		if(c == EOF)
			goto ignore;
#ifdef UW
        if(c == '\\') {         /* allow escaping of the '%' character */
            c = getchar();
            if (c != '%')       /* if not a '%', write out bkslash too */
                *cp++ = '\\';
            if (c != '\n') {    /* don't write out newline in any case */
                *cp++ = c;
				c = getchar();
            }
			continue; 	/* move on in any case */
		}
#endif UW
		if(c == '%')
			c = '\n';
		*cp++ = c;
		c = getchar();
	}
	*cp++ = '\n';
	*cp++ = 0;
	goto loop;

ignore:
	cp = ocp;
	while(c != '\n') {
		if(c == EOF) {
			fclose(stdin);
			listend = cp;
			return;
		}
		c = getchar();
	}
	goto loop;
}

number(c)
register c;
{
	register n = 0;

	while (isdigit(c)) {
		n = n*10 + c - '0';
		c = getchar();
	}
	ungetc(c, stdin);
	if (n>=100)
		return(-1);
	return(n);
}

reapchild()
{
	union wait status;
	int pid;

	while ((pid = wait3(&status, WNOHANG, 0)) > 0)
		dprintf(debug, "%d: child exits with signal %d status %d\n",
			pid, status.w_termsig, status.w_retcode),
			fflush (debug);
}

untty()
{
	int i;

	i = open("/dev/tty", O_RDWR);
	if (i >= 0) {
		ioctl(i, TIOCNOTTY, (char *)0);
		(void) close(i);
	}
}
