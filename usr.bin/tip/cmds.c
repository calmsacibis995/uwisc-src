/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)cmds.c	5.1 (Berkeley) 4/30/85";
#endif not lint

#include "tip.h"
#include <sys/time.h>
/*
 * tip
 *
 * miscellaneous commands
 */

int	quant[] = { 60, 60, 24 };

char	null = '\0';
char	*sep[] = { "second", "minute", "hour" };
static char *argv[10];		/* argument vector for take and put */

int	timeout();		/* timeout function called on alarm */
int	stopsnd();		/* SIGINT handler during file transfers */
int	intprompt();		/* used in handling SIG_INT during prompt */
int	intcopy();		/* interrupt routine for file transfers */

/*
 * FTP - remote ==> local
 *  get a file from the remote host
 */
getfl(c)
	char c;
{
	char buf[256], *cp, *expand();
	
	putchar(c);
	/*
	 * get the UNIX receiving file's name
	 */
	if (prompt("Local file name? ", copyname))
		return;
	cp = expand(copyname);
	if ((sfd = creat(cp, 0666)) < 0) {
		printf("\r\n%s: cannot creat\r\n", copyname);
		return;
	}
	
	/*
	 * collect parameters
	 */
	if (prompt("List command for remote system? ", buf)) {
		unlink(copyname);
		return;
	}
	transfer(buf, sfd, value(EOFREAD));
}

/*
 * Cu-like take command
 */
cu_take(cc)
	char cc;
{
	int fd, argc;
	char line[BUFSIZ], *expand(), *cp;

	if (prompt("[take] ", copyname))
		return;
	if ((argc = args(copyname, argv)) < 1 || argc > 2) {
		printf("usage: <take> from [to]\r\n");
		return;
	}
	if (argc == 1)
		argv[1] = argv[0];
	cp = expand(argv[1]);
	if ((fd = creat(cp, 0666)) < 0) {
		printf("\r\n%s: cannot create\r\n", argv[1]);
		return;
	}
	sprintf(line, "cat %s;echo \01", argv[0]);
	transfer(line, fd, "\01");
}

static	jmp_buf intbuf;
/*
 * Bulk transfer routine --
 *  used by getfl(), cu_take(), and pipefile()
 */
transfer(buf, fd, eofchars)
	char *buf, *eofchars;
{
	register int ct;
	char c, buffer[BUFSIZ];
	register char *p = buffer;
	register int cnt, eof;
	time_t start;
	int (*f)();
	int toscreen = boolean(value(TOSCREEN));

	pwrite(FD, buf, size(buf));
	quit = 0;
	kill(pid, SIGIOT);
	read(repdes[0], (char *)&ccc, 1);  /* Wait until read process stops */
	
	/*
	 * finish command
	 */
	pwrite(FD, "\r", 1);
	do
		read(FD, &c, 1); 
	while ((c&0177) != '\n');
	ioctl(0, TIOCSETC, &defchars);
	
	(void) setjmp(intbuf);
	f = signal(SIGINT, intcopy);
	start = time(0);
	for (ct = 0; !quit;) {
		eof = read(FD, &c, 1) <= 0;
		c &= 0177;
		if (quit)
			continue;
		if (eof || any(c, eofchars))
			break;
		if (c == 0)
			continue;	/* ignore nulls */
		if (c == '\r')
			continue;
		*p++ = c;
		if (toscreen)
			putchar(c);

		if (c == '\n' && boolean(value(VERBOSE))) {
			if (toscreen) {
				putchar('\r'); ct++;
				fflush(stdout);
			} else
				printf("\r%d", ++ct);
		}
		if ((cnt = (p-buffer)) == number(value(FRAMESIZE))) {
			if (write(fd, buffer, cnt) != cnt) {
				printf("\r\nwrite error\r\n");
				quit = 1;
			}
			p = buffer;
		}
	}
	if (cnt = (p-buffer)) {
		if (write(fd, buffer, cnt) != cnt)
			printf("\r\nwrite error\r\n");
	}

	if (boolean(value(VERBOSE))) {
		if (toscreen)
			printf("\r%d", ct);
		prtime(" lines transferred in ", (long)time(0)-start);
	}
	ioctl(0, TIOCSETC, &tchars);
	write(fildes[1], (char *)&ccc, 1);
	signal(SIGINT, f);
	close(fd);
}

/*
 * FTP - remote ==> local process
 *   send remote input to local process via pipe
 */
