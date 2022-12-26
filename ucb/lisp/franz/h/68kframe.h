/*
 * $Header: 68kframe.h,v 1.1 86/08/26 22:08:14 root Exp $
 * $Locker:  $
 * machine stack frame
 */
struct machframe {
struct 	machframe	*fp;
	lispval	(*pc)();
	lispval ap[1];
};
