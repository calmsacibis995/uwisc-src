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
static char sccsid[] = "@(#)script.c	5.4 (Berkeley) 11/13/85";
#endif not lint

/*
 * script
 *
 *   Modified 10/15/84 by Derek Zahn to make it a bit more difficult to
 *   run scripts inside of other scripts.  Upon starting a script, we look
 *   for a file called ~/.scriptlock.  If it exists, it will contain a
 *   number.  If this number is equal to the parent process ID of this
 *   script (the shell), the new typescript is not allowed.  Otherwise,
 *   The process ID of the newly created shell is written into ~/.scriptlock.
 *   Upon successful completion of a script, ~/.scriptlock is removed.
 *
 *	 Modified 5/2/85 by David Parter to check for clobbering of 
 *   typescript file.
 *
 *	 Modified 5/9/85 by dave cohrs @ uwisc to try to make script exit
 *	 correctly when the shell process dies.  This shouldn't be a problem
 *	 but there seems to be a kernel bug somewhere.
 *
 * 	Above modifications merged into 4.3 src by david parter 10/26/85
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sgtty.h>
#include <sys/time.h>
#include <sys/file.h>
#ifdef	UW
#include <pwd.h>
#endif	UW

char	*getenv();
char	*ctime();
char	*shell;
FILE	*fscript;
int	master;
int	slave;
int	child;
int	subchild;
char	*fname = "typescript";
int	finish();

#ifdef UW
char	*home;			/* for the .scriptlock */
char	lockfile[80];
FILE	*lfp;
int	shellid;
struct	passwd *pwd;
char	answer[4];		/* for checking clobering of file */
char	*gets();
#endif UW

struct	sgttyb b;
struct	tchars tc;
struct	ltchars lc;
struct	winsize win;
int	lb;
int	l;
char	*line = "/dev/ptyXX";
int	aflg;
#ifdef	UW
int vflag = 0;
#endif	UW

main(argc, argv)
	int argc;
	char *argv[];
{

	shell = getenv("SHELL");
	if (shell == 0)
		shell = "/bin/sh";
#ifdef	UW
	/* Get the home directory for the .scriptlock */

	if((home = getenv("HOME")) == NULL) {
		pwd = getpwuid(getuid());
		home = pwd->pw_dir;
	}
	strcpy(lockfile, home);
	strcat(lockfile, "/.scriptlock");
	if((lfp = fopen(lockfile,"r+")) != NULL) {
		fscanf(lfp, "%d", &shellid);
		fclose(lfp);
		if(shellid == getppid()) {
			printf("You are already running a script.  Type 'exit' to quit it.\n");
			exit(1);
		}
	}
	lfp = fopen(lockfile, "w+");
#endif	UW
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch (argv[0][1]) {

		case 'a':
			aflg++;
			break;

#ifdef UW
        case 'v':
            vflag++;
            break;
#endif UW

		default:
			fprintf(stderr,
#ifdef UW
                "usage: script [ -a ] [ -v ] [ typescript ]\n");
#else
			    "usage: script [ -a ] [ typescript ]\n");
#endif	UW
			exit(1);
		}
		argc--, argv++;
	}
	if (argc > 0)
		fname = argv[0];
#ifdef UW
                            /*  check if clobbering file    */
    if (!aflg && (access(fname,F_OK) == 0)) {
        fprintf(stderr,"script: overwrite %s? ",fname);
        (void) gets(answer);
        if (answer[0] != 'y' && answer[0] != 'Y')
            exit(1);
    }
#endif UW
	if ((fscript = fopen(fname, aflg ? "a" : "w")) == NULL) {
		perror(fname);
		fail();
	}
	getmaster();
	printf("Script started, file is %s\n", fname);
	fixtty();

	(void) signal(SIGCHLD, finish);
	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) {
#ifdef UW
        (void) signal(SIGCHLD, finish);
#endif UW
		subchild = child = fork();
		if (child < 0) {
			perror("fork");
			fail();
		}
		if (child)
#ifdef UW
		{
            fprintf(lfp, "%d", child);
            fclose(lfp);
			dooutput();
		}
#else
			dooutput();
#endif UW
		else
			doshell();
	}
	doinput();
}

