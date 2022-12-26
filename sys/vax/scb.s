/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)scb.s	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
	.data
_scb_rcs_id: 
	.asciz "$Header: scb.s,v 2.1 86/08/13 13:27:27 tadl Exp $"
	.text
#endif not lint
/*
 * RCS Info
 *	$Locker: tadl $
 */


#include "uba.h"

/*
 * System control block
 */
	.set	INTSTK,1	# handle this interrupt on the interrupt stack
	.set	HALT,3		# halt if this interrupt occurs

_scb:	.globl	_scb

#define	STRAY	.long	_Xstray+INTSTK
#define	STRAY8	STRAY;STRAY;STRAY;STRAY;STRAY;STRAY;STRAY;STRAY
#define	STRAY15	STRAY;STRAY;STRAY;STRAY;STRAY;STRAY;STRAY;STRAY8
#define	KS(a)	.long	_X/**/a
#define	IS(a)	.long	_X/**/a+INTSTK
#define	STOP(a)	.long	_X/**/a+HALT

/* 000 */	STRAY;		IS(machcheck);	IS(kspnotval);	STOP(powfail);
/* 010 */	KS(privinflt);	KS(xfcflt);	KS(resopflt);	KS(resadflt);
/* 020 */	KS(protflt);	KS(transflt);	KS(tracep);	KS(bptflt);
/* 030 */	KS(compatflt);	KS(arithtrap);	STRAY;		STRAY;
/* 040 */	KS(syscall);	KS(chme);	KS(chms);	KS(chmu);
/* 050 */	STRAY;		IS(cmrd);	STRAY;		STRAY;
/* 060 */	IS(wtime);	STRAY;		STRAY;		STRAY;
/* 070 */	STRAY;		STRAY;		STRAY;		STRAY;
/* 080 */	STRAY;		STRAY;		KS(astflt);	STRAY;
/* 090 */	STRAY;		STRAY;		STRAY;		STRAY;
/* 0a0 */	IS(softclock);	STRAY;		STRAY;		STRAY;
/* 0b0 */	IS(netintr);	STRAY;		STRAY;		STRAY;
/* 0c0 */	IS(hardclock);	STRAY;		KS(emulate);	KS(emulateFPD);
/* 0d0 */	STRAY;		STRAY;		STRAY;		STRAY;
/* 0e0 */	STRAY;		STRAY;		STRAY;		STRAY;
/* 0f0 */	IS(consdin);	IS(consdout);	IS(cnrint);	IS(cnxint);
/* 100 */	IS(nexzvec); STRAY15;		/* ipl 0x14, nexus 0-15 */
/* 140 */	IS(nexzvec); STRAY15;		/* ipl 0x15, nexus 0-15 */
/* 180 */	IS(nexzvec); STRAY15;		/* ipl 0x16, nexus 0-15 */
/* 1c0 */	IS(nexzvec); STRAY15;		/* ipl 0x17, nexus 0-15 */

	.globl	_UNIvec
_UNIvec:	.space	512		# 750 unibus intr vector
					# 1st UBA jump table on 780's
#if NUBA > 1
	.globl	_UNI1vec
_UNI1vec:	.space	512		# 750 second unibus intr vector
					# 2nd UBA jump table on 780's
#endif