pipefile()
{
	int cpid, pdes[2];
	char buf[256];
	int status, p;
	extern int errno;

	if (prompt("Local command? ", buf))
		return;

	if (pipe(pdes)) {
		printf("can't establish pipe\r\n");
		return;
	}

	if ((cpid = fork()) < 0) {
		printf("can't fork!\r\n");
		return;
	} else if (cpid) {
		if (prompt("List command for remote system? ", buf)) {
			close(pdes[0]), close(pdes[1]);
			kill (cpid, SIGKILL);
		} else {
			close(pdes[0]);
			signal(SIGPIPE, intcopy);
			transfer(buf, pdes[1], value(EOFREAD));
			signal(SIGPIPE, SIG_DFL);
			while ((p = wait(&status)) > 0 && p != cpid)
				;
		}
	} else {
		register int f;

		dup2(pdes[0], 0);
		close(pdes[0]);
		for (f = 3; f < 20; f++)
			close(f);
		execute(buf);
		printf("can't execl!\r\n");
		exit(0);
	}
}

/*
 * Interrupt service routine for FTP
 */
stopsnd()
{

	stop = 1;
	signal(SIGINT, SIG_IGN);
}

/*
 * FTP - local ==> remote
 *  send local file to remote host
 *  terminate transmission with pseudo EOF sequence
 */
sendfile(cc)
	char cc;
{
	FILE *fd;
	char *fnamex;
	char *expand();

	putchar(cc);
	/*
	 * get file name
	 */
	if (prompt("Local file name? ", fname))
		return;

	/*
	 * look up file
	 */
	fnamex = expand(fname);
	if ((fd = fopen(fnamex, "r")) == NULL) {
		printf("%s: cannot open\r\n", fname);
		return;
	}
	transmit(fd, value(EOFWRITE), NULL);
	if (!boolean(value(ECHOCHECK))) {
		struct sgttyb buf;

		ioctl(FD, TIOCGETP, &buf);	/* this does a */
		ioctl(FD, TIOCSETP, &buf);	/*   wflushtty */
	}
}

/*
 * Bulk transfer routine to remote host --
 *   used by sendfile() and cu_put()
 */
transmit(fd, eofchars, command)
	FILE *fd;
	char *eofchars, *command;
{
	char *pc, lastc;
	int c, ccount, lcount;
	time_t start_t, stop_t;
	int toscreen = boolean(value(TOSCREEN));
	int (*f)();

	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	stop = 0;
	f = signal(SIGINT, stopsnd);
	ioctl(0, TIOCSETC, &defchars);
	read(repdes[0], (char *)&ccc, 1);
	if (command != NULL) {
		for (pc = command; *pc; pc++)
			send(*pc);
		if (boolean(value(ECHOCHECK)))
			read(FD, (char *)&c, 1);	/* trailing \n */
		else {
			struct sgttyb buf;

			ioctl(FD, TIOCGETP, &buf);	/* this does a */
			ioctl(FD, TIOCSETP, &buf);	/*   wflushtty */
			sleep(5); /* wait for remote stty to take effect */
		}
	}
	lcount = 0;
	lastc = '\0';
	start_t = time(0);
	while (1) {
		ccount = 0;
		do {
			c = getc(fd);
			if (stop)
				goto out;
			if (c == EOF)
				goto out;
			if (c == 0177 && !boolean(value(RAWFTP)))
				continue;
			lastc = c;
			if (c < 040) {
				if (c == '\n') {
					if (!boolean(value(RAWFTP)))
						c = '\r';
				}
				else if (c == '\t') {
					if (!boolean(value(RAWFTP))) {
						if (boolean(value(TABEXPAND))) {
							send(' ');
							while ((++ccount % 8) != 0)
								send(' ');
							continue;
						}
					}
				} else
					if (!boolean(value(RAWFTP)))
						continue;
			}
			send(c);
			putchar(c);
		} while (c != '\r' && !boolean(value(RAWFTP)));
		if (boolean(value(VERBOSE))) {
			if (toscreen) {
				putchar('\r'); lcount++;
				fflush(stdout);
			} else
				printf("\r%d", ++lcount);
		}
		if (boolean(value(ECHOCHECK))) {
			timedout = 0;
			alarm(value(ETIMEOUT));
			do {	/* wait for prompt */
				read(FD, (char *)&c, 1);
				if (timedout || stop) {
					if (timedout)
						printf("\r\ntimed out at eol\r\n");
					alarm(0);
					goto out;
				}
			} while ((c&0177) != character(value(PROMPT)));
			alarm(0);
		}
	}
out:
	if (lastc != '\n' && !boolean(value(RAWFTP)))
		send('\r');
	for (pc = eofchars; *pc; pc++)
		send(*pc);
	stop_t = time(0);
	fclose(fd);
	signal(SIGINT, f);
	if (boolean(value(VERBOSE)))
		if (boolean(value(RAWFTP)))
			prtime(" chars transferred in ", stop_t-start_t);
		else {
			if (toscreen)
				printf("\r%d", lcount);
			prtime(" lines transferred in ", stop_t-start_t);
		}
	write(fildes[1], (char *)&ccc, 1);
	ioctl(0, TIOCSETC, &tchars);
}

