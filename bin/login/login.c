/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)login.c	5.15 (Berkeley) 4/12/86";
#endif not lint

/*
 * login [ name ]
 * login -r hostname (for rlogind)
 * login -h hostname (for telnetd, etc.)
 */

#include <sys/param.h>
#ifndef VFS
#include <sys/quota.h>
#endif !VFS
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>

#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <lastlog.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <grp.h>
#ifdef UW
#include <ctype.h>
#ifndef TTY_RESTRICTED
#define TTY_RESTRICTED  0x4 /* terminal restricted list */
#endif TTY_RESTRICTED
#endif UW

#define TTYGRPNAME	"tty"		/* name of group to own ttys */
#define TTYGID(gid)	tty_gid(gid)	/* gid that owns all ttys */

#define	SCMPN(a, b)	strncmp(a, b, sizeof(a))
#define	SCPYN(a, b)	strncpy(a, b, sizeof(a))

#define NMAX	sizeof(utmp.ut_name)
#define HMAX	sizeof(utmp.ut_host)

#define	FALSE	0
#define	TRUE	-1

#ifdef VFS
#define QUOTAWARN	"/usr/ucb/quota"	/* warn user about quotas */
#endif VFS

char	nolog[] =	"/etc/nologin";
char	qlog[]  =	".hushlogin";
char	maildir[30] =	"/usr/spool/mail/";
char	lastlog[] =	"/usr/adm/lastlog";
struct	passwd nouser = {"", "nope", -1, -1, -1, "", "", "", "" };
struct	sgttyb ttyb;
struct	utmp utmp;
char	minusnam[16] = "-";
char	*envinit[] = { 0 };		/* now set by setenv calls */
/*
 * This bounds the time given to login.  We initialize it here
 * so it can be patched on machines where it's too small.
 */
int	timeout = 60;

char	term[64];

struct	passwd *pwd;
char	*strcat(), *rindex(), *index(), *malloc(), *realloc();
int	timedout();
char	*ttyname();
char	*crypt();
char	*getpass();
char	*stypeof();
extern	char **environ;
extern	int errno;

struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

struct winsize win = { 0, 0, 0, 0 };

int	rflag;

int	usererr = -1;
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	name[NMAX+1];
char	*rhost;

