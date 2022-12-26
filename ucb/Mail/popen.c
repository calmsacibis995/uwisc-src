/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char *sccsid = "@(#)popen.c	5.2 (Berkeley) 6/21/85";
#endif not lint

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1
static	int	popen_pid[20];

#ifndef VMUNIX
#define vfork	fork
#endif VMUNIX
#ifndef	SIGRETRO
#define	sigchild()
#endif

FILE *
popen(cmd,mode)
char	*cmd;
char	*mode;
{
	int p[2];
	register myside, hisside, pid;
#ifdef UW
    char *index();
#endif UW

	if(pipe(p) < 0)
		return NULL;
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		sigchild();
		close(myside);
		dup2(hisside, tst(0, 1));
		close(hisside);
#ifdef UW
        /* popen() only used with one-word commands.  Skip the csh stuff */
        if((index(cmd, ' ') == NULL) && (index(cmd, '|') == NULL))
            execl(cmd, cmd, 0);
        else
#endif UW
		execl("/bin/csh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return NULL;
	popen_pid[myside] = pid;
	close(hisside);
	return(fdopen(myside, mode));
}

pclose(ptr)
FILE *ptr;
{
	register f, r;
	int status, omask;
	extern int errno;

	f = fileno(ptr);
	fclose(ptr);
# ifdef VMUNIX
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
# endif VMUNIX
	while((r = wait(&status)) != popen_pid[f] && r != -1 && errno != EINTR)
		;
	if(r == -1)
		status = -1;
# ifdef VMUNIX
	sigsetmask(omask);
# endif VMUNIX
	return(status);
}
