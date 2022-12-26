#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: strlen.s,v 1.2 86/09/08 15:53:10 tadl Exp $"
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
	.asciz	"@(#)strlen.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/*
 * Return the length of cp (not counting '\0').
 *
 * strlen(cp)
 *	char *cp;
 */
#include "DEFS.h"

ENTRY(strlen, 0)
	movl	4(ap),r1
1:
	locc	$0,$65535,(r1)	# look for '\0'
	beql	1b
	subl3	4(ap),r1,r0	# len = cp - base
	ret
