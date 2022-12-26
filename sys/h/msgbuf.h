/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)msgbuf.h	7.1 (Berkeley) 6/4/86
 */
/*
 * RCS Info	
 *	$Header: msgbuf.h,v 3.1 86/10/22 13:24:57 tadl Exp $
 *	$Locker:  $
 */

#define	MSG_MAGIC	0x063061
#define	MSG_BSIZE	(4096 - 3 * sizeof (long))
struct	msgbuf {
	long	msg_magic;
	long	msg_bufx;
	long	msg_bufr;
	char	msg_bufc[MSG_BSIZE];
};
#ifdef KERNEL
struct	msgbuf msgbuf;
#endif
