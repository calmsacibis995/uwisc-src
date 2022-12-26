#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: exportfs.c,v 1.2 86/09/08 15:55:38 tadl Exp $"
	.text
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/* @(#)exportfs.c 1.1 86/02/03 SMI */

#include "SYS.h"

SYSCALL(exportfs)
	ret
