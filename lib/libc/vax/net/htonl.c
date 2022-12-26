#if defined(LIBC_RCS) && !defined(lint)
_rcsid: .asciz	"$Header: htonl.c,v 1.2 86/09/08 15:53:32 tadl Exp $"
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
_sccsid:.asciz	"@(#)htonl.c	5.3 (Berkeley) 3/9/86"
#endif LIBC_SCCS

/* netorder = htonl(hostorder) */

#include "DEFS.h"

ENTRY(htonl)
	rotl	$-8,4(ap),r0
	insv	r0,$16,$8,r0
	movb	7(ap),r0
	ret
