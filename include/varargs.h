/*
 * RCS Info	
 *	$Header: varargs.h,v 1.1 86/09/05 08:03:21 tadl Exp $
 *	$Locker: tadl $
 */
/*	varargs.h	4.1	83/05/03	*/

typedef char *va_list;
# define va_dcl int va_alist;
# define va_start(list) list = (char *) &va_alist
# define va_end(list)
# define va_arg(list,mode) ((mode *)(list += sizeof(mode)))[-1]
