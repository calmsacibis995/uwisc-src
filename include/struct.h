/*
 * RCS Info	
 *	$Header: struct.h,v 1.1 86/09/05 08:03:05 tadl Exp $
 *	$Locker: tadl $
 */
/*	struct.h	4.1	83/05/03	*/

/*
 * access to information relating to the fields of a structure
 */

#define	fldoff(str, fld)	((int)&(((struct str *)0)->fld))
#define	fldsiz(str, fld)	(sizeof(((struct str *)0)->fld))
#define	strbase(str, ptr, fld)	((struct str *)((char *)(ptr)-fldoff(str, fld)))
