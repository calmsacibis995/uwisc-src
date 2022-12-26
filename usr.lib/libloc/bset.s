/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
	.asciz	"@(#)bzero.s	5.2 (Berkeley) 6/5/85"
#endif not lint

/* bset(base, length, value) */

#include "DEFS.h"

ENTRY(bset, R6)
	movl	4(ap),r3
	movl	12(ap),r6
	jbr	2f
1:
	subl2	r0,8(ap)
	movc5	$0,(r3),r6,r0,(r3)
2:
	movzwl	$65535,r0
	cmpl	8(ap),r0
	jgtr	1b
	movc5	$0,(r3),r6,8(ap),(r3)
	ret
