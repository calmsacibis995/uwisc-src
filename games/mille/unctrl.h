/* $Header: unctrl.h,v 1.1 86/08/26 20:21:41 root Exp $ */

/*
 * unctrl.h
 */

extern char	*_unctrl[];

# define	unctrl(ch)	(_unctrl[ch & 0177])
