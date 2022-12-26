
/* Proteon V2LNI */

#define VIIDEV 24
#define	VII_BROADCAST	255

/* Bit definition - new style */

#define	VII_ENB	01		/* Enable Operation */
#define	VII_DEN	02		/* Enable DMA */
#define	VII_HEN	04		/* Host Relay Enable (Rcv) */
#define	VII_CPB	04		/* Clear Packet Buffer (Xmit) */
#define	VII_STE	010		/* Self Test Enable (Rcv) */
#define	VII_UT1	010		/* Unused (Xmit) */
#define	VII_LPB	020		/* Modem Disable (Rcv) */
#define	VII_INR	020		/* Initialize Ring (Xmit) */
#define	VII_RST	040		/* Reset */
#define	VII_IEN	0100		/* Interrupt Enable */
#define	VII_RDY	0200		/* Done */
#define	VII_DPR	0400		/* Data Present (Rcv) */
#define	VII_RFS	0400		/* Refused (Xmit) */
#define	VII_NXM	01000		/* Non Existent Memory */
#define	VII_OVR	02000		/* Overrun */
#define	VII_ODB	04000		/* Odd Byte (Achtung, mein Fuehrer) (Rcv) */
#define	VII_UT2	04000		/* Unused (Xmit) */
#define	VII_LDE	010000		/* Link Data Error (Rcv) */
#define	VII_OPT	010000		/* Output Timeout (Xmit) */
#define	VII_NOK	020000		/* Ring Not OK */
#define	VII_BDF	040000		/* Bad Format in Operation */
#define	VII_NIR	0100000		/* Not in Ring */

#define	VIIXERR	(VII_NXM|VII_OVR|VII_OPT|VII_BDF)	/* Xmit errs */
#define	VIIRERR	(VII_NXM|VII_OVR|VII_ODB|VII_LDE|VII_BDF)	/* Rcv errs */
#define	VIIHE	(VII_NXM|VII_OVR)			/* Fatal errors */

#define VII_IBITS \
"\10\20NIR\17BDF\16NOK\15LDE\14ODB\13OVR\12NXM\11DPR\10RDY\7IEN\6RST\5LPB\4STE\3HEN\2DEN\1ENB"

#define VII_OBITS \
"\10\20NIR\17BDF\16NOK\15OPT\13OVR\12NXM\11RFS\10RDY\7IEN\6RST\5INR\3HEN\2DEN\1ENB"

struct viiregs {			/* device registers */
	short viircsr;				/* input csr */
	unsigned short viirwcr;			/* input word count */
	unsigned short viirarl;			/* input addr lo */
	unsigned short viirarh;			/* input addr hi */
	short viixcsr;				/* output csr */
	unsigned short viixwcr;			/* output word count */
	unsigned short viixarl;			/* output addr lo */
	unsigned short viixarh;			/* output addr hi */
};
		
