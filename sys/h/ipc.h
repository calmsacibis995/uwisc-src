/* @(#)ipc.h	1.1 */
#ifndef _IPC_H_
#define _IPC_H_
/* Common IPC Access Structure */
struct ipc_perm {
	u_short	uid;	/* owner's user id */
	u_short	gid;	/* owner's group id */
	u_short	cuid;	/* creator's user id */
	u_short	cgid;	/* creator's group id */
	u_short	mode;	/* access modes */
	u_short	seq;	/* slot usage sequence number */
	key_t	key;	/* key */
};

/* Common IPC Definitions. */
/* Mode bits. */
#define	IPC_ALLOC	0100000		/* entry currently allocated */
#define	IPC_CREAT	0001000		/* create entry if key doesn't exist */
#define	IPC_EXCL	0002000		/* fail if key exists */
#define	IPC_NOWAIT	0004000		/* error if request must wait */

/* Keys. */
#define	IPC_PRIVATE	(key_t)0	/* private key */

/* Control Commands. */
#define	IPC_RMID	0	/* remove identifier */
#define	IPC_SET		1	/* set options */
#define	IPC_STAT	2	/* get options */

#endif