/*
 * Cu-like put command
 */
cu_put(cc)
	char cc;
{
	FILE *fd;
	char line[BUFSIZ];
	int argc;
	char *expand();
	char *copynamex;

	if (prompt("[put] ", copyname))
		return;
	if ((argc = args(copyname, argv)) < 1 || argc > 2) {
		printf("usage: <put> from [to]\r\n");
		return;
	}
	if (argc == 1)
		argv[1] = argv[0];
	copynamex = expand(argv[0]);
	if ((fd = fopen(copynamex, "r")) == NULL) {
		printf("%s: cannot open\r\n", copynamex);
		return;
	}
	if (boolean(value(ECHOCHECK)))
		sprintf(line, "cat>%s\r", argv[1]);
	else
		sprintf(line, "stty -echo;cat>%s;stty echo\r", argv[1]);
	transmit(fd, "\04", line);
}

/*
 * FTP - send single character
 *  wait for echo & handle timeout
 */
send(c)
	char c;
{
	char cc;
	int retry = 0;
	struct timeval tv;
	int nn;

	cc = c;
	pwrite(FD, &cc, 1);
	if ((nn=number(value(CDELAY))) > 0 && c != '\r') {
		tv.tv_sec = nn / 1000;
		tv.tv_usec = nn % 1000;
		(void) select(0, (int *) 0, (int *) 0, (int *) 0, &tv);
	}
	if (!boolean(value(ECHOCHECK))) {
		if ((nn=number(value(LDELAY))) > 0 && c == '\r') {
			tv.tv_sec = nn / 1000;
			tv.tv_usec = nn % 1000;
			(void) select(0, (int *) 0, (int *) 0, (int *) 0, &tv);
		}
		return;
	}
tryagain:
	timedout = 0;
	alarm(value(ETIMEOUT));
	read(FD, &cc, 1);
	alarm(0);
	if (timedout) {
		printf("\r\ntimeout error (%s)\r\n", ctrl(c));
		if (retry++ > 3)
			return;
		pwrite(FD, &null, 1); /* poke it */
		goto tryagain;
	}
}

timeout()
{
	signal(SIGALRM, timeout);
	timedout = 1;
}

#ifdef CONNECT
/*
 * Fork a program with:
 *  0 <-> local tty in
 *  1 <-> local tty out
 *  2 <-> local tty out
 *  3 <-> remote tty in
 *  4 <-> remote tty out
 */
consh(c)
{
	char buf[256];
	int cpid, status, p;
	time_t start;

	putchar(c);
	if (prompt("Local command? ", buf))
		return;
	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	ioctl(0, TIOCSETC, &defchars);
	read(repdes[0], (char *)&ccc, 1);
	/*
	 * Set up file descriptors in the child and
	 *  let it go...
	 */
	if ((cpid = fork()) < 0)
		printf("can't fork!\r\n");
	else if (cpid) {
		start = time(0);
		while ((p = wait(&status)) > 0 && p != cpid)
			;
	} else {
		register int i;

		dup2(FD, 3);
		dup2(3, 4);
		for (i = 5; i < 20; i++)
			close(i);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execute(buf);
		printf("can't find `%s'\r\n", buf);
		exit(0);
	}
	if (boolean(value(VERBOSE)))
		prtime("away for ", time(0)-start);
	write(fildes[1], (char *)&ccc, 1);
	ioctl(0, TIOCSETC, &tchars);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}
#endif

/*
 * Escape to local shell
 */
shell()
{
	int shpid, status;
	extern char **environ;
	char *cp;

	printf("[sh]\r\n");
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	unraw();
	if (shpid = fork()) {
		while (shpid != wait(&status));
		raw();
		printf("\r\n!\r\n");
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		return;
	} else {
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		if ((cp = rindex(value(SHELL), '/')) == NULL)
			cp = value(SHELL);
		else
			cp++;
		execl(value(SHELL), cp, 0);
		printf("\r\ncan't execl!\r\n");
		exit(1);
	}
}

/*
 * TIPIN portion of scripting
 *   initiate the conversation with TIPOUT
 */
