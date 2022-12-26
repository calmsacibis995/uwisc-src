/*	@(#)des.h 1.1 85/05/30 SMI	*/

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

/*
 * Generic DES driver interface
 * Keep this file hardware independent!
 */
struct	deskey {
	u_char	des_key[8];			/* key (with low bit parity) */
	enum { ENCRYPT, DECRYPT} des_dir;	/* direction */
	enum { ECB, CBC }	des_mode;	/* mode */
};

/*
 * Initial vector for CBC mode
 */
struct	desivec {
	u_char	des_ivec[8];			/* initial vector */
};

/*
 * Data block descriptor
 */
struct desblock {
	char	*des_data;			/* data address */
	int	des_len;			/* data length (mult of 8) */
};

typedef	char deschunk_t[8];

/*
 * Setting modes also sets the initial vector to zero
 */
#define	DESIOCSETKEY	_IOW(d, 0, struct deskey)	/* set des mode */
#define	DESIOCGETKEY	_IOR(d, 1, struct deskey)	/* get des mode */
#define	DESIOCSETIVEC	_IOW(d, 2, struct desivec)	/* set init vector */
#define	DESIOCGETIVEC	_IOR(d, 3, struct desivec)	/* get init vector */
#define	DESIOCBLOCK	_IOW(d, 4, struct desblock)	/* process block */
#define	DESIOCCHUNK	_IOWR(d, 5, deschunk_t)		/* process 8 bytes */
