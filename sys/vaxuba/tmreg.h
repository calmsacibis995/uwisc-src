/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)tmreg.h	7.1 (Berkeley) 6/5/86
 */
/*
 * RCS Info	
 *	$Header: tmreg.h,v 3.1 86/10/22 14:08:06 tadl Exp $
 *	$Locker:  $
 */

/*
 * TM11 controller registers
 */
struct tmdevice {
	u_short	tmer;		/* error register, per drive */
	u_short	tmcs;		/* control-status register */
	short	tmbc;		/* byte/frame count */
	u_short tmba;		/* address */
	short	tmdb;		/* data buffer */
	short	tmrd;		/* read lines */
	short	tmmr;		/* maintenance register */
#ifdef	AVIV
	short	tmfsr;		/* formatter status reading */
#endif
};

#define	b_repcnt  b_bcount
#define	b_command b_resid

/* bits in tmcs */
#define	TM_GO		0000001
#define	TM_OFFL		0000000		/* offline */
#define	TM_RCOM		0000002		/* read */
#define	TM_WCOM		0000004		/* write */
#define	TM_WEOF		0000006		/* write-eof */
#define	TM_SFORW	0000010		/* space forward */
#define	TM_SREV		0000012		/* space backwards */
#define	TM_WIRG		0000014		/* write with xtra interrecord gap */
#define	TM_REW		0000016		/* rewind */
#define	TM_SENSE	TM_IE		/* sense (internal to driver) */

#define	tmreverseop(cmd)		((cmd)==TM_SREV || (cmd)==TM_REW)

/* TM_SNS is a pseudo-op used to get tape status */
#define	TM_IE		0000100		/* interrupt enable */
#define	TM_CUR		0000200		/* control unit is ready */
#define	TM_DCLR		0010000		/* drive clear */
#define	TM_D800		0060000		/* select 800 bpi density */
#define	TM_ERR		0100000		/* drive error summary */

/* bits in tmer */
#define	TMER_ILC	0100000		/* illegal command */
#define	TMER_EOF	0040000		/* end of file */
#define	TMER_CRE	0020000		/* cyclic redundancy error */
#define	TMER_PAE	0010000		/* parity error */
#define	TMER_BGL	0004000		/* bus grant late */
#define	TMER_EOT	0002000		/* at end of tape */
#define	TMER_RLE	0001000		/* record length error */
#define	TMER_BTE	0000400		/* bad tape error */
#define	TMER_NXM	0000200		/* non-existant memory */
#define	TMER_SELR	0000100		/* tape unit properly selected */
#define	TMER_BOT	0000040		/* at beginning of tape */
#define	TMER_CH7	0000020		/* 7 channel tape */
#define	TMER_SDWN	0000010		/* gap settling down */
#define	TMER_WRL	0000004		/* tape unit write protected */
#define	TMER_RWS	0000002		/* tape unit rewinding */
#define	TMER_TUR	0000001		/* tape unit ready */

#define	TMER_BITS	\
"\10\20ILC\17EOF\16CRE\15PAE\14BGL\13EOT\12RLE\11BTE\10NXM\
\7SELR\6BOT\5CH7\4SDWN\3WRL\2RWS\1TUR"

#define	TMER_HARD	(TMER_ILC|TMER_EOT)
#define	TMER_SOFT	(TMER_CRE|TMER_PAE|TMER_BGL|TMER_RLE|TMER_BTE|TMER_NXM)

#ifdef	AVIV
/* bits in tmmr (formatter diagnostic reading) */
#define	DTS		000000		/* select dead track status */
#   define	DTS_MASK	0xff

#define	DAB		010000		/* select diagnostic aid bits */
#   define  DAB_MASK		037	/* reject code only */

#define	RWERR		020000		/* select read-write errors */
#    define RWERR_MASK		01777	/* include bit 9 (MAI) */
#    define RWERR_BITS \
"\10\12MAI\11CRC ERR\10WTMCHK\7UCE\6PART REC\5MTE\3END DATA CHK\
\2VEL ERR\1DIAG MODE"

#define	DRSENSE		030000		/* select drive sense */
#    define DRSENSE_MASK	0777
#    define DRSENSE_BITS \
"\10\11WRTS\10EOTS\7BOTS\6WNHB\5PROS\4BWDS\3HDNG\2RDYS\1ON LINE"

#define	CRCF		040000		/* CRC-F Generator */

#define	FSR_BITS \
"\10\20REJ\17TMS\16OVRN\15DATACHK\14SSC\13EOTS\12WRTS\11ROMPS\10CRERR\
\7ONLS\6BOTS\5HDENS\4BUPER\3FPTS\2REWS\1RDYS"
#endif	AVIV
