/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)cmdtab.c	5.1 (Berkeley) 4/30/85";
#endif not lint

#include "tip.h"

extern	int shell(), getfl(), sendfile(), chdirectory();
extern	int finish(), help(), pipefile(), consh(), variable();
extern	int cu_take(), cu_put(), dollar(), genbrk(), suspend();

esctable_t etable[] = {
	{ '!',	NORM,	"shell",			 shell },
	{ '<',	NORM,	"receive file from remote host", getfl },
	{ '>',	NORM,	"send file to remote host",	 sendfile },
	{ 't',	NORM,	"take file from remote UNIX",	 cu_take },
	{ 'p',	NORM,	"put file to remote UNIX",	 cu_put },
	{ '|',	NORM,	"pipe remote file",		 pipefile },
#ifdef CONNECT
	{ 'C',  NORM,	"connect program to remote host",consh },
#endif
	{ 'c',	NORM,	"change directory",		 chdirectory },
	{ '.',	NORM,	"exit from tip",		 finish },
	{CTRL(d),NORM,	"exit from tip",		 finish },
	{CTRL(z),NORM,	"suspend tip",			 suspend },
	{ 's',	NORM,	"set variable",			 variable },
	{ '?',	NORM,	"get this summary",		 help },
	{ '#',	NORM,	"send break",			 genbrk },
	{ 0, 0, 0 }
};
