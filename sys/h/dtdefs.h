/* options */
#ifndef DTDEBUG
#define	DTDEBUG			/* Debugging */
#endif

#define	DTSTATS			/* Statistics */
#define	DTHEADONLY		/* allow header only transfers */
/* eventually these oughta be permanant - you can take out the ifdefs : */
#define DT_THREE
#define DTRW
#define DTNOSTE
#define DTNOINR
#define DTRWX
#define DTRUNTCK
#define DTFLY

/* general */
#define FALSE		0
#define TRUE		1
#define	DT_ERROR	0
#define	DT_OK		1

/* identification */
#define	DT_VERSION		77
#define	DT_BOOTPK		0x1234
#define	VII_BROADCAST	255

/* sizes */
#define	DT_PKTSZ	2046
#define	DT_NSOCK	64
#define	DT_NNODE	64
#define	DT_HEADSZ	(sizeof(struct dt_head))
#define DT_BODYSZ	(DT_PKTSZ-DT_HEADSZ)
#define	DT_ACKSZ	4

#ifndef STANDALONE
#ifdef KERNEL
/* transmit states */
#define	DT_XOFF			0
#define DT_XSTART		1
#define	DT_XACK			2
#define	DT_XHEAD		3
#define	DT_XADONE		4
#define	DT_XBDONE		5


/* receive states */
#define	DT_RSTART		0
#define DT_RHEAD		1
#define DT_RBODY		2
#define	DT_RDONE		3
#define DT_RSTOP		4	

#ifdef DTDEBUG
char	*dtxstates[7] = {
		"XOFF", "XSTART", "XACK", "XHEAD", "XADONE", "XBDONE"
		};
char	*dtrstates[5] = {
		"RSTART", "RHEAD", "RBODY", "RDONE", "RSTOP"
		};
#endif DTDEBUG
#endif KERNEL
#endif STANDALONE

/*
** Packet header
** Ack body and packet header.
*/
#ifndef DT_THREE
struct	dt_head {
	u_char	dh_dstn;			/* destination node number */
	u_char	dh_srcn;			/* source node number */
	u_char	dh_vers;			/* version number */
	u_char						/* flag bits */
			dh_res:3,					/* reserved bits */
			dh_val:1,					/* ack valid bit */
			dh_bak:1,					/* bare ack bit */
			dh_dgm:1,					/* datagram bit */
			dh_ack:1,					/* ack bit */
			dh_seq:1;					/* sequence number bit */
	u_short	dh_rsvd;			/* reserved field */
	u_short	dh_lgth;			/* length of body in bytes */
	u_short	dh_dsts;			/* destination socket */
	u_short	dh_srcs;			/* source socket */
};
#else DT_THREE

#undef DT_VERSION
#define	DT_VERSION		86

#undef DT_ACKSZ
#define	DT_ACKSZ	(sizeof(struct dt_head))

typedef struct cry_addr CRYADDR;
struct cry_addr {
	u_char	c_net;
	u_char	c_node;
};

typedef struct cry_cap CRYCAP;
struct cry_cap {
	u_long	c_ctrl;
	u_long	c_comm;
};

struct dt_head {
	/* crystal net specific fields */
	/* LAN (hardware) Specific Fields */
	u_char			dt_vvdst;		/* Hardware node destination             */
	u_char			dt_vvsrc;		/* Hardware node source (set by hardware)*/
	u_char			dt_vvvers;		/* LAN version # (RING_VERSION)          */
	u_char			dt_vvproto;		/* LAN protocol #  (RING_CRY)            */

	/* Crystal Net (software) Specific Fields */
	u_long			dt_seq;			/* sequence number                       */
	CRYCAP			dt_cap;			/* capability                            */
	CRYADDR			dt_dtdst;		/* net and  node of dst                  */
	CRYADDR			dt_dtsrc;		/* net and  node of src                  */
	u_short			dt_dport;		/* port of dst                           */
	u_short			dt_sport;		/* port of src                           */
	u_short			dt_len;			/* Length of data + header               */
	u_short			dt_credit;		/* Amount of data, credit                */
	u_char			dt_vers;		/* version number                        */
	u_char			dt_proto;		/* protocol type                         */