setscript()
{
	char c;
	/*
	 * enable TIPOUT side for dialogue
	 */
	kill(pid, SIGEMT);
	if (boolean(value(SCRIPT)))
		write(fildes[1], value(RECORD), size(value(RECORD)));
	write(fildes[1], "\n", 1);
	/*
	 * wait for TIPOUT to finish
	 */
	read(repdes[0], &c, 1);
	if (c == 'n')
		printf("can't create %s\r\n", value(RECORD));
}

/*
 * Change current working directory of
 *   local portion of tip
 */
chdirectory()
{
	char dirname[80];
	register char *cp = dirname;

	if (prompt("[cd] ", dirname)) {
		if (stoprompt)
			return;
		cp = value(HOME);
	}
	if (chdir(cp) < 0)
		printf("%s: bad directory\r\n", cp);
	printf("!\r\n");
}

abort(msg)
	char *msg;
{

	kill(pid, SIGTERM);
	disconnect(msg);
	if (msg != NOSTR)
		printf("\r\n%s", msg);
	printf("\r\n[EOT]\r\n");
	delock(uucplock);
	unraw();
	exit(0);
}

finish()
{
	char *dismsg;

	if ((dismsg = value(DISCONNECT)) != NOSTR) {
		write(FD, dismsg, strlen(dismsg));
		sleep(5);
	}
	abort(NOSTR);
}

intcopy()
{

	raw();
	quit = 1;
	longjmp(intbuf, 1);
}

execute(s)
	char *s;
{
	register char *cp;

	if ((cp = rindex(value(SHELL), '/')) == NULL)
		cp = value(SHELL);
	else
		cp++;
	execl(value(SHELL), cp, "-c", s, 0);
}

args(buf, a)
	char *buf, *a[];
{
	register char *p = buf, *start;
	register char **parg = a;
	register int n = 0;

	do {
		while (*p && (*p == ' ' || *p == '\t'))
			p++;
		start = p;
		if (*p)
			*parg = p;
		while (*p && (*p != ' ' && *p != '\t'))
			p++;
		if (p != start)
			parg++, n++;
		if (*p)
			*p++ = '\0';
	} while (*p);

	return(n);
}

prtime(s, a)
	char *s;
	time_t a;
{
	register i;
	int nums[3];

	for (i = 0; i < 3; i++) {
		nums[i] = (int)(a % quant[i]);
		a /= quant[i];
	}
	printf("%s", s);
	while (--i >= 0)
		if (nums[i] || i == 0 && nums[1] == 0 && nums[2] == 0)
			printf("%d %s%c ", nums[i], sep[i],
				nums[i] == 1 ? '\0' : 's');
	printf("\r\n!\r\n");
}

variable()
{
	char	buf[256];

	if (prompt("[set] ", buf))
		return;
	vlex(buf);
	if (vtable[BEAUTIFY].v_access&CHANGED) {
		vtable[BEAUTIFY].v_access &= ~CHANGED;
		kill(pid, SIGSYS);
	}
	if (vtable[SCRIPT].v_access&CHANGED) {
		vtable[SCRIPT].v_access &= ~CHANGED;
		setscript();
		/*
		 * So that "set record=blah script" doesn't
		 *  cause two transactions to occur.
		 */
		if (vtable[RECORD].v_access&CHANGED)
			vtable[RECORD].v_access &= ~CHANGED;
	}
	if (vtable[RECORD].v_access&CHANGED) {
		vtable[RECORD].v_access &= ~CHANGED;
		if (boolean(value(SCRIPT)))
			setscript();
	}
	if (vtable[TAND].v_access&CHANGED) {
		vtable[TAND].v_access &= ~CHANGED;
		if (boolean(value(TAND)))
			tandem("on");
		else
			tandem("off");
	}
 	if (vtable[LECHO].v_access&CHANGED) {
 		vtable[LECHO].v_access &= ~CHANGED;
 		HD = boolean(value(LECHO));
 	}
	if (vtable[PARITY].v_access&CHANGED) {
		vtable[PARITY].v_access &= ~CHANGED;
		setparity();
	}
}

/*
 * Turn tandem mode on or off for remote tty.
 */
tandem(option)
	char *option;
{
	struct sgttyb rmtty;

	ioctl(FD, TIOCGETP, &rmtty);
	if (strcmp(option,"on") == 0) {
		rmtty.sg_flags |= TANDEM;
		arg.sg_flags |= TANDEM;
	} else {
		rmtty.sg_flags &= ~TANDEM;
		arg.sg_flags &= ~TANDEM;
	}
	ioctl(FD, TIOCSETP, &rmtty);
	ioctl(0,  TIOCSETP, &arg);
}

/*
 * Send a break.
 */
