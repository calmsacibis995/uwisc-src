#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: fstatfs.c,v 1.2 86/09/08 15:56:01 tadl Exp $"
	.text
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#include "SYS.h"

SYSCALL(fstatfs)
	ret