	/* Delta_t specific fields */
	u_char	dt_flags;	/* Should be u_short and not have bit fields below */
	u_char						/* flag bits */
			dh_res:3,					/* reserved bits */
			dh_val:1,					/* ack valid bit */
			dh_bak:1,					/* bare ack bit */
			dh_dgm:1,					/* datagram bit */
			dh_ack:1,					/* ack bit */
			dh_seq:1;					/* sequence number bit */
};
#define	dh_dstn		dt_vvdst
#define dh_srcn		dt_vvsrc
#define dh_vers		dt_vvvers
#define dh_rsvd
#define dh_lgth		dt_len
#define dh_dsts		dt_dport
#define dh_srcs		dt_sport
#endif DT_THREE


#ifndef STANDALONE
/*
** Ack
** One per destination node.  Contains the ack body, count
** and available flag.
*/
struct	dt_ack {
	u_char						/* ack flags */
		da_avail:1;						/* ack struct available */
	struct dt_head da_head;		/* ack body */
	struct dt_ack *da_next;		/* ack queue next */
};


/*
** Interface control structure
*/
struct	dt_softc {
	u_char						/* flags */
				ds_xerror:1,		/* transmit error */
				ds_rerror:1;		/* receive error */
	u_char		ds_xstate;		/* transmit state */
	u_char		ds_rstate;		/* receive state */
	u_char		ds_sendack;		/* node to send ack to */
	u_long		ds_xaddr;		/* transmit uba map address */
	u_long		ds_raddr;		/* receive uba map address */
	int			ds_ubano;		/* uba number */
	int			ds_host;		/* our node number */
	int			ds_xtries;		/* number of transmit retries */
	struct dt_head *ds_xdh;		/* transmit current header */
	struct dt_head ds_rhead;	/* receive header */
	struct buf	*ds_xbp;		/* transmit current buf */
	struct buf	*ds_rbp;		/* receive current buf */
	struct dt_ack *ds_xda;		/* transmit current ack */
	struct buf	*ds_xnext;		/* transmit queue first pointer */
	struct buf	*ds_xlast;		/* transmit queue last pointer */
	struct dt_ack *ds_anext;	/* ack queue first */
	struct dt_ack *ds_alast;	/* ack queue first */
#ifdef DTFLY
	int			ds_ferrs;		/* # input errors in last tick */
	u_char		ds_fmode;		/* recovery mode */
	u_char		ds_frstop;		/* used to disable input interrupts */
#endif DTFLY
#ifdef DTNOINR
	unsigned	ds_inr;			/* 0 if don't use inr, VII_INR if do */
#endif DTNOINR
};


/*
** Node Info
** One per destination node.  Contains the node transmit queue,
** sequence and ack numbers, need ack flag and active flag.
*/
struct dt_node {
	u_short						/* node info flags */
			dn_active:1,			/* active transmit request */
			dn_queued:1,			/* first packet is on xqueue */
			dn_xdone:1,				/* xdone called while on xqueue */
			dn_xdonerr:1,			/* xdone set and error on call */
			dn_needack:1,			/* expecting an ack on current packet */
			dn_retrans:1,			/* current request is a retransmission */
			dn_sndst:1,				/* send state */
			dn_recst:1,				/* receive state */
			dn_risdead:1,			/* think receive side is dead */
			dn_xisdead:1			/* think transmit side is dead */
#ifdef DTPT
			,dn_trace:1				/* protocol trace */
#endif DTPT
			;
	struct buf	*dn_next;		/* NODE transmit queue, first pointer */
	struct buf	*dn_last;		/* NODE transmit queue, last pointer */
	struct dt_ack dn_ack;		/* node ack structure */
};

/*
** node map
** Bit map of nodes that a given socket can interact with.
** a 1 bit means talk is allowed.
*/
typedef u_char dt_nmap[DT_NNODE/8];

