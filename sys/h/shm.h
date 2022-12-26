/* @(#)shm.h	1.2 */
#ifndef _SHM_H_
#define _SHM_H_
/*
**	IPC Shared Memory Facility.
*/
#ifndef _IPC_H_
#	ifdef KERNEL
#		include "../h/ipc.h"
#	else
#		include <sys/ipc.h>
#	endif
#endif

/*
**	Permission Definitions.
*/

#define	SHM_R	0400	/* read permission */
#define	SHM_W	0200	/* write permission */

/*
**	ipc_perm Mode Definitions.
*/

#define	SHM_CLEAR	01000	/* clear segment on next attach */
#define	SHM_DEST	02000	/* destroy segment when # attached = 0 */

/*
**	Message Operation Flags.
*/

#define	SHM_RDONLY	010000	/* attach read-only (else read-write) */
#define	SHM_RND		020000	/* round attach address to SHMLBA */

/*
**	Structure Definitions.
*/

/*
**	There is a shared mem id data structure for each segment in the system.
*/

struct shmid_ds {
	struct ipc_perm	shm_perm;	/* operation permission struct */
	int		shm_segsz;	/* segment size (pages)*/
	struct pte	*shm_ptbl;	/* ptr to associated page table */
	u_short		shm_lpid;	/* pid of last shmop */
	u_short		shm_cpid;	/* pid of creator */
	u_short		shm_nattch;	/* current # attached */
	time_t		shm_atime;	/* last shmat time */
	time_t		shm_dtime;	/* last shmdt time */
	time_t		shm_ctime;	/* last change time */
};

struct	shminfo {
	int	shmmap,	/* size of shared memory pte resource map */
		shmmax,	/* max shared memory segment size */
		shmmin,	/* min shared memory segment size */
		shmmni,	/* # of shared memory identifiers */
		shmseg,	/* max attached shared memory segments per process */
		shmall	/* max total shared memory system wide (in pages) */
};

/*
**	Implementation Constants.
*/

#define	SHMLBA	(NBPG*CLSIZE)	/* segment low boundary address multiple */
			/* (SHMLBA must be a power of 2) */

/*
 * SHMMAXPGS defines the total size (in pages) of the shared
 * memory address space.
 * Note that this is a global resource and establishes an upper 
 * bound on the total amount of shared memory address space in the
 * system.
 */
#define SHMMAXPGS	128

/*
 * The range of addresses used for shared memory are defined by
 * SHMEMBASE and SHMLIMIT-1. 
 */
#define SHMEMBASE	((caddr_t)(0x7F000000 - (int)ptob(shminfo.shmall)))

#ifdef KERNEL
#define SHMPAGES	(shminfo.shmall)
#else
#define SHMPAGES	SHMMAXPGS
#endif

#define SHMEMLIMIT	((caddr_t)(SHMEMBASE + (int)(ptob(SHMPAGES))))

#ifdef KERNEL

#endif

#endif