main(argc, argv)
	char *argv[];
{
	register char *namep;
	int pflag = 0, hflag = 0, t, f, c;
	int invalid, quietlog;
	FILE *nlfd;
	char *ttyn, *tty;
	int ldisc = 0, zero = 0, i;
	char **envnew;

	signal(SIGALRM, timedout);
	alarm(timeout);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	setpriority(PRIO_PROCESS, 0, 0);
#ifndef VFS
	quota(Q_SETUID, 0, 0, 0);
#endif !VFS
	/*
	 * -p is used by getty to tell login not to destroy the environment
	 * -r is used by rlogind to cause the autologin protocol;
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in utmp and wtmp
	 */
	while (argc > 1) {
		if (strcmp(argv[1], "-r") == 0) {
#ifdef UW
			register char *host;
			register len;
#endif UW

			if (rflag || hflag) {
				printf("Only one of -r and -h allowed\n");
				exit(1);
			}
			rflag = 1;
			usererr = doremotelogin(argv[2]);
#ifdef UW
			len = strlen(strcat(strcpy(utmp.ut_host, rusername),"@"));
            host = argv[2];
            strncpy(utmp.ut_host+len, host, sizeof(utmp.ut_host)-len);
#else
			SCPYN(utmp.ut_host, argv[2]);
#endif UW
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-h") == 0 && getuid() == 0) {
			if (rflag || hflag) {
				printf("Only one of -r and -h allowed\n");
				exit(1);
			}
			hflag = 1;
			SCPYN(utmp.ut_host, argv[2]);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-p") == 0) {
			argc--;
			argv++;
			pflag = 1;
			continue;
		}
		break;
	}
	ioctl(0, TIOCLSET, &zero);
	ioctl(0, TIOCNXCL, 0);
	ioctl(0, FIONBIO, &zero);
	ioctl(0, FIOASYNC, &zero);
	ioctl(0, TIOCGETP, &ttyb);
	/*
	 * If talking to an rlogin process,
	 * propagate the terminal type and
	 * baud rate across the network.
	 */
	if (rflag)
		doremoteterm(term, &ttyb);
#ifdef UW
    else if (ttyb.sg_ispeed == B0) {        /* fix arpanet default speed */
        ttyb.sg_ispeed = ttyb.sg_ospeed = B9600;
    }
#endif UW
	ttyb.sg_erase = CERASE;
	ttyb.sg_kill = CKILL;
	ioctl(0, TIOCSLTC, &ltc);
	ioctl(0, TIOCSETC, &tc);
	ioctl(0, TIOCSETP, &ttyb);
	for (t = getdtablesize(); t > 2; t--)
		close(t);
	ttyn = ttyname(0);
	if (ttyn == (char *)0 || *ttyn == '\0')
		ttyn = "/dev/tty??";
	tty = rindex(ttyn, '/');
	if (tty == NULL)
		tty = ttyn;
	else
		tty++;
	openlog("login", LOG_ODELAY, LOG_AUTH);
	t = 0;
	invalid = FALSE;
	do {
		ldisc = 0;
		ioctl(0, TIOCSETD, &ldisc);
		SCPYN(utmp.ut_name, "");
		/*
		 * Name specified, take it.
		 */
		if (argc > 1) {
			SCPYN(utmp.ut_name, argv[1]);
			argc = 0;
		}
		/*
		 * If remote login take given name,
		 * otherwise prompt user for something.
		 */
		if (rflag && !invalid)
#ifdef UW
		{
			/* DLC - if bad, and no password, disallow */
			if (*pwd->pw_passwd == '\0') {
				printf("Login refused\n");
				exit(1);
			}
#endif UW
			SCPYN(utmp.ut_name, lusername);
#ifdef UW
		}
#endif UW
		else
			getloginname(&utmp);
		invalid = FALSE;
		if (!strcmp(pwd->pw_shell, "/bin/csh")) {
			ldisc = NTTYDISC;
			ioctl(0, TIOCSETD, &ldisc);
		}
		/*
		 * If no remote login authentication and
		 * a password exists for this user, prompt
		 * for one and verify it.
		 */
		if (usererr == -1 && *pwd->pw_passwd != '\0') {
			char *pp;

			setpriority(PRIO_PROCESS, 0, -4);
			pp = getpass("Password:");
			namep = crypt(pp, pwd->pw_passwd);
			setpriority(PRIO_PROCESS, 0, 0);
			if (strcmp(namep, pwd->pw_passwd))
				invalid = TRUE;
		}
		/*
		 * If user not super-user, check for logins disabled.
		 */
		if (pwd->pw_uid != 0 && (nlfd = fopen(nolog, "r")) > 0) {
			while ((c = getc(nlfd)) != EOF)
				putchar(c);
			fflush(stdout);
			sleep(5);
			exit(0);
		}
		/*
		 * If valid so far and root is logging in,
		 * see if root logins on this terminal are permitted.
		 */
		if (!invalid && pwd->pw_uid == 0 && !rootterm(tty)) {
			if (utmp.ut_host[0])
				syslog(LOG_CRIT,
				    "ROOT LOGIN REFUSED ON %s FROM %.*s",
				    tty, HMAX, utmp.ut_host);
			else
				syslog(LOG_CRIT,
				    "ROOT LOGIN REFUSED ON %s", tty);
			invalid = TRUE;
		}
		if (invalid) {
			printf("Login incorrect\n");
			if (++t >= 5) {
				if (utmp.ut_host[0])
					syslog(LOG_CRIT,
					    "REPEATED LOGIN FAILURES ON %s FROM %.*s, %.*s",
					    tty, HMAX, utmp.ut_host,
					    NMAX, utmp.ut_name);
				else
					syslog(LOG_CRIT,
					    "REPEATED LOGIN FAILURES ON %s, %.*s",
						tty, NMAX, utmp.ut_name);
				ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
				close(0), close(1), close(2);
				sleep(10);
				exit(1);
			}
		}
		if (*pwd->pw_shell == '\0')
			pwd->pw_shell = "/bin/sh";
		if (chdir(pwd->pw_dir) < 0 && !invalid ) {
			if (chdir("/") < 0) {
				printf("No directory!\n");
				invalid = TRUE;
			} else {
				printf("No directory! %s\n",
				   "Logging in with home=/");
				pwd->pw_dir = "/";
			}
		}
		/*
		 * Remote login invalid must have been because
		 * of a restriction of some sort, no extra chances.
		 */
		if (!usererr && invalid)
			exit(1);
	} while (invalid);
/* committed to login turn off timeout */
	alarm(0);

#ifndef VFS
	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
		if (errno == EUSERS)
			printf("%s.\n%s.\n",
			   "Too many users logged on already",
			   "Try again later");
		else if (errno == EPROCLIM)
			printf("You have too many processes running.\n");
		else
			perror("quota (Q_SETUID)");
		sleep(5);
		exit(0);
	}
