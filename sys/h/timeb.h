/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)timeb.h	7.1 (Berkeley) 6/4/86
 */
/*
 * RCS Info	
 *	$Header: timeb.h,v 3.1 86/10/22 13:29:04 tadl Exp $
 *	$Locker:  $
 */

/*
 * Structure returned by ftime system call
 */
struct timeb
{
	time_t	time;
	unsigned short millitm;
	short	timezone;
	short	dstflag;
};
