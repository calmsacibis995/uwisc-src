#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: bcmp.s,v 1.2 86/09/08 15:52:02 tadl Exp $"
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
	.asciz	"@(#)bcmp.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* bcmp(s1, s2, n) */

#include "DEFS.h"

ENTRY(bcmp, 0)
	movl	4(ap),r1
	movl	8(ap),r3
	movl	12(ap),r4
1:
	movzwl	$65535,r0
	cmpl	r4,r0
	jleq	2f
	subl2	r0,r4
	cmpc3	r0,(r1),(r3)
	jeql	1b
	addl2	r4,r0
	ret
2:
	cmpc3	r4,(r1),(r3)
	ret
