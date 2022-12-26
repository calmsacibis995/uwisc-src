#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: bcopy.s,v 1.2 86/09/08 15:52:06 tadl Exp $"
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
	.asciz	"@(#)bcopy.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* bcopy(from, to, size) */

#include "DEFS.h"

ENTRY(bcopy, R6)
	movl	4(ap),r1
	movl	8(ap),r3
	movl	12(ap),r6
	cmpl	r1,r3
	bgtr	2f		# normal forward case
	blss	3f		# overlapping, must do backwards
	ret			# equal, nothing to do
1:
	subl2	r0,r6
	movc3	r0,(r1),(r3)
2:
	movzwl	$65535,r0
	cmpl	r6,r0
	jgtr	1b
	movc3	r6,(r1),(r3)
	ret
3:
	addl2	r6,r1
	addl2	r6,r3
	movzwl	$65535,r0
	jbr	5f
4:
	subl2	r0,r6
	subl2	r0,r1
	subl2	r0,r3
	movc3	r0,(r1),(r3)
	movzwl	$65535,r0
	subl2	r0,r1
	subl2	r0,r3
5:
	cmpl	r6,r0
	jgtr	4b
	subl2	r6,r1
	subl2	r6,r3
	movc3	r6,(r1),(r3)
	ret
