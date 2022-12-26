#if defined(LIBC_RCS) && !defined(lint)
_rcsid: .asciz "$Header: htons.c,v 1.2 86/09/08 15:53:35 tadl Exp $"
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
_sccsid:.asciz	"@(#)htons.c	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* hostorder = htons(netorder) */

#include "DEFS.h"

ENTRY(htons)
	rotl	$8,4(ap),r0
	movb	5(ap),r0
	movzwl	r0,r0
	ret
