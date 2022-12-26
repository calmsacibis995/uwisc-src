/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vm_text.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: vm_text.c,v 3.1 86/10/22 13:49:17 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */

#include "../machine/pte.h"

#include "param.h"
#include "systm.h"
#include "map.h"
#include "user.h"
#include "proc.h"
#include "text.h"
#include "buf.h"
#include "seg.h"
#include "vm.h"
#include "cmap.h"
#include "uio.h"
#include "exec.h"
#include "vfs.h"
#include "vnode.h"

#define X_LOCK(xp) { \
	while ((xp)->x_flag & XLOCK) { \
		(xp)->x_flag |= XWANT; \
		sleep((caddr_t)(xp), PSWP); \
	} \
	(xp)->x_flag |= XLOCK; \
}
#define	XUNLOCK(xp) { \
	if ((xp)->x_flag & XWANT) \
		wakeup((caddr_t)(xp)); \
	(xp)->x_flag &= ~(XLOCK|XWANT); \
}
#define FREE_AT_HEAD(xp) { \
	(xp)->x_forw = xhead; \
	xhead = (xp); \
	(xp)->x_back = &xhead; \
	if (xtail == &xhead) \
		xtail = &(xp)->x_forw; \
	else \
		(xp)->x_forw->x_back = &(xp)->x_forw; \
}
#define FREE_AT_TAIL(xp) { \
	(xp)->x_back = xtail; \
	*xtail = (xp); \
	xtail = &(xp)->x_forw; \
	/* x_forw is NULL */ \
}
#define	ALLOC(xp) { \
	*((xp)->x_back) = (xp)->x_forw; \
	if ((xp)->x_forw) \
		(xp)->x_forw->x_back = (xp)->x_back; \
	else \
		xtail = (xp)->x_back; \
	(xp)->x_forw = NULL; \
	(xp)->x_back = NULL; \
}

/*
 * We place free text table entries on a free list.
 * All text images are treated as "sticky,"
 * and are placed on the free list (as an LRU cache) when unused.
 * They may be reclaimed from the free list until reused.
 * Files marked sticky are locked into the table, and are never freed.
 * For machines with limited swap space, this may result
 * in filling up swap, and thus we allow a limit
 * to be placed on the number of text images to cache.
 * (In that case, really should change the algorithm
 * for freeing a text when the cache is full;
 * should free least-recently-used text rather than current one.)
 */
struct	text *xhead, **xtail;		/* text table free list */
int	xcache;				/* number of "sticky" texts retained */
int	maxtextcache = -1;		/* maximum number of "sticky" texts */
struct	xstats xstats;			/* cache statistics */

/*
 * initialize text table
 */
