#ifdef LIBC_RCS
_rcsid: .asciz	"$Header: rindex.s,v 1.2 86/09/08 15:52:50 tadl Exp $"
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
	.asciz	"@(#)rindex.s	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/*
 * Find the last occurence of c in the string cp.
 * Return pointer to match or null pointer.
 *
 * char *
 * rindex(cp, c)
 *	char *cp, c;
 */
#include "DEFS.h"

ENTRY(rindex, 0)
	movq	4(ap),r1	# r1 = cp; r2 = c
	tstl	r2		# check for special case c == '\0'
	bneq	2f
1:
	locc	$0,$65535,(r1)	# just find end of string
	beql	1b		# still looking
	movl	r1,r0		# found it
	ret
2:
	moval	tbl,r3		# r3 = address of table
	bbss	$0,(r3),5f	# insure not reentering
	movab	(r3)[r2],r5	# table entry for c
	incb	(r5)
	clrl	r4		# last found
3:
	scanc	$65535,(r1),(r3),$1	# look for c or '\0'
	beql	3b		# keep looking
	tstb	(r1)		# if have found '\0'
	beql	4f		#    we are done
	movl	r1,r4		# save most recently found
	incl	r1		# skip over character
	jbr	3b		# keep looking
4:
	movl	r4,r0		# return last found (if any)
	clrb	(r5)		# clean up table
	clrb	(r3)
	ret

	.data
tbl:	.space	256
	.text

/*
 * Reentrant, but slower version of rindex
 */
5:
	movl	r1,r3
	clrl	r4		# r4 = pointer to last match
6:
	locc	$0,$65535,(r3)	# look for '\0'
	bneq	8f
	decw	r0		# r0 = 65535
1:
	locc	r2,r0,(r3)	# look for c
	bneq	7f
	movl	r1,r3		# reset pointer and ...
	jbr	6b		# ... try again
7:
	movl	r1,r4		# stash pointer ...
	addl3	$1,r1,r3	# ... skip over match and ...
	decl	r0		# ... decrement count
	jbr	6b		# ... try again
8:
	subl3	r3,r1,r0	# length of short block
	incl	r0		# +1 for '\0'
9:
	locc	r2,r0,(r3)	# look for c
	beql	0f
	movl	r1,r4		# stash pointer ...
	addl3	$1,r1,r3	# ... skip over match ...
	decl	r0		# ... adjust count and ...
	jbr	9b		# ... try again
0:
	movl	r4,r0		# return stashed pointer
	ret