/* node map checking macros */
#define	DTACCESS(m, n)	(m[n/8]&(1<<(n%8)))
#define	DTSETACCESS(m, n)	m[n/8] |= (1<<(n%8))
#define	DTCOPYMAP(f, t)	bcopy((caddr_t)f, (caddr_t)t, sizeof(dt_nmap))

/*
** Socket info
** One per socket.  Socket number is determined by device
** minor number.  Contains the buf and header structures for
** outstanding transmit request, process id of owner, destination
** of current request, available and receive active flags.
**
** The struct dt_sockx is just so the ioctl that pulls socket info
** can get only the interesting stuff, not the buf structures.
** This kludge was necessary because the generic ioctl mechanism
** limits copyouts to 127 bytes.
*/
struct dt_sockx {
	u_char							/* socket flags */
		xdtopen:1,							/* socket is open */
#ifdef DTRW
		xdtrinuse:1,						/* read active */
		/* the timing on this flag is different from the timing on
		 * dt_ractive, so it's not really redundant 
		 */
		xdtwinuse:1,						/* write active */
#else
		xdtactive:1,						/* request (r/w) active */
#endif DTRW
		xdtwanted:1,						/* do wakeup when request done */
		xdtractive:1;						/* receive request active */
		/* dt_ractive tells whether dtrint() can ack the incoming
		 * packet (is there somewhere to put it?)
		 */
	int		xdtreadstop;			/* read stop time in 100ths */
	dt_nmap	xdtnodemap;				/* node permission bit map */
#ifndef DTRWX
	struct	dt_head xdtheadr;		/* current packet header */
#endif DTRWX
};

struct	dt_sock {
	struct	dt_sockx dtx;		/* interesting socket info */
#ifdef DTRWX
	struct	dt_head dtxheadr;		/* current packet header */
	struct	dt_head dtrheadr;		/* current packet header */
#endif DTRWX
#ifdef DTRW
	struct	buf	dt_xbuff;			/* X current packet body (buffer header) */
	struct	buf	dt_rbuff;			/* R current packet body (buffer header) */
#else
	struct	buf	dt_buff;			/* current packet body (buffer header) */
#endif DTRW
};

#define dt_open dtx.xdtopen
#define dt_rinuse dtx.xdtrinuse
#define dt_winuse dtx.xdtwinuse
#define dt_active dtx.xdtactive
#define dt_wanted dtx.xdtwanted
#define dt_ractive dtx.xdtractive
#define dt_readstop dtx.xdtreadstop
#define dt_nodemap dtx.xdtnodemap
#ifdef DTRWX
#define dt_headr dtrheadr
#define dt_headx dtxheadr
#else
#define dt_headr dtx.xdtheadr
#endif DTRWX


/* ioctl parameters */
struct dt_param {
	u_char							/* flags */
				dp_debug:1				/* debug interface */
				,dp_trace:7			/* protocol trace */
				;
	int			dp_xretries;		/* times to retry sending */
	int			dp_retrans;			/* retransmission timeout in 100ths */
	int			dp_deadsend;		/* max time retransmitting in 100ths */
	int			dp_readstop;		/* time before stop read in 100ths */
	int			dp_rdead;			/* receive dead timeout in 100ths */
	int			dp_xdead;			/* transmit dead timeout in 100ths */
	int			dp_piggy;			/* wait before sending bare ack */
#ifdef DTFLY
	int			dp_flythresh;		/* threshhold for errors */
#endif DTFLY
	dt_nmap		dp_nodemap;			/* node permission bit map */
};

