/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rpb.s	7.1 (Berkeley) 6/5/86
 */
/*
 * RCS Info
 *	$Header: rpb.s,v 2.1 86/08/13 13:27:22 tadl Exp $
 *	$Locker: tadl $
 */


/*
 * This has to get loaded first (physical 0) as 780 memory restart rom
 * only looks for rpb on a 64K page boundary (doc isn't wrong,
 * it never says what size "page boundary" rpb has to be on).
 */
	.globl	_rpb
_rpb:
	.space	508
erpb:
	.space	4