xinit()
{
	register struct text *xp;

	xtail = &xhead;
	for (xp = text; xp < textNTEXT; xp++)
		FREE_AT_TAIL(xp);
	if (maxtextcache == -1)
		maxtextcache = ntext;
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct vnode *vp;
	struct vattr vattr;

	if ((xp = u.u_procp->p_textp) == NULL)
		return;
	xstats.free++;
	X_LOCK(xp);
	vp = xp->x_vptr;
	VOP_GETATTR(vp, &vattr, u.u_cred);
	if (--xp->x_count == 0 && (vattr.va_mode & VSVTX) == 0) {
		if (xcache >= maxtextcache || xp->x_flag & XTRC ||
		    vattr.va_nlink == 0) {			/* XXX */
			xp->x_rssize -= vmemfree(tptopte(u.u_procp, 0), (int)u.u_tsize);
			if (xp->x_rssize != 0)
				panic("xfree rssize");
			while (xp->x_poip)
				sleep((caddr_t)&xp->x_poip, PSWP+1);
			xp->x_flag &= ~XLOCK;
			xuntext(xp);
			FREE_AT_HEAD(xp);
		} else {
			if (xp->x_flag & XWRIT) {
				xstats.free_cacheswap++;
				xp->x_flag |= XUNUSED;
			}
			xcache++;
			xstats.free_cache++;
			xccdec(xp, u.u_procp);
			FREE_AT_TAIL(xp);
		}
	} else {
		xccdec(xp, u.u_procp);
		xstats.free_inuse++;
	}
	xunlink(u.u_procp);
	XUNLOCK(xp);
	u.u_procp->p_textp = NULL;
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the vnode (vp); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */
xalloc(vp, ep, pagi)
	register struct vnode *vp;
	struct exec *ep;
{
	register struct text *xp;
	register size_t ts;

	if (ep->a_text == 0)
		return;
	xstats.alloc++;
	while ((xp = vp->v_text) != NULL) {
		if (xp->x_flag&XLOCK) {
			/*
			 * Wait for text to be unlocked,
			 * then start over (may have changed state).
			 */
			xwait(xp);
			continue;
		}
		X_LOCK(xp);
		if (xp->x_back) {
			xstats.alloc_cachehit++;
			ALLOC(xp);
			xp->x_flag &= ~XUNUSED;
			xcache--;
		} else
			xstats.alloc_inuse++;
		xp->x_count++;
		u.u_procp->p_textp = xp;
		xlink(u.u_procp);
		XUNLOCK(xp);
		return;
	}
	xp = xhead;
	if (xp == NULL) {
		tablefull("text");
		psignal(u.u_procp, SIGKILL);
		return;
	}
	ALLOC(xp);
	if (xp->x_vptr) {
		xstats.alloc_cacheflush++;
		if (xp->x_flag & XUNUSED)
			xstats.alloc_unused++;
		xuntext(xp);
		xcache--;
	}
	xp->x_flag = XLOAD|XLOCK;
	if (pagi)
		xp->x_flag |= XPAGV;
	ts = clrnd(btoc(ep->a_text));
	xp->x_size = ts;
	if (vsxalloc(xp) == NULL) {
		swkill(u.u_procp, "xalloc: no swap space");
		return;
	}
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_rssize = 0;
	xp->x_vptr = vp;
	vp->v_flag |= VTEXT;
	vp->v_text = xp;
	VN_HOLD(vp);
	u.u_procp->p_textp = xp;
	xlink(u.u_procp);
	if (pagi == 0) {
		settprot(RW);
		u.u_procp->p_flag |= SKEEP;
		(void) vn_rdwr(UIO_READ, vp,
			(caddr_t)ctob(tptov(u.u_procp, 0)),
			(int)ep->a_text, 
			(off_t)(ep->a_magic == 0413?
				CLBYTES:
				sizeof (struct exec)),
			UIO_USERSPACE, IO_UNIT, (int *)0);
		u.u_procp->p_flag &= ~SKEEP;
	}
	settprot(RO);
	xp->x_flag |= XWRIT;
	xp->x_flag &= ~XLOAD;
	XUNLOCK(xp);
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(xp)
	register struct text *xp;
{

	X_LOCK(xp);
}

/*
 * Wait for xp to be unlocked if it is currently locked.
 */
xwait(xp)
register struct text *xp;
{

	X_LOCK(xp);
	XUNLOCK(xp);
}

xunlock(xp)
register struct text *xp;
{

	XUNLOCK(xp);
}

/*
 * Decrement the in-core usage count of a shared text segment,
 * which must be locked.  When the count drops to zero,
 * free the core space.
 */
xccdec(xp, p)
	register struct text *xp;
	register struct proc *p;
{

	if (--xp->x_ccount == 0) {
		if (xp->x_flag & XWRIT) {
			vsswap(p, tptopte(p, 0), CTEXT, 0, (int)xp->x_size,
			    (struct dmap *)0);
			if (xp->x_flag & XPAGV)
				(void)swap(p, xp->x_ptdaddr,
				    (caddr_t)tptopte(p, 0),
				    (int)xp->x_size * sizeof (struct pte),
				    B_WRITE, B_PAGET, swapdev_vp, 0);
			xp->x_flag &= ~XWRIT;
		} else
			xp->x_rssize -= vmemfree(tptopte(p, 0),
			    (int)xp->x_size);
		if (xp->x_rssize != 0)
			panic("text rssize");
	}
}

/*
 * Detach a process from the in-core text.
 * External interface to xccdec, used when swapping out a process.
 */
xdetach(xp, p)
	register struct text *xp;
	struct proc *p;
{

	if (xp && xp->x_ccount != 0) {
		X_LOCK(xp);
		xccdec(xp, p);
		xunlink(p);
		XUNLOCK(xp);
	}
}

/*
 * Free the swap image of all unused saved-text text segments
 * which are from virtual filesystem vfsp (used by umount system call).
 */
xumount(vfsp)
	register struct vfs *vfsp;
{
	register struct text *xp;

	for (xp = text; xp < textNTEXT; xp++) 
		if (xp->x_vptr != NULL && (xp->x_vptr->v_vfsp == vfsp))
			xuntext(xp);
	mpurgevfs(vfsp);
}

/*
 * remove a shared text segment from the text table, if possible.
 */
xrele(vp)
	register struct vnode *vp;
{

	if (vp->v_flag & VTEXT)
		xuntext(vp->v_text);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
	register struct text *xp;
{
	register struct vnode *vp;

	X_LOCK(xp);
	if (xp->x_count == 0) {
		vp = xp->x_vptr;
		xp->x_vptr = (struct vnode *)0;
		vsxfree(xp, (long)xp->x_size);
		vp->v_flag &= ~(VTEXT|VTEXTMOD);
		vp->v_text = NULL;
		VN_RELE(vp);
	}
	XUNLOCK(xp);
}

/*
 * Add a process to those sharing a text segment by
 * getting the page tables and then linking to x_caddr.
 */
xlink(p)
	register struct proc *p;
{
	register struct text *xp = p->p_textp;

	if (xp == 0)
		return;
	vinitpt(p);
	p->p_xlink = xp->x_caddr;
	xp->x_caddr = p;
	xp->x_ccount++;
}

xunlink(p)
	register struct proc *p;
{
	register struct text *xp = p->p_textp;
	register struct proc *q;

	if (xp == 0)
		return;
	if (xp->x_caddr == p) {
		xp->x_caddr = p->p_xlink;
		p->p_xlink = 0;
		return;
	}
	for (q = xp->x_caddr; q->p_xlink; q = q->p_xlink)
		if (q->p_xlink == p) {
			q->p_xlink = p->p_xlink;
			p->p_xlink = 0;
			return;
		}
	panic("lost text");
}

/*
 * Replace p by q in a text incore linked list.
 * Used by vfork(), internally.
 */
xrepl(p, q)
	struct proc *p, *q;
{
	register struct text *xp = q->p_textp;

	if (xp == 0)
		return;
	xunlink(p);
	q->p_xlink = xp->x_caddr;
	xp->x_caddr = q;
}

int xkillcnt = 0;

/*
 * Invalidate the text associated with vp.
 * Purge in core cache of pages associated with vp and kill all active
 * processes.
 */
xinval(vp)
	register struct vnode *vp;
{
	register struct text *xp;
	register struct proc *p;

	mpurge(vp);
	for (xp = text; xp < textNTEXT; xp++) {
		if ((xp->x_flag & XPAGV) && (xp->x_vptr == vp)) {
			for (p = xp->x_caddr; p; p = p->p_xlink) {
				/*
				 * swkill without uprintf
				 */
				printf(
				   "pid %d killed due to text modification\n",
				   p->p_pid);
				psignal(p, SIGKILL);
				p->p_flag |= SULOCK;
				xkillcnt++;
			}
			break;
		}
	}
}
