/*	@(#)vfs_dev.c 1.1 86/02/03 SMI	*/
/*      NFSSRC @(#)vfs_dev.c	2.1 86/04/15 */

#include "param.h"
#include "systm.h"
#include "time.h"
#include "conf.h"
#include "buf.h"
#include "vfs.h"
#include "vnode.h"

/*
 * Convert a dev into a vnode pointer suitable for bio.
 */

struct dev_vnode {
	struct vnode dv_vnode;
	struct dev_vnode *dv_link;
} *dev_vnode_headp;

devtovp_badop()
{
	panic("devtovp_badop");
}

/*ARGSUSED*/
int
devtovp_inactive(vp)
	struct vnode *vp;
{
	/* could free the vnode here */
	return(0);
}

int
devtovp_strategy(bp)
	struct buf *bp;
{

	(*bdevsw[major(bp->b_vp->v_rdev)].d_strategy)(bp);
	return(0);
}

struct vnodeops dev_vnode_ops = {
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_badop,
	devtovp_inactive,
	devtovp_badop,
	devtovp_strategy
};

struct vnode *
devtovp(dev)
	dev_t dev;
{
	register struct dev_vnode *dvp;
	register struct dev_vnode *endvp;

	endvp = (struct dev_vnode *)0;
	for (dvp = dev_vnode_headp; dvp; dvp = dvp->dv_link) {
		if (dvp->dv_vnode.v_rdev == dev) {
			VN_HOLD(&dvp->dv_vnode);
			return (&dvp->dv_vnode);
		}
		endvp = dvp;
	}
	dvp = (struct dev_vnode *)
		kmem_alloc((u_int)sizeof (struct dev_vnode));
	bzero((caddr_t)dvp, sizeof(struct dev_vnode));
	dvp->dv_vnode.v_count = 1;
	dvp->dv_vnode.v_op = &dev_vnode_ops;
	dvp->dv_vnode.v_rdev = dev;
	if (endvp != (struct dev_vnode *)0) {
		endvp->dv_link = dvp;
	} else {
		dev_vnode_headp = dvp;
	}
	return (&dvp->dv_vnode);
}
