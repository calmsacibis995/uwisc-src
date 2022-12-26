#ifndef lint
	.data
_rcs_id: 
	.asciz "$Header: udiv.s,v 2.1 86/08/13 13:27:32 tadl Exp $"
	.text
#endif not lint
/*
 * RCS Info
 *	$Locker: tadl $
 */
/*
 *	@(#)udiv.s	7.1 (Berkeley) 6/5/86
 *
 *	udiv - unsigned division for vax-11
 *
 *	arguments: dividend, divisor.
 *	result: quotient.
 *	uses r0-r2
 *
 *	If 1 < divisor <= 2147483647, zero-extend the dividend
 *	to 64 bits and let ediv do the work.  If the divisor is 1,
 *	ediv will overflow if bit 31 of the dividend is on, so
 *	just return the dividend unchanged.  If the divisor is 0,
 *	do the ediv also, so it will generate the proper exception.
 *	All other values of the divisor have bit 31 on: in this case
 *	the quotient must be 0 if divisor > dividend, and 1 otherwise,
 *	provided that the comparison is made as unsigned.
 */

	.text
	.align	1
	.globl	udiv
udiv:	.word	0x0000
#ifdef GPROF
	jsb	mcount
#endif GPROF
	movl	4(ap),r0	# Dividend
	movl	8(ap),r2	# Divisor
	jeql	div		# If divisor=0, force exception
	cmpl	r2,$1		# If divisor <= 1 (signed),
	jleq	nodiv		#  no division is necessary
div:	clrl	r1		# Zero-extend the dividend
	ediv	r2,r0,r0,r2	# Divide.  q->r0, r->r2 (discarded)
	ret
nodiv:	jeql	retn		# If divisor=1, return dividend
	cmpl	r0,r2		# Unsigned comparison between
	jgequ	one		#  dividend and divisor
	clrl	r0		# Dividend < divisor, return 0
	ret
one:	movl	$1,r0		# Dividend >= divisor, return 1
retn:	ret
/*
 * audiv - unsigned dividion for vax-11
 *
 * arguments: *dividend, divisor.
 * result: quotient in r0 and *dividend.
 * uses: r0-r2
 */

	.text
	.align	1
	.globl	audiv
audiv:	.word	0x0000
#ifdef GPROF
	jsb	mcount
#endif GPROF
	movl	*4(ap),r0	# Dividend
	movl	8(ap),r2	# Divisor
	jeql	adiv		# If divisor=0, force exception
	cmpl	r2,$1		# If divisor <= 1 (signed),
	jleq	anodiv		#  no division is necessary
adiv:	clrl	r1		# Zero-extend the dividend
	ediv	r2,r0,r0,r2	# Divide.  q->r0, r->r2 (discarded)
	movl	r0,*4(ap)	# save result
	ret
anodiv:	jeql	aretn		# If divisor=1, return dividend
	cmpl	r0,r2		# Unsigned comparison between
	jgequ	aone		#  dividend and divisor
	clrl	r0		# Dividend < divisor, return 0
	movl	r0,*4(ap)	# save result
	ret
aone:	movl	$1,r0		# Dividend >= divisor, return 1
aretn:	movl	r0,*4(ap)	# save result
	ret
