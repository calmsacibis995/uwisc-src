/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uipc_mbuf.c	7.1 (Berkeley) 6/5/86
 */
#ifndef lint
static char rcs_id[] = {"$Header: uipc_mbuf.c,v 3.1 86/10/22 13:44:58 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */

#include "../machine/pte.h"

#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "cmap.h"
#include "map.h"
#include "mbuf.h"
#include "vm.h"
#include "kernel.h"
#include "socketvar.h"

mbinit()
{
	int s;

	s = splimp();
	if (m_clalloc(4096/CLBYTES, MPG_MBUFS, M_DONTWAIT) == 0)
		goto bad;
	if (m_clalloc(4096/CLBYTES, MPG_CLUSTERS, M_DONTWAIT) == 0)
		goto bad;
	splx(s);
	return;
bad:
	panic("mbinit");
}

/*
 * Must be called at splimp.
 */
caddr_t
m_clalloc(ncl, how, canwait)
	register int ncl;
	int how;
{
	int npg, mbx;
	register struct mbuf *m;
	register int i;

	npg = ncl * CLSIZE;
	mbx = rmalloc(mbmap, (long)npg);
	if (mbx == 0) {
		if (canwait == M_WAIT)
			panic("out of mbufs: map full");
		return (0);
	}
	m = cltom(mbx / CLSIZE);
	if (memall(&Mbmap[mbx], npg, proc, CSYS) == 0) {
		rmfree(mbmap, (long)npg, (long)mbx);
		return (0);
	}
	vmaccess(&Mbmap[mbx], (caddr_t)m, npg);
	switch (how) {

	case MPG_CLUSTERS:
		for (i = 0; i < ncl; i++) {
			m->m_off = 0;
			m->m_next = mclfree;
			mclfree = m;
			m += CLBYTES / sizeof (*m);
			mbstat.m_clfree++;
		}
		mbstat.m_clusters += ncl;
		break;

	case MPG_MBUFS:
		for (i = ncl * CLBYTES / sizeof (*m); i > 0; i--) {
			m->m_off = 0;
			m->m_type = MT_DATA;
			mbstat.m_mtypes[MT_DATA]++;
			mbstat.m_mbufs++;
			(void) m_free(m);
			m++;
		}
		break;
	}
	return ((caddr_t)m);
}

m_pgfree(addr, n)
	caddr_t addr;
	int n;
{

#ifdef lint
	addr = addr; n = n;
#endif
}

/*
 * Must be called at splimp.
 */
m_expand(canwait)
	int canwait;
{

	if (m_clalloc(1, MPG_MBUFS, canwait) == 0)
		goto steal;
	return (1);
steal:
	/* should ask protocols to free code */
	return (0);
}

/* NEED SOME WAY TO RELEASE SPACE */

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	if (m == 0)
		return (0);
	bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

struct mbuf *
m_free(m)
	struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n);
	return (n);
}

/*
 * Get more mbufs; called from MGET macro if mfree list is empty.
 * Must be called at splimp.
 */
/*ARGSUSED*/
struct mbuf *
m_more(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	while (m_expand(canwait) == 0) {
		if (canwait == M_WAIT) {
			m_want++;
			sleep((caddr_t)&mfree, PZERO - 1);
		} else {
			mbstat.m_drops++;
			return (NULL);
		}
	}
#define m_more(x,y) (panic("m_more"), (struct mbuf *)0)
	MGET(m, canwait, type);
#undef m_more
	return (m);
}

m_freem(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	register int s;

	if (m == NULL)
		return;
	s = splimp();
	do {
		MFREE(m, n);
	} while (m = n);
	splx(s);
}

/*
 * Mbuffer utility routines.
 */

