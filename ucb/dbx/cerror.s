#!/bin/csh -f
#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)cerror.s	5.1 (Berkeley) 5/31/85
#
# static char rcsid[] = "$Header: cerror.s,v 1.1 86/08/26 21:33:18 root Exp $";
#
# modified version of cerror
#
# The idea is that every time an error occurs in a system call
# I want a special function "syserr" called.  This function will
# either print a message and exit or do nothing depending on
# defaults and use of "onsyserr".
#

.globl	cerror
.comm	_errno,4

cerror:
	movl	r0,_errno
	calls	$0,_syserr	# new code
	mnegl	$1,r0
	ret

.globl	__mycerror		# clumsy way to get this loaded

__mycerror:
	.word	0
	ret
