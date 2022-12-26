/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kernel.h	7.1 (Berkeley) 6/4/86
 */
/*
 * RCS Info	
 *	$Header: kernel.h,v 3.1 86/10/22 13:24:27 tadl Exp $
 *	$Locker:  $
 */

/*
 * Global variables for the kernel
 */

long	rmalloc();

/* 1.1 */
long	hostid;
char	hostname[MAXHOSTNAMELEN];
int	hostnamelen;
/* #ifdef NFS */
char	domainname[32];
int		domainnamelen;
/* #endif NFS */

/* 1.2 */
struct	timeval boottime;
struct	timeval time;
struct	timezone tz;			/* XXX */
int	hz;
int	phz;				/* alternate clock's frequency */
int	tick;
int	lbolt;				/* awoken once a second */
int	realitexpire();

#ifdef UW
double	avenrun[4];
#else
double	avenrun[3];
#endif UW

#ifdef GPROF
extern	int profiling;
extern	char *s_lowpc;
extern	u_long s_textsize;
extern	u_short *kcount;
#endif
