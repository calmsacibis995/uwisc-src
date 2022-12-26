#if defined(LIBC_RCS) && !defined(lint)
	.data
_rcsid: .asciz "$Header: setdomainname.c,v 1.2 86/09/08 15:59:04 tadl Exp $"
	.text
#endif
/*
 * RCS info
 *	$Locker:  $
 */

#include "SYS.h"

SYSCALL(setdomainname)
	ret		/* setdomainname(name, len) */