/* dt statistics */
struct dt_stat {
	u_short		dd_ubarst;			/* uba resets */
	u_short		dd_rerrs;			/* input interrupt errors */
	u_short		dd_rhead;			/* read header state */
	u_short		dd_rbadhd;			/* bad headers read */
	u_short		dd_rbadbd;			/* bad bodies read */
	u_short		dd_rdone;			/* read done state */
	u_short		dd_rstart;			/* read start state */
	u_short		dd_xretry;			/* transmit collision retries */
	u_short		dd_xerrs;			/* transmit errors */
	u_short		dd_xstart;			/* transmit start state */
	/* 20 */
	u_short		dd_xhead;			/* transmit header state */
	u_short		dd_xack;			/* transmit ack state */
	u_short		dd_xadone;			/* transmit ack done */
	u_short		dd_xbdone;			/* transmit body done */
	u_short		dd_xnoq;			/* packets sent without xmit queue */
	u_short		dd_xanoq;			/* acks sent without xmit queue */
	u_short		dd_write;			/* write system calls */
	u_short		dd_writebad;		/* write error returns */
	u_short		dd_read;			/* read system calls */
	u_short		dd_readbad;			/* read error returns */
	/* 40 */
	u_short		dd_badhead;			/* bad headers received */
	u_short		dd_noread;			/* no reads */
	u_short		dd_goodack;			/* good acks */
	u_short		dd_badack;			/* bad acks */
	u_short		dd_rpigs;			/* piggy back acks received */
	u_short		dd_xpigs;			/* piggy back acks sent */
	u_short		dd_rdgm;			/* datagrams received */
	u_short		dd_xdgm;			/* datagrams sent */
	u_short		dd_rinval;			/* invalid acks recieved */
	u_short		dd_xinval;			/* invalid acks sent */
	/* 60 */
	u_short		dd_rejects;			/* rejected packets */
	u_short		dd_deadsend;		/* dead send timeouts */
	u_short		dd_livesend;		/* live send timeouts */
	u_short		dd_deadrecv;		/* dead receive timeouts */
	u_short		dd_retrans;			/* retransmission timeouts */
	u_short		dd_readstop;		/* read stop timeouts */
	u_short		dd_unrdstop;		/* read stop undos */
	u_short		dd_xrfs;			/* xmit RFS error */
	u_short		dd_xovr;			/* xmit OVR error */
	u_short		dd_xnok;			/* xmit NOK error */
	/* 80 */
	u_short		dd_xbdf;			/* xmit BDF error */
	u_short		dd_rovr;			/* recv OVR error */
	u_short		dd_rodb;			/* recv ODB error */
	u_short		dd_rlde;			/* recv LDE error */
	u_short		dd_rnok;			/* recv NOK error */
	u_short		dd_rbdf;			/* recv BDF error */
	u_short		dd_rhdshrt;			/* UNUSED */
	/*  94 */
#ifdef DTFLY
	u_short		dd_fly;				/* # calls to dtfly */
	u_short		dd_flyinit;			/* # time dtfly initialized the if */
	u_short		dd_flyhiwat;		/* high water mark for ds_ferrs */
	u_short		dd_flybad;		/* high water mark for ds_ferrs */
#endif DTFLY
#ifdef DTRUNTCK
	u_short		dd_runt;		/* runt packets */
#endif DTRUNTCK
#ifdef DTNOINR
	u_short		dd_xcoll;			/* collisions */
	u_short		dd_xinr;			/* times INR used */
	u_short		dd_xnxm;			/* xmit nxm errs */
	u_short		dd_xopt;			/* xmit output timeout errs */
#endif DTNOINR
};

#ifdef KERNEL
struct	dt_sock	Sockets[DT_NSOCK];		/* Socket info table */
struct	dt_node	Nodes[DT_NNODE];		/* Node info, one per node */
struct	dt_softc dtsoftc;
#ifdef DTSTATS
struct	dt_stat dtstat;					/* statistics */
struct	dt_stat dtstatinit;				/* initialize statistics */
#endif DTSTATS
struct	dt_param dtparam = {			/* ioctl parameters */
	0,				/* dp_debug */
	0,				/* dp_trace */
	20,				/* dp_xretries */
	3,				/* dp_retrans */
	360,			/* dp_deadsend - this plus dp_xdead == Stimer in spec */
	1200,			/* dp_readstop */
	720,			/* dp_rdead */
	360,			/* dp_xdead */
	0				/* dp_piggy */
#ifdef DTFLY
	,30				/* dp_flythresh */
#endif DTFLY

	};

#ifdef DTHANG
struct dtcmap {
	caddr_t dtc_base;
	int		dtc_count;
};
#endif DTHANG

#endif KERNEL
#endif STANDALONE
