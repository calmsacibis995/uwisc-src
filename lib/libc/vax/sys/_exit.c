#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: _exit.c,v 1.2 86/09/08 15:55:35 tadl Exp $"
	.text
	.data
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

#ifdef SYSLIBC_SCCS
_sccsid:.asciz	"@(#)_exit.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

#include "SYS.h"

	.align	1
PSEUDO(_exit,exit)
			# _exit(status)
