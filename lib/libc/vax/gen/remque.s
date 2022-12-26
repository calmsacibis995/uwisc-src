#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: remque.s,v 1.2 86/09/08 15:52:47 tadl Exp $"
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
	.asciz	"@(#)remque.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* remque(entry) */

#include "DEFS.h"

ENTRY(remque, 0)
	remque	*4(ap),r0
	ret
