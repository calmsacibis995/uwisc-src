/*
**	dtio.h
*/

/* STRUCTURES COPIED OUT MAY NOT BE BIGGER THAN 127 BYTES!! */

/* unprotected commands */
/* get statistics */
#define	DTIOSGET	_IOR(d, 0, dtstat)
/* get socket table */
#define	DTIOKGET	_IOWR(d, 1, struct dt_sockx)
/* get parameters */
#define	DTIOPGET	_IOR(d, 2, dtparam)
/* get read timeout */
#define	DTIORGET	_IOR(d, 3, int)
/* set read timeout */
#define	DTIORSET	_IOW(d, 4, int)
/* get host number */
#define	DTIOHGET	_IOR(d, 5, int)

/* protected commands */
/* get node map */
#define	DTIOMGET	_IOWR(d, 6, struct dt_nmio)
/* set node map */
#define	DTIOMSET	_IOW(d, 7, struct dt_nmio)
/* clear partition from all nodes */
#define	DTIOMCLR	_IOW(d, 8, struct dt_nmio)
/* initialize statistics */
#define	DTIOSSET	_IO(d, 9)
/* set parameters */
#define	DTIOPSET	_IOW(d, 10, dtparam)
/* reinitialize interface */
#define DTRESET		_IO(d,11)
/* turn trace on/off */
#define DTTRACEON		_IOW(d,12,int)
#define DTTRACEOFF		_IOW(d,13,int)


/* monitor on/off */
#define DTIOMON	_IOW(d,14,int)

/*
** struct for getting and setting node maps
*/
struct dt_nmio {
	u_short	di_socket;		/* socket number */
	dt_nmap	di_nodemap;		/* node permission bit map */
};
