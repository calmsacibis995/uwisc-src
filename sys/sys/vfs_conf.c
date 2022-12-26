/*	@(#)vfs_conf.c 1.1 86/02/03 SMI	*/
/*	@(#)vfs_conf.c	2.1 86/04/15 NFSSRC */

#include "param.h"
#include "vfs.h"

extern	struct vfsops ufs_vfsops;	/* XXX Should be ifdefed */

#ifdef NFS
extern	struct vfsops nfs_vfsops;
#endif

#ifdef PCFS
extern	struct vfsops pcfs_vfsops;
#endif

struct vfsops *vfssw[] = {
	&ufs_vfsops,		/* 0 = MOUNT_UFS */
#ifdef NFS
	&nfs_vfsops,		/* 1 = MOUNT_NFS */
#else
	(struct vfsops *)0,
#endif
#ifdef PCFS
	&pcfs_vfsops,		/* 2 = MOUNT_PC */
#else
	(struct vfsops *)0,
#endif
};
