#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: ptrace.c,v 1.2 86/09/08 15:58:11 tadl Exp $"
	.text
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
_sccsid:.asciz	"@(#)ptrace.c	5.3 (Berkeley) 3/9/86"
#endif SYSLIBC_SCCS

#include "SYS.h"

ENTRY(ptrace)
	clrl	_errno
	chmk	$SYS_ptrace
	jcs	err
	ret
err:
	jmp	cerror