#endif !VFS
	time(&utmp.ut_time);
	t = ttyslot();
	if (t > 0 && (f = open("/etc/utmp", O_WRONLY)) >= 0) {
		lseek(f, (long)(t*sizeof(utmp)), 0);
		SCPYN(utmp.ut_line, tty);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND)) >= 0) {
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	quietlog = access(qlog, F_OK) == 0;
	if ((f = open(lastlog, O_RDWR)) >= 0) {
		struct lastlog ll;

		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
		if (read(f, (char *) &ll, sizeof ll) == sizeof ll &&
		    ll.ll_time != 0 && !quietlog) {
			printf("Last login: %.*s ",
			    24-5, (char *)ctime(&ll.ll_time));
			if (*ll.ll_host != '\0')
				printf("from %.*s\n",
				    sizeof (ll.ll_host), ll.ll_host);
			else
				printf("on %.*s\n",
				    sizeof (ll.ll_line), ll.ll_line);
		}
		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
		time(&ll.ll_time);
		SCPYN(ll.ll_line, tty);
		SCPYN(ll.ll_host, utmp.ut_host);
		write(f, (char *) &ll, sizeof ll);
		close(f);
	}
	chown(ttyn, pwd->pw_uid, TTYGID(pwd->pw_gid));
	if (!hflag && !rflag)					/* XXX */
		ioctl(0, TIOCSWINSZ, &win);
	chmod(ttyn, 0620);
	setgid(pwd->pw_gid);
	strncpy(name, utmp.ut_name, NMAX);
	name[NMAX] = '\0';
	initgroups(name, pwd->pw_gid);

#ifdef UW
    if (pwd->pw_uid && tty_denied(pwd->pw_name,tty)) {
        syslog(LOG_INFO,"denied access to %s on restricted tty %s",
            pwd->pw_name,tty);
        printf("Sorry, %s, but you may not login on %s\n",pwd->pw_name,tty);
        sleep(5);
        exit(0);
    }
#endif UW

#ifndef VFS
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
#endif !VFS
	setuid(pwd->pw_uid);
	/* destroy environment unless user has asked to preserve it */
	if (!pflag)
		environ = envinit;

	/* set up environment, this time without destruction */
	/* copy the environment before setenving */
	i = 0;
	while (environ[i] != NULL)
		i++;
	envnew = (char **) malloc(sizeof (char *) * (i + 1));
	for (; i >= 0; i--)
		envnew[i] = environ[i];
	environ = envnew;

	setenv("HOME=", pwd->pw_dir, 1);
	setenv("SHELL=", pwd->pw_shell, 1);
	if (term[0] == '\0')
		strncpy(term, stypeof(tty), sizeof(term));
	setenv("TERM=", term, 0);
	setenv("USER=", pwd->pw_name, 1);
	setenv("PATH=", ":/usr/ucb:/bin:/usr/bin", 0);

	if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;
	strcat(minusnam, namep);
	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
	if (pwd->pw_uid == 0)
		if (utmp.ut_host[0])
			syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %.*s",
			    tty, HMAX, utmp.ut_host);
		else
			syslog(LOG_NOTICE, "ROOT LOGIN %s", tty);
	if (!quietlog) {
		struct stat st;

		showmotd();
		strcat(maildir, pwd->pw_name);
		if (stat(maildir, &st) == 0 && st.st_size != 0)
			printf("You have %smail.\n",
				(st.st_mtime > st.st_atime) ? "new " : "");
	}
#ifdef VFS
#ifdef UW
	if (pwd->pw_uid != 0)
#endif UW
		system(QUOTAWARN);