genbrk()
{

	ioctl(FD, TIOCSBRK, NULL);
	sleep(1);
	ioctl(FD, TIOCCBRK, NULL);
}

/*
 * Suspend tip
 */
suspend()
{

	unraw();
	kill(0, SIGTSTP);
	raw();
}

/*
 *	expand a file name if it includes shell meta characters
 */

char *
expand(name)
	char name[];
{
	static char xname[BUFSIZ];
	char cmdbuf[BUFSIZ];
	register int pid, l, rc;
	register char *cp, *Shell;
	int s, pivec[2], (*sigint)();

	if (!anyof(name, "~{[*?$`'\"\\"))
		return(name);
	/* sigint = signal(SIGINT, SIG_IGN); */
	if (pipe(pivec) < 0) {
		perror("pipe");
		/* signal(SIGINT, sigint) */
		return(name);
	}
	sprintf(cmdbuf, "echo %s", name);
	if ((pid = vfork()) == 0) {
		Shell = value(SHELL);
		if (Shell == NOSTR)
			Shell = "/bin/sh";
		close(pivec[0]);
		close(1);
		dup(pivec[1]);
		close(pivec[1]);
		close(2);
		execl(Shell, Shell, "-c", cmdbuf, 0);
		_exit(1);
	}
	if (pid == -1) {
		perror("fork");
		close(pivec[0]);
		close(pivec[1]);
		return(NOSTR);
	}
	close(pivec[1]);
	l = read(pivec[0], xname, BUFSIZ);
	close(pivec[0]);
	while (wait(&s) != pid);
		;
	s &= 0377;
	if (s != 0 && s != SIGPIPE) {
		fprintf(stderr, "\"Echo\" failed\n");
		return(NOSTR);
	}
	if (l < 0) {
		perror("read");
		return(NOSTR);
	}
	if (l == 0) {
		fprintf(stderr, "\"%s\": No match\n", name);
		return(NOSTR);
	}
	if (l == BUFSIZ) {
		fprintf(stderr, "Buffer overflow expanding \"%s\"\n", name);
		return(NOSTR);
	}
	xname[l] = 0;
	for (cp = &xname[l-1]; *cp == '\n' && cp > xname; cp--)
		;
	*++cp = '\0';
	return(xname);
}

/*
 * Are any of the characters in the two strings the same?
 */

anyof(s1, s2)
	register char *s1, *s2;
{
	register int c;

	while (c = *s1++)
		if (any(c, s2))
			return(1);
	return(0);
}

#define FAIL	-1
#define SUCCESS	0
#define SLCKTIME 5200

#ifdef DIALINOUT
/*
 * disable and reenable:  allow a single line to be use for dialin/dialout
 *  routines added by David Sebok 4/15/84 based on those created by
 *  William Sebok
 */

static int disableflg = 0;

disable(dev)
register char *dev;
{
	register int pid;
	int status;

	setuid(getuid());	/* for chown(uid()) in acu program */
	if ((pid = fork()) == 0) {
		execl(DIALINOUT, "acucntrl", "disable", dev, (char *)0);
		perror("acucntrl");
		exit(9);
	}
	while(wait(&status) != pid)
		;
	if (status != 0) { /* FAIL if ACU exits other than 0 */
		printf("could not disable %s\n",dev);
		return FAIL;
	}
	disableflg = 1;
	return SUCCESS;
}

reenable(dev)
register char *dev;
{
	register int pid;
	int status;

	if (disableflg == 0)
		return FAIL;
	disableflg = 0;
	if ((pid = fork()) == 0) {
		setuid(getuid());	/* for chown(uid()) in acu program */
		execl(DIALINOUT, "acucntrl", "enable", dev, (char *)0);
		perror("acucntrl");
		exit(9);
	}
	/* don't force the user to wait for the enable 
	 * the child will be orphaned when tip exits, so don't
	 * have to worry about it
	 */
	return status ? FAIL : SUCCESS;	/*FAIL if ACU exits other than 0 */
}
#endif DIALINOUT

/* These interface to the uucp locking code */
int Debug = 0;

assert(s1, s2, i)
{
}

logent(msg, sta)
char *msg, *sta;
{
	log(value(HOST), "", msg, sta);
}

delock(dev)
char *dev;
{
#ifdef DIALINOUT
	(void) reenable(dev);
#endif DIALINOUT
	return rmlock(dev);
}
		
mlock(dev)
{
	if (ulockf(dev, SLCKTIME) == FAIL)
		return FAIL;

#ifdef DIALINOUT
	if (strncmp(dev, "ttyd", 4) == 0)
		return disable(dev);
#endif DIALINOUT
	return SUCCESS;
}