doinput()
{
	char ibuf[BUFSIZ];
	int cc;

	(void) fclose(fscript);
	while ((cc = read(0, ibuf, BUFSIZ)) > 0)
		(void) write(master, ibuf, cc);
	done();
}

#include <sys/wait.h>

finish()
{
	union wait status;
	register int pid;
	register int die = 0;

	while ((pid = wait3(&status, WNOHANG, 0)) > 0)
		if (pid == child)
			die = 1;

	if (die)
		done();
}

dooutput()
{
	time_t tvec;
	char obuf[BUFSIZ];
	int cc;

	(void) close(0);
	tvec = time((time_t *)0);
	fprintf(fscript, "Script started on %s", ctime(&tvec));
	for (;;) {
		cc = read(master, obuf, sizeof (obuf));
		if (cc <= 0)
			break;
		(void) write(1, obuf, cc);
#ifdef UW
        if (vflag) {           /* the sucker's playing rogue */
            int scan;

            scan = sizeof (obuf);
            while (scan--)
                if (obuf[scan] == '\015') obuf[scan] = '\0';
        }
#endif UW
		(void) fwrite(obuf, 1, cc, fscript);
	}
	done();
}

doshell()
{
	int t;

	t = open("/dev/tty", O_RDWR);
	if (t >= 0) {
		(void) ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	}
	getslave();
	(void) close(master);
	(void) fclose(fscript);
	(void) dup2(slave, 0);
	(void) dup2(slave, 1);
	(void) dup2(slave, 2);
	(void) close(slave);
	execl(shell, "sh", "-i", 0);
	perror(shell);
	fail();
}

fixtty()
{
	struct sgttyb sbuf;

	sbuf = b;
	sbuf.sg_flags |= RAW;
	sbuf.sg_flags &= ~ECHO;
	(void) ioctl(0, TIOCSETP, (char *)&sbuf);
}

fail()
{

	(void) kill(0, SIGTERM);
	done();
}

done()
{
	time_t tvec;

	if (subchild) {
		tvec = time((time_t *)0);
		fprintf(fscript,"\nscript done on %s", ctime(&tvec));
		(void) fclose(fscript);
		(void) close(master);
	} else {
		(void) ioctl(0, TIOCSETP, (char *)&b);
		printf("Script done, file is %s\n", fname);
#ifdef	UW
		unlink(lockfile);
#endif	UW
	}
	exit(0);
}

getmaster()
{
	char *pty, *bank, *cp;
	struct stat stb;

	pty = &line[strlen("/dev/ptyp")];
	for (bank = "pqrs"; *bank; bank++) {
		line[strlen("/dev/pty")] = *bank;
		*pty = '0';
		if (stat(line, &stb) < 0)
			break;
		for (cp = "0123456789abcdef"; *cp; cp++) {
			*pty = *cp;
			master = open(line, O_RDWR);
			if (master >= 0) {
				char *tp = &line[strlen("/dev/")];
				int ok;

				/* verify slave side is usable */
				*tp = 't';
				ok = access(line, R_OK|W_OK) == 0;
				*tp = 'p';
				if (ok) {
				    (void) ioctl(0, TIOCGETP, (char *)&b);
				    (void) ioctl(0, TIOCGETC, (char *)&tc);
				    (void) ioctl(0, TIOCGETD, (char *)&l);
				    (void) ioctl(0, TIOCGLTC, (char *)&lc);
				    (void) ioctl(0, TIOCLGET, (char *)&lb);
				    (void) ioctl(0, TIOCGWINSZ, (char *)&win);
					return;
				}
				(void) close(master);
			}
		}
	}
	fprintf(stderr, "Out of pty's\n");
	fail();
}

getslave()
{

	line[strlen("/dev/")] = 't';
	slave = open(line, O_RDWR);
	if (slave < 0) {
		perror(line);
		fail();
	}
	(void) ioctl(slave, TIOCSETP, (char *)&b);
	(void) ioctl(slave, TIOCSETC, (char *)&tc);
	(void) ioctl(slave, TIOCSLTC, (char *)&lc);
	(void) ioctl(slave, TIOCLSET, (char *)&lb);
	(void) ioctl(slave, TIOCSETD, (char *)&l);
	(void) ioctl(slave, TIOCSWINSZ, (char *)&win);
}