#endif VFS
	signal(SIGALRM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_IGN);
	execlp(pwd->pw_shell, minusnam, 0);
	perror(pwd->pw_shell);
	printf("No shell\n");
#ifdef UW
    if(pwd->pw_uid == 0)
    {
        char newshell[256];
        printf("And what would you like for your shell? ");
        scanf("%s",newshell);
        execlp(newshell,minusnam,0);
        printf("Hmm.  That didn't work either.\n");
    }
#endif UW
	exit(0);
}

getloginname(up)
	register struct utmp *up;
{
	register char *namep;
	char c;

	while (up->ut_name[0] == '\0') {
		namep = up->ut_name;
		printf("login: ");
		while ((c = getchar()) != '\n') {
			if (c == ' ')
				c = '_';
			if (c == EOF)
				exit(0);
			if (namep < up->ut_name+NMAX)
				*namep++ = c;
		}
	}
	strncpy(lusername, up->ut_name, NMAX);
	lusername[NMAX] = 0;
	if ((pwd = getpwnam(lusername)) == NULL)
		pwd = &nouser;
}

timedout()
{

	printf("Login timed out after %d seconds\n", timeout);
	exit(0);
}

int	stopmotd;
catch()
{

	signal(SIGINT, SIG_IGN);
	stopmotd++;
}

rootterm(tty)
	char *tty;
{
	register struct ttyent *t;

	if ((t = getttynam(tty)) != NULL) {
		if (t->ty_status & TTY_SECURE)
			return (1);
	}
	return (0);
}

showmotd()
{
	FILE *mf;
	register c;

	signal(SIGINT, catch);
	if ((mf = fopen("/etc/motd", "r")) != NULL) {
		while ((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
	}
	signal(SIGINT, SIG_IGN);
}

#undef	UNKNOWN
#define UNKNOWN "su"

char *
stypeof(ttyid)
	char *ttyid;
{
	register struct ttyent *t;

	if (ttyid == NULL || (t = getttynam(ttyid)) == NULL)
		return (UNKNOWN);
	return (t->ty_type);
}

doremotelogin(host)
	char *host;
{
	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		return(-1);
	}
	return(ruserok(host, (pwd->pw_uid == 0), rusername, lusername));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term, tp)
	char *term;
	struct sgttyb *tp;
{
	register char *cp = index(term, '/'), **cpp;
	char *speed;

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
				tp->sg_ispeed = tp->sg_ospeed = cpp-speeds;
				break;
			}
	}
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}

/*
 * Set the value of var to be arg in the Unix 4.2 BSD environment env.
 * Var should end with '='.
 * (bindings are of the form "var=value")
 * This procedure assumes the memory for the first level of environ
 * was allocated using malloc.
 */
setenv(var, value, clobber)
	char *var, *value;
{
	extern char **environ;
	int index = 0;
	int varlen = strlen(var);
	int vallen = strlen(value);

	for (index = 0; environ[index] != NULL; index++) {
		if (strncmp(environ[index], var, varlen) == 0) {
			/* found it */
			if (!clobber)
				return;
			environ[index] = malloc(varlen + vallen + 1);
			strcpy(environ[index], var);
			strcat(environ[index], value);
			return;
		}
	}
	environ = (char **) realloc(environ, sizeof (char *) * (index + 2));
	if (environ == NULL) {
		fprintf(stderr, "login: malloc out of memory\n");
		exit(1);
	}
	environ[index] = malloc(varlen + vallen + 1);
	strcpy(environ[index], var);
	strcat(environ[index], value);
	environ[++index] = NULL;
}

#ifdef UW
/*
 *	written by tom christiansen
 *
 *  called by login if the restricted bit is set for your tty,
 *  tty_denied() gets passed your ttyname, your user, and the head
 *  of your group list.  consulting the file "/etc/ttys.perms",
 *  it determines whether you have permission to use this line.
 *
 *  the format the the config file is:
 *  	ttyname LIST
 *  where LIST is composed of comma-separated entries for 
 *  either group or user names.  a group name is indicated
 *  by placing the name in <brackets> like this.  additionally,
 *  any entry may be negated by preceding it with a !.  thus
 *  a sample entry might be:
 *  
 *  	ttyh2	uucp,<staff>,!nuke
 *
 *  if there are any assertions, then to login you must be on 
 *  the assertion list and not on the denial list.  if there are
 *  only denials, then you may log in so long as you aren't on the
 *  denial list.
 *  
 *  returns 
 *  	1		- for a "tty_denied", ie: you can't login here
 *  	0		- you're ok.
 */