/*
 * Make a copy of an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * Should get M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy");
	while (off > 0) {
		if (m == 0)
			panic("m_copy");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy");
			break;
		}
		MGET(n, M_DONTWAIT, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_off > MMAXOFF && n->m_len > MLEN) {
			mcldup(m, n, off);
			n->m_off += off;
		} else
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);
nospace:
	m_freem(top);
	return (0);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m;
	register count;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculating its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == (struct mbuf *)0)
				break;
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			return;
		}
		count -= len;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		for (m = mp; m; m = m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m = m->m_next)
			m->m_len = 0;
	}
}

/*
 * Rearange an mbuf chain so that len bytes are contiguous
 * and in the data area of an mbuf (so that mtod and dtom
 * will work for a structure of size len).  Returns the resulting
 * mbuf chain on success, frees it and returns null on failure.
 * If there is room, it will add up to MPULL_EXTRA bytes to the
 * contiguous region in an attempt to avoid being called next time.
 */
struct mbuf *
m_pullup(n, len)
	register struct mbuf *n;
	int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	if (n->m_off + len <= MMAXOFF && n->m_next) {
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if (len > MLEN)
			goto bad;
		MGET(m, M_DONTWAIT, n->m_type);
		if (m == 0)
			goto bad;
		m->m_len = 0;
	}
	space = MMAXOFF - m->m_off;
	do {
		count = MIN(MIN(space - m->m_len, len + MPULL_EXTRA), n->m_len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len,
		  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		if (n->m_len)
			n->m_off += count;
		else
			n = m_free(n);
	} while (len > 0 && n);
	if (len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

/*
 * Copy an mbuf to the contiguous area pointed to by cp.
 * Skip <off> bytes and copy <len> bytes.
 * Returns the number of bytes not transferred.
 * The mbuf is NOT changed.
 */
int
m_cpytoc(m, off, len, cp)
	register struct mbuf *m;
	register int off, len;
	register caddr_t cp;
{
	register int ml;

	if (m == NULL || off < 0 || len < 0 || cp == NULL)
		panic("m_cpytoc");
	while (off && m)
		if (m->m_len <= off) {
			off -= m->m_len;
			m = m->m_next;
			continue;
		} else
			break;
	if (m == NULL)
		return (len);

	ml = imin(len, m->m_len - off);
	bcopy(mtod(m, caddr_t)+off, cp, (u_int)ml);
	cp += ml;
	len -= ml;
	m = m->m_next;

	while (len && m) {
		ml = m->m_len;
		bcopy(mtod(m, caddr_t), cp, (u_int)ml);
		cp += ml;
		len -= ml;
		m = m->m_next;
	}

	return (len);
}

/*
 * Append a contiguous hunk of memory to a particular mbuf;
 * that is, it does not follow the mbuf chain.
 * XXX It should return 1 if it won't fit.  Maybe someday.
 */
int
m_cappend(cp, len, m)
	char *cp;
	register int len;
	register struct mbuf *m;
{
	bcopy(cp, mtod(m, char *) + m->m_len, (u_int)len);
	m->m_len += len;
	return (0);
}

/*
 * Tally the bytes used in an mbuf chain.
 * op: -1 == subtract; 0 == assign; 1 == add;
 */
m_tally(m, op, cc, mbcnt)
	register struct mbuf *m;
	int op, *cc, *mbcnt;
{
	struct sockbuf sockbuf;
	register struct sockbuf *sb = &sockbuf;

	bzero((caddr_t)sb, sizeof(*sb));
	while (m) {
		sballoc(sb, m);
		m = m->m_next;
	}
	if (cc)
		switch (op) {
		case -1:
			*cc -= sb->sb_cc;
			break;
		case 0:
			*cc  = sb->sb_cc;
			break;
		case 1:
			*cc += sb->sb_cc;
			break;
		}
	if (mbcnt)
		switch (op) {
		case -1:
			*mbcnt -= sb->sb_mbcnt;
			break;
		case 0:
			*mbcnt  = sb->sb_mbcnt;
			break;
		case 1:
			*mbcnt += sb->sb_mbcnt;
			break;
		}
}

#ifdef notdef

/*
 * Given an mbuf, allocate and attach a cluster mbuf to it.
 * Return 1 if successful, 0 otherwise.
 * NOTE: m->m_len is set to CLBYTES!
 */
mclget(m)
	register struct mbuf *m;
{
	int ms;
	register struct mbuf *p;

	ms = splimp();
	if (mclfree == 0)
		/* XXX need a way to  reclaim */
		(void) m_clalloc(1, MPG_CLUSTERS);
	if (p = mclfree) {
		++mclrefcnt[mtocl(p)];
		mbstat.m_clfree--;
		mclfree = p->m_next;
		m->m_len = CLBYTES;
		m->m_off = (int)p - (int)m;
		m->m_cltype = 1;
	}
	(void) splx(ms);
	return (p ? 1 : 0);
}
#endif notdef

/* Allocate a "funny" mbuf, that is, one whose data is owned by someone else */
struct mbuf *
mclgetx(fun, arg, addr, len, wait)
	int (*fun)(), arg, len, wait;
	caddr_t addr;
{
	register struct mbuf *m;

	MGET(m, wait, MT_DATA);
	if (m == 0)
		return (0);
	m->m_off = (int)addr - (int)m;
	m->m_len = len;
	m->m_cltype = 2;
	m->m_clfun = fun;
	m->m_clarg = arg;
	m->m_clswp = NULL;
	return (m);
}

/* Generic cluster mbuf unallocator -- invoked from within MFREE */
mclput(m)
	register struct mbuf *m;
{

	switch (m->m_cltype) {
	case 1:
		m = (struct mbuf *)(mtod(m, int) & ~(CLBYTES - 1));
		if (--mclrefcnt[mtocl(m)] == 0) {
			m->m_next = mclfree;
			mclfree = m;
			mbstat.m_clfree++;
		}
		break;

	case 2:
		(*m->m_clfun)(m->m_clarg);
		break;

	default:
		panic("mclput");
	}
}

/* Internal routine used for mcldup on funny mbufs */
static
buffree(arg)
	int arg;
{
	extern int kmem_free_intr();

	kmem_free_intr((caddr_t)arg, *(u_int *)arg);
}

/*
 * Generic cluster mbuf duplicator
 * which duplicates <m> into <n>.
 * If <m> is a regular cluster mbuf, mcldup simply
 * bumps the reference count and ignores <off>.
 * If <m> is a funny mbuf, mcldup allocates a chunck
 * kernel memory and makes a copy, starting at <off>.
 * XXX does not set the m_len field in <n>!
 */
mcldup(m, n, off)
	register struct mbuf *m, *n;
	int off;
{
	register struct mbuf *p;
	register caddr_t copybuf;

	switch (m->m_cltype) {
	case 1:
		p = mtod(m, struct mbuf *);
		n->m_off = (int)p - (int)n;
		n->m_cltype = 1;
		mclrefcnt[mtocl(p)]++;
		break;
	case 2:
		copybuf = (caddr_t)kmem_alloc((u_int)(n->m_len + sizeof(int)));
		* (int *) copybuf = n->m_len + sizeof (int);
		bcopy(mtod(m, caddr_t) + off, copybuf + sizeof (int),
		    (unsigned)n->m_len);
		n->m_off = (int)copybuf + sizeof (int) - (int)n - off;
		n->m_cltype = 2;
		n->m_clfun = buffree;
		n->m_clarg = (int)copybuf;
		n->m_clswp = NULL;
		break;
	default:
		panic("mcldup");
	}
}

/*
 * Move an mbuf chain to contiguous locations.
 * Checks for possibility of page exchange to accomplish move.
 * Free chain when moved.
 */
m_movtoc(m, to, count)
	register struct mbuf *m;
	register caddr_t to;
	register int count;
{
	register struct mbuf *m0;
	register caddr_t from;
	register int i;

	while (m != NULL) {
		i = MIN(m->m_len, count);
		from = mtod(m, caddr_t);
		if (i >= CLBYTES && m->m_cltype == 2 && m->m_clswp &&
		    (((int)from | (int)to) & (CLBYTES-1)) == 0 &&
		    (*m->m_clswp)(m->m_clarg, from, to)) {
			i -= CLBYTES;
			from += CLBYTES;
			to += CLBYTES;
		}
		if (i > 0) {
			bcopy(from, to, (unsigned)i);
			count -= i;
			to += i;
		}
		m0 = m;
		MFREE(m0, m);
	}
	return (count);
}