#define 	OK			0
#define		DENIED 		1
#define		USIZE		8
#define		UMAX		(USIZE+1)
#define		BLANK(x)	( (x) == ',' || (x)==' ' || (x)=='\t' || (x)=='\n')

#ifdef DEBUG
static void syslog();
#endif DEBUG

/* set tabstops=4 */

int 
tty_denied(user,tty)
	char tty[];
	char user[];
{
#ifdef DEBUG
	static		 	char	 PERMS [] = "./ttys.perms";
#else
	static		 	char	 PERMS [] = "/etc/ttys.perms";
#endif DEBUG
	static  		char	 		line [ BUFSIZ ];
	static			char	 		gnames [ NGROUPS ] [ UMAX ];
	static			int		 		groups [ NGROUPS ];
	static			char	 		ent [ UMAX ];
	static			char	 		tname [ 20 ];

	auto			FILE		   *fp;		
	auto			int		 		ngroups;
	auto			int		 		negated = 0;
	auto			int		 		assertions = 0;
	auto			struct ttyent  *tent;

	register		int				 i;
	register		struct group 	*g;
	register		char			*p, *q, *r;


	/*
	 *  open permissions file
	 */
	if ( (fp=fopen(PERMS,"r")) == NULL ||
		 (tent=getttynam(tty)) == NULL ||
		 (tent->ty_status & TTY_RESTRICTED) == 0) {
		return OK;
	}

	/*
	 *  get this users groups
	 */
	if ((ngroups=getgroups(NGROUPS,groups)) == -1) {
		syslog(LOG_WARNING,"Couldn't get groups for %s",user);
		return OK;
	}

	/*
	 *  fill in the names for the groups.
	 */
	if (ngroups) {
		setgrent();
		for (i=0; i < ngroups; i++ ) {
			if (!(g=getgrgid(groups[i]))) {
				syslog(LOG_WARNING,"Couldn't find name of group %d",groups[i]);
				continue;
			}
			strncpy(gnames[i],g->gr_name, sizeof(gnames[i]));
		}
		endgrent();
	}

	while (fgets(line,sizeof(line),fp)) {
		if ( (*line == '#')	 ||
			 (!sscanf(line,"%s",tname)) ||
			 (strcmp(tname,tty)))
			continue;

		bzero(ent,sizeof(ent));

		for ( p = line + strlen(tname) ; *p; ) {
			while (*p && BLANK(*p))
				p++;

			for (q=ent; *p && !BLANK(*p) && q < ent+UMAX; *q++ = *p++)
				 ;

			*q = 0;

			if (*ent == '\0' || *ent == '#')
				break;

			while (p != (line + sizeof(line) -1) && *p && !BLANK(*p))
				p++;

			if ( *(q=ent) == '!' ) {
				q++;
				negated = 1;
			} else {
				negated = 0;
				assertions = 1;
			}

			if (*q == '<') {
				q++;
				if ( *(r=q+strlen(q)-1) == '>')
					*r = 0;
				for (i=0; i < ngroups; i++ ) 
					if (!strcmp(q,gnames[i])) {
						(void) fclose(fp);
						return negated
							?	DENIED
							:	OK;
					}
			} else if (!strcmp(q,user)) {
					(void) fclose(fp);
					return negated
						?	DENIED
						:	OK;
				}

		}
		break;
	}


/* got this far so it must be ok */

	(void) fclose(fp);
	return assertions
		?	DENIED
		:	OK;
}

#ifdef DEBUG
static void
syslog(pri,fmt,args)
	char *fmt;
{
	fprintf(stderr,"SYSLOG (pri = %d) ",pri);
	_doprnt(fmt,&args,stderr);
	fprintf(stderr,"\n");
}
#endif DEBUG

#endif UW

tty_gid(default_gid)
	int default_gid;
{
	struct group *getgrnam(), *gr;
	int gid = default_gid;

	gr = getgrnam(TTYGRPNAME);
	if (gr != (struct group *) 0)
		gid = gr->gr_gid;

	endgrent();

	return (gid);
}
