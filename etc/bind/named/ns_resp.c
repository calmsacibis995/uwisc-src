#ifndef lint
static char sccsid[] = "@(#)ns_resp.c	4.8 (Berkeley) 9/24/86";
#endif

/*
 * Copyright (c) 1986 Regents of the University of California
 *	All Rights Reserved
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <syslog.h>
#include <arpa/nameser.h>
#include "ns.h"
#include "db.h"

extern	int	debug;
extern	FILE	*ddt;
extern int errno;
extern char *dnptrs[];
extern char *malloc();
struct	databuf *nsp[MAXNS], **nspp;

ns_resp(msg, msglen)
	char *msg;
	int msglen;
{
	register struct qinfo *qp;
	register HEADER *hp;
	register char *cp, *tp;
	int i, c, n, ancount, nscount, arcount;
	int type;
	int qtype, qclass;
	int cname = 0; /* flag for processing cname response */
	int count, founddata, foundname;
	int buflen;
	int newmsglen;
	char name[MAXDNAME], *dname;
	char *fname;
	char *newmsg;
	char **dpp;
	time_t curtime;

	struct hashbuf *htp;
	struct databuf *dp, *tmp, *pdp;
	struct namebuf *np;
	extern struct qinfo *qhead;
	extern int nsid;
	extern int addcount;
	extern int priming;

	hp = (HEADER *) msg;
	if ((qp = qfindid(hp->id)) == NULL ) {
#ifdef DEBUG
		if (debug >= 2)
			fprintf(ddt,"DUP? dropped\n");
#endif
		return;
	}

#ifdef DEBUG
	if (debug >= 2)
		fprintf(ddt,"%s response id=%d\n",
			qp->q_system ? "SYSTEM" : "USER", ntohs(qp->q_id));
#endif
	if (hp->rcode != NOERROR || hp->opcode != QUERY) {
		hp->id = qp->q_id; hp->rd = hp->ra = 1;
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving #1\n");
#endif
	    	if (!qp->q_system)
			(void) send_msg(msg, msglen, qp);
		qremove(qp);
		return;
	}
	/*
	 * Skip query section
	 */
	addcount = 0;
	cp = msg + sizeof(HEADER);
	dpp = dnptrs;
	*dpp++ = msg;
	if ((*cp & INDIR_MASK) == 0)
		*dpp++ = cp;
	*dpp = NULL;
	if (hp->qdcount) {
		if ((n = dn_skip(cp)) < 0) {
#ifdef DEBUG
			if (debug)
			    fprintf(ddt,"FORMERR ns_resp() dn_skip failed\n");
#endif
			hp->rcode = FORMERR;
			hp->id = qp->q_id; hp->rd = 1; hp->ra = 1;
#ifdef DEBUG
			if (debug >= 3)
				fprintf(ddt,"resp: leaving #2\n");
#endif
		    	if (!qp->q_system)
				(void) send_msg(msg, msglen, qp);
			qremove(qp);
			return;
		}
		cp += n;
		qtype = getshort(cp);
		cp += sizeof(u_short);
		qclass = getshort(cp);
		cp += sizeof(u_short);
	}
	/*
	 * Save answers, name server, and additional records for future use.
	 */
	ancount = ntohs(hp->ancount);
	nscount = ntohs(hp->nscount);
	arcount = ntohs(hp->arcount);
	if (ancount == 1 || nscount) {
		/*
	 	 *  Check if it's a CNAME response
	 	 */
		tp = cp;
		tp += dn_skip(tp); /* name */
		type = getshort(tp);
		tp += sizeof(u_short); /* type */
		if (type == T_CNAME) {
			tp += sizeof(u_short); /* class */
			tp += sizeof(u_long);  /* ttl */
			tp += sizeof(u_short); /* dlen */
			cname++;
#ifdef DEBUG
			if (debug) {
				fprintf(ddt,"CNAME - needs more processing\n");
			}
#endif
			if (!qp->q_cmsglen) {
				qp->q_cmsg = qp->q_msg;
				qp->q_cmsglen = qp->q_msglen;
			}
		}
	}
	/*
	 * Add the info received in the response to the Data Base
	 */
	c = ancount + nscount + arcount;
	if (hp->qdcount && qtype == T_CNAME && c == 0 && !qp->q_system) {
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving #3\n");
#endif
		/* Nothing to store, just give user the answer */
		hp->id = qp->q_id; hp->rd = hp->ra = 1;
		(void) send_msg(msg, msglen, qp);

		/* Cause us to put it in the cache later */
		prime(qclass, T_ANY, qp);
		qremove(qp);
		return;
	}
	nspp = nsp;
	for (i = 0; i < c; i++) {
		if (cp >= msg + msglen) {
#ifdef DEBUG
			if (debug)
			    fprintf(ddt, "FORMERR ns_resp() message bad count?\n");
#endif
			hp->rcode = FORMERR;
			hp->id = qp->q_id; hp->rd = hp->ra = 1;
		    	if (!qp->q_system)
				(void) send_msg(msg, msglen, qp);
			qremove(qp);
			return;
		}
		if ((n = doupdate(msg, msglen, cp, 0,
		    !ancount && i < nscount)) < 0) {
#ifdef DEBUG
			if (debug)
			    fprintf(ddt,"FORMERR ns_resp() doupdate failed\n");
#endif
			hp->rcode = FORMERR;
			hp->id = qp->q_id; hp->rd = hp->ra = 1;
		    	if (!qp->q_system)
				(void) send_msg(msg, msglen, qp);
			qremove(qp);
			return;
		}
		cp += n;
	}

	if (qp->q_system) {
		if (qp->q_system == PRIMING_CACHE) 
			priming = qp->q_system = 0;
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving #4\n");
		if (debug >= 2)
			fp_query(msg, ddt);
#endif
		qremove(qp);
		return;
	}

	if (cp > msg + msglen) {
#ifdef DEBUG
		if (debug)
		    fprintf(ddt,"FORMERR ns_resp()  packet size err %d, %d\n",
		       cp-msg, msglen);
#endif
		hp->rcode = FORMERR;
		hp->id = qp->q_id; hp->rd = hp->ra = 1;
		(void) send_msg(msg, msglen, qp);
		qremove(qp);
		return;
	}
	if (qtype == C_ANY && ancount) {
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving #5\n");
#endif
		hp->id = qp->q_id; hp->rd = hp->ra = 1;
		(void) send_msg(msg, msglen, qp);
		qremove(qp);
		return;
	}
	if ((!cname && !qp->q_cmsglen) && (ancount || nscount == 0)) {
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving #6\n");
#endif
		hp->id = qp->q_id; hp->rd = hp->ra = 1;
		(void) send_msg(msg, msglen, qp);
		qremove(qp);
		return;
	}
	if (hp->aa && ancount == 0 && nscount == 0) {
		/* We have an authoritative NO */
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving with nothing\n");
#endif
		hp = (HEADER *)(qp->q_cmsglen ? qp->q_cmsg : qp->q_msg);
		hp->aa = 1;
		hp->id = qp->q_id; hp->rd = hp->ra = hp->qr = 1;
		if (qp->q_cmsglen)
			(void) send_msg(qp->q_cmsg, qp->q_cmsglen, qp);
		else
			(void) send_msg(qp->q_msg, qp->q_msglen, qp);
		qremove(qp);
		return;
	}

	/*
	 * All messages in here need further processing.  i.e. they
	 * are either CNAMEs or we got referred again.
	 */
	count = 0;
	founddata = 0;
	foundname = 0;
	dname = name;
	if ((newmsg = malloc(BUFSIZ)) == NULL) {
#if DEBUG
		if (debug)
			fprintf(ddt,"ns_resp: malloc error\n");
#endif
		syslog(LOG_ERR, "ns_resp: Out Of Memory");
		hp->rcode = SERVFAIL;
		hp->id = qp->q_id; hp->rd = 1; hp->ra = 1;
		(void) send_msg(msg, msglen, qp);
		qremove(qp);
		return;
	}
	buflen = BUFSIZ;
	if (!cname && qp->q_cmsglen && ancount) {
#if DEBUG
		if (debug)
			fprintf(ddt,"Cname second pass\n");
#endif
		newmsglen = qp->q_cmsglen;
		bcopy(qp->q_cmsg, newmsg, newmsglen);
	} else {
		newmsglen = msglen;
		bcopy(msg, newmsg, newmsglen);
	}
	buflen = buflen - newmsglen;
	hp = (HEADER *) newmsg;
	dnptrs[0] = newmsg;
	dnptrs[1] = NULL;
	cp = newmsg + sizeof(HEADER);
	if (cname)
		cp += dn_skip(cp) + QFIXEDSZ;
	if ((n = dn_expand(newmsg, newmsg + newmsglen,
		cp, dname, sizeof(name))) < 0) {
#ifdef DEBUG
		if (debug)
			fprintf(ddt,"dn_expand failed\n" );
#endif
		hp->rcode = SERVFAIL;
		free(newmsg);
		hp->id = qp->q_id; hp->rd = hp->ra = 1;
		(void) send_msg(msg, msglen, qp);
		(void) free(newmsg);
		qremove(qp);
		return;
	}
	cp = newmsg + sizeof(HEADER);
	if (cname)
		cp += dn_skip(cp);
	else
		cp += n;
	cp += QFIXEDSZ;

again:
	htp = hashtab;		/* lookup relative to root */
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"ns_resp() nlookup(%s) type=%d\n",dname, qtype);
#endif
	fname = "";
	np = nlookup(dname, &htp, &fname, 0);
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"resp: %s '%s' as '%s' (cname=%d)\n",
			np == NULL ? "missed" : "found", dname, fname, cname);
#endif
	if (np == NULL || fname != dname) {
#ifdef DEBUG
		if (debug >= 2)
			fprintf(ddt, "fname != dname\n");
#endif
		goto findns;
	}
	foundname++;
	pdp = NULL;
	dp = np->n_data;
	/* look for the data */
	while (dp != NULL) {
		if (!wanted(dp, qclass, qtype)) {
			pdp = dp;
			dp = dp->d_next;
			continue;
		}
#ifdef DEBUG
		if (debug >=3)
			fprintf(ddt,"adding data type=%d\n", dp->d_type);
#endif
		if ((n = make_rr(dname, dp, cp, buflen, 1)) < 0) {
			if (n == -1) {
				hp->tc = 1;
				break;
			}
#ifdef DEBUG
			if (debug >=3)
				fprintf(ddt,"deleting cache entry %s\n", np->n_dname);
#endif
			rminv(dp);
			tmp = dp->d_next;
			free((char *)dp);
			dp = tmp;
			if (pdp == NULL)
				np->n_data = dp;
			else
				pdp->d_next = dp;
			continue;
		}
		cp += n;
		buflen -= n;
		count++;
		if (dp->d_zone)
			hp->aa = 1;
		if (dp->d_type == T_CNAME) {
			if (qtype == T_ANY)
				break;
			dname = dp->d_data;
			goto again;
		}
		founddata++;
		pdp = dp;
		dp = dp->d_next;
	}

	if (qtype == T_CNAME) {
		founddata++;	/* Just finding it is enough */
		if (np->n_data && np->n_data->d_zone)
			hp->aa = 1;
	}
#ifdef DEBUG
	if (debug >= 2) {
		fprintf(ddt,"resp: foundname = %d count = %d ", foundname, count);
		fprintf(ddt,"founddata = %d cname = %d\n", founddata, cname);
	}
#endif

findns:
	/*
 	 * Look for name servers to refer to and fill in the authority
 	 * section or record the address for forwarding the query
 	 * (recursion desired).
 	 */
	hp->ancount = htons((u_short)count);
	for (count = 0;; np = np->n_parent) {
#ifdef DEBUG
		if (debug >= 5)
			fprintf(ddt, "fname = '%s'\n", fname);
#endif
		if (*fname == '\0') {
			for (np = hashtab->h_tab[0]; np != NULL;
			     np = np->n_next)
				if (np->n_dname[0] == '\0')
					goto foundns;
#ifdef DEBUG
			if (debug)
				fprintf(ddt, "No root nameserver?\n");
#endif
			syslog(LOG_ERR, "No root Nameserver\n");
			hp->rcode = SERVFAIL;
			break;
		}
foundns:
		nspp = nsp;
		pdp = NULL;
		dp = np->n_data;
		curtime =(u_long) tt.tv_sec;
		while (dp != NULL) {
			if (dp->d_zone && match(dp, qclass, T_SOA)
			  && (!foundname || hp->aa)) {
#ifdef DEBUG
				if (debug >= 3)
					fprintf(ddt,"resp: leaving #7\n");
#endif
				if (!foundname)
			            hp->rcode = NXDOMAIN;
			        hp->aa = hp->qr = hp->ra = 1;
				hp->id = qp->q_id;
				n = doaddinfo(hp, cp, buflen);
			  	cp += n;
			        (void) send_msg(msg, cp - msg, qp);
				(void) free(newmsg);
				qremove(qp);
				return;
			}	
			if (!match(dp, qclass, T_NS)) {
				pdp = dp;
				dp = dp->d_next;
				continue;
			}
			if ((dp->d_zone == 0) && (dp->d_ttl < curtime)) {
				/* delete old cache entry */
#ifdef DEBUG
				if (debug)
					fprintf(ddt,"deleting cache entry %s\n",
					    np->n_dname);
#endif
				rminv(dp);
				tmp = dp->d_next;		
				(void) free((char *)dp);
				dp = tmp;
				if (pdp == NULL)
					np->n_data = dp;
				else
					pdp->d_next = dp;
				if (np->n_dname[0] == '\0')
					prime_cache();
				continue;
			}
			if (!founddata) {
				if (nspp < &nsp[MAXNS-1])
					*nspp++ = dp;
				pdp = dp;
				dp = dp->d_next;
				continue;
			}
			if ((n = make_rr(fname, dp, cp, buflen, 1)) < 0) {
				if (n == -1) {
					hp->tc = 1;
					break;
				}
				/* delete old cache entry */
#ifdef DEBUG
				if (debug)
					fprintf(ddt,"deleting cache entry %s\n",
					    np->n_dname);
#endif
				rminv(dp);
				tmp = dp->d_next;		
				free((char *)dp);
				dp = tmp;
				if (pdp == NULL)
					np->n_data = dp;
				else
					pdp->d_next = dp;
				continue;
			}
#ifdef DEBUG
			if (debug >= 3)
				fprintf(ddt,"adding ns data\n");
#endif
			cp += n;
			buflen -= n;
			count++;
			pdp = dp;
			dp = dp->d_next;
		}
		if (count && founddata) {
#ifdef DEBUG
			if (debug >= 3)
				fprintf(ddt,"resp: leaving #8\n");
#endif
			hp = (HEADER *)newmsg;
			hp->nscount = htons((u_short)count);
			hp->id = qp->q_id; hp->qr = hp->rd = hp->ra = 1;
			n = doaddinfo(hp, cp, buflen);
			cp += n;
			(void) send_msg(newmsg, cp - newmsg, qp);
			(void) free(newmsg);
			qremove(qp);
			return;
		}
		if (nspp != nsp) {
		    *nspp = NULL;
		    if (qp->q_nqueries++ > MAXQUERIES) {
#if DEBUG
			if (debug)
			    fprintf(ddt,"ns_resp: MAXQUERIES exceeded (%s, class %d, type %d)\n",
				dname, qclass, qtype);
#endif
			syslog(LOG_ERR, "ns_resp: MAXQUERIES exceeded (%s, class %d, type %d)",
				dname, qclass, qtype);
		    	if (cname)
			    hp = (HEADER *)qp->q_cmsg;
			hp->rcode = SERVFAIL;
			hp->id = qp->q_id; hp->rd = 1; hp->ra = 1;
		    	if(cname)
			    (void) send_msg(qp->q_cmsg, qp->q_cmsglen, qp);
		    	else
			    (void) send_msg(msg, msglen, qp);
			qremove(qp);
			return;
		    }
		    qp->q_naddr = 0;
		    qp->q_curaddr = 0;
		    if (nslookup(nsp, qp, hashtab) == 0) {
#ifdef DEBUG
			if (debug >= 3)
				fprintf(ddt,"resp: none found in nsp\n");
#endif
		    	if (cname)
			    hp = (HEADER *)qp->q_cmsg;
			hp->rcode = SERVFAIL;
			hp->id = qp->q_id; hp->rd = 1; hp->ra = 1;
		    	if(cname)
			    (void) send_msg(qp->q_cmsg, qp->q_cmsglen, qp);
		    	else
			    (void) send_msg(msg, msglen, qp);
			(void) free(newmsg);
			qremove(qp);
			return;
		    }
		    if (cname) {
#ifdef DEBUG
			if (debug >= 3)
			       fprintf(ddt,"resp: building recursive query; nslookp3\n");
#endif
			qp->q_cname++;
			if ((newmsg = malloc(BUFSIZ)) == NULL) {
#if DEBUG
			     if (debug)
			           fprintf(ddt,"ns_resp: malloc fail\n");
#endif
			     syslog(LOG_ERR, "ns_resp: Out Of Memory");
			     hp = (HEADER *)qp->q_cmsg;
			     hp->rcode = SERVFAIL;
			     hp->rd = hp->ra = 1;
			     (void) send_msg(qp->q_cmsg, qp->q_cmsglen, qp);
			     qremove(qp);
			     return;
			}
			buflen = BUFSIZ;
			dnptrs[0] = newmsg;
			dnptrs[1] = NULL;
			newmsglen = res_mkquery(QUERY, dname, qclass,
			    qtype, (char *)NULL, 0, NULL, newmsg, buflen);
			qp->q_msg = newmsg;
			qp->q_msglen = newmsglen;
			hp = (HEADER *) newmsg;
			hp->id = qp->q_nsid = htons((u_short)++nsid);
			hp->ancount = 0;
			hp->nscount = 0;
			hp->arcount = 0;
		    	hp->rd = 0;		/* Not for NAMED ??? */
		    	unsched(qp);
			schedretry(qp, (time_t)RETRYTIME);

#ifdef DEBUG
			if (debug)
			    fprintf(ddt,"resp: forw -> %s (%d) id=%d\n",
			        inet_ntoa(qp->q_addr[0].sin_addr),
				ntohs(qp->q_addr[0].sin_port),
				ntohs(qp->q_id));
			if ( debug >= 10)
			    fp_query(newmsg, ddt);
#endif
			if (sendto(ds, newmsg, newmsglen, 0,
			    &qp->q_addr[0], sizeof(qp->q_addr[0])) < 0){
#ifdef DEBUG
			    if (debug)
			        fprintf(ddt,"sendto error \n");
#endif
			}
			return;
		    }
		    hp = (HEADER *)qp->q_msg;
		    hp->id = qp->q_nsid = htons((u_short)++nsid);

		    unsched(qp);
		    schedretry(qp, (time_t)RETRYTIME);
#ifdef DEBUG
		    if (debug)
			fprintf(ddt,"resp: forw2 -> %s (%d) id=%d\n",
			    inet_ntoa(qp->q_addr[0].sin_addr),
			    ntohs(qp->q_addr[0].sin_port),
			    ntohs(qp->q_id));
		    if ( debug >= 10)
			fp_query(msg, ddt);
#endif
		    if (sendto(ds, qp->q_msg, qp->q_msglen, 0, &qp->q_addr[0],
			sizeof(qp->q_addr[0])) < 0) {
#ifdef DEBUG
			if (debug >= 5)
			    fprintf(ddt,"error forwarding msg\n");
#endif
		    }
#ifdef DEBUG
		    if (debug >= 3)
			fprintf(ddt,"resp: leaving #9\n");
#endif
		    (void) free(newmsg);
		    return;
		}
		if (((*fname == '\0') || (count > 0)) || (nspp != nsp)) {
			hp->id = qp->q_id; hp->rd = 1; hp->ra = 1;
			(void) send_msg(msg, msglen, qp);
			(void) free(newmsg);
			qremove(qp);
			return;
		}
		if ((fname = index(fname, '.')) == NULL)
			fname = "";
		else
			fname++;
	}
	*nspp = NULL;
	if (qp->q_nqueries++ > MAXQUERIES) {
#if DEBUG
		if (debug)
		    fprintf(ddt,"ns_resp: MAXQUERIES exceeded (%s, class %d, type %d)\n",
			dname, qclass, qtype);
#endif
		syslog(LOG_ERR, "ns_resp: MAXQUERIES exceeded (%s, class %d, type %d)",
			dname, qclass, qtype);
	    	if (cname)
		    hp = (HEADER *)qp->q_cmsg;
		hp->rcode = SERVFAIL;
		hp->id = qp->q_id; hp->rd = 1; hp->ra = 1;
	    	if(cname)
		    (void) send_msg(qp->q_cmsg, qp->q_cmsglen, qp);
	    	else
		    (void) send_msg(msg, msglen, qp);
		free(newmsg);
		qremove(qp);
		return;
	}
	if (cname) {
		newmsglen = res_mkquery(QUERY, dname, C_ANY, T_A, (char *)NULL,
					0, NULL, newmsg, BUFSIZ);
		qp->q_msglen = newmsglen;
		hp->id = qp->q_nsid;
	} else {
		hp->ancount = 0;
		hp->nscount = 0;
		hp->arcount = 0;
		hp->qr = 0;
	}
	qp->q_naddr = 0;
	qp->q_curaddr = 0;
#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"resp: nslookup2\n");
#endif
	n = nslookup(nsp, qp, hashtab);

#ifdef DEBUG
	if (debug > 7) {
		int kjd;

		fprintf(ddt,"n = %d\n",n);
		for (kjd = 0; kjd < qp->q_naddr; kjd++ )
			fprintf(ddt,"list %d-> %s (%d)\n",  kjd,
				inet_ntoa(qp->q_addr[kjd].sin_addr),
				ntohs(qp->q_addr[kjd].sin_port));
	}
#endif DEBUG
	qp->q_msg = newmsg;
 	if (qp->q_cname++ == MAXCNAMES) {
 		hp->id = qp->q_id;
 		hp->rd = hp->ra = 1;		
 		(void) send_msg(newmsg, newmsglen, qp);
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"resp: leaving #11\n");
#endif
	 	qremove(qp);	/* Frees up newmsg */
 		return;
 	}
#ifdef DEBUG
 	if (debug)
 		fprintf(ddt,"q_cname = %d\n",qp->q_cname);
#endif
	unsched(qp);
	schedretry(qp, (time_t)RETRYTIME);

#ifdef DEBUG
	if (debug >= 3)
		fp_query(qp->q_msg, ddt);
	if (debug)
		fprintf(ddt,"try -> %s (%d)\n",
			inet_ntoa(qp->q_addr[0].sin_addr),
			ntohs(qp->q_addr[0].sin_port));
#endif
	if (sendto(ds, qp->q_msg, qp->q_msglen, 0,
		&qp->q_addr[0], sizeof(qp->q_addr[0])) < 0) {
#ifdef DEBUG
		if (debug)
			fprintf(ddt, "error returning msg errno=%d\n",errno);
#endif
	}
	return;
}

/*
 * Decode the resource record 'rrp' and update the database.
 * If savens is true, record pointer for forwarding queries a second time.
 */
doupdate(msg, msglen, rrp, zone, savens)
	char *msg ;
	char *rrp;
	int  msglen, zone, savens;
{
	register char *cp;
	register int n;
	int class, type, dlen, n1;
	u_long ttl;
	struct databuf *dp;
	char dname[MAXDNAME];
	char data[BUFSIZ], *cp1;

#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"doupdate(%d, %d)\n", zone, savens);
#endif

	cp = rrp;
	if ((n = dn_expand(msg, msg + msglen, cp, dname, sizeof(dname))) < 0)
		return (-1);
	cp += n;
	type = getshort(cp);
	cp += sizeof(u_short);
	class = getshort(cp);
	cp += sizeof(u_short);
	ttl = getlong(cp);
	cp += sizeof(u_long);
	dlen = getshort(cp);
	cp += sizeof(u_short);
	if (zone == 0) {
		if (ttl == 0)
			ttl = 5 * 60;
		ttl += (u_long) tt.tv_sec;
	}
	/*
	 * Convert the resource record data into the internal
	 * database format.
	 */
	switch (type) {
	case T_A:
	case T_HINFO:
	case T_UINFO:
	case T_UID:
	case T_GID:
		cp1 = cp;
		n = dlen;
		cp += n;
		break;

	case T_CNAME:
	case T_MB:
	case T_MG:
	case T_MR:
	case T_NS:
	case T_PTR:
		if ((n = dn_expand(msg, msg + msglen, cp, data,
		   sizeof(data))) < 0)
			return (-1);
		cp += n;
		cp1 = data;
		n = strlen(data) + 1;
		break;

	case T_MINFO:
	case T_SOA:
		if ((n = dn_expand(msg, msg + msglen, cp, data,
		    sizeof(data))) < 0)
			return (-1);
		cp += n;
		cp1 = data + (n = strlen(data) + 1);
		n1 = sizeof(data) - n;
		if (type == T_SOA)
			n1 -= 5 * sizeof(u_long);
		if ((n = dn_expand(msg, msg + msglen, cp, cp1, n1)) < 0)
			return (-1);
		cp += n;
		cp1 += strlen(cp1) + 1;
		if (type == T_SOA) {
			bcopy(cp, cp1, n = 5 * sizeof(u_long));
			cp += n;
			cp1 += n;
		}
		n = cp1 - data;
		cp1 = data;
		break;

  	case T_MX:
 		/* grab preference */
 		bcopy(cp,data,sizeof(u_short));
 		cp1 = data + sizeof(u_short);
 		cp += sizeof(u_short);

 		/* get name */
  		if ((n = dn_expand(msg, msg + msglen, cp, cp1,
		    sizeof(data)-sizeof(u_short))) < 0)
  			return(-1);
  		cp += n;

 		/* compute end of data */
 		cp1 += strlen(cp1) + 1;
 		/* compute size of data */
  		n = cp1 - data;
  		cp1 = data;
  		break;

	default:
#ifdef DEBUG
		if (debug >= 3)
			fprintf(ddt,"unknown type %d\n", type);
#endif
		return ((cp - rrp) + dlen);
	}
	dp = savedata(class, type, ttl, cp1, n);
	dp->d_zone = zone;
	if ((n = db_update(dname, dp, dp, DB_NODATA, hashtab)) < 0) {
#ifdef DEBUG
		if (debug && (n != DATAEXISTS))
			fprintf(ddt,"update failed (%d)\n", n);
		else if (debug >= 3)
			fprintf(ddt,"update failed (DATAEXISTS)\n");
#endif
		(void) free((char *)dp);
	} else if (savens && type == T_NS && nspp < &nsp[MAXNS-1])
		*nspp++ = dp;
	return (cp - rrp);
}

send_msg(msg, msglen, qp)
	char *msg;
	int msglen;
	struct qinfo *qp;
{
	extern struct qinfo *qhead;
#ifdef DEBUG
	struct qinfo *tqp;

	if (debug) {
		fprintf(ddt,"send_msg -> %s (%s %d) id=%d\n",
			inet_ntoa(qp->q_from.sin_addr),
			qp->q_stream == QSTREAM_NULL ? "UDP" : "TCP",
			ntohs(qp->q_from.sin_port),
			ntohs(qp->q_id));
	}
	if (debug>4)
		for (tqp = qhead; tqp!=QINFO_NULL; tqp = tqp->q_link) {
		    fprintf(ddt, "qp %x q_id: %d  q_nsid: %d q_msglen: %d ",
		    	tqp, tqp->q_id,tqp->q_nsid,tqp->q_msglen);
	            fprintf(ddt,"q_naddr: %d q_curaddr: %d\n", tqp->q_naddr,
			tqp->q_curaddr);
	            fprintf(ddt,"q_next: %x q_link: %x\n", qp->q_next,
		   	 qp->q_link);
		}
	if (debug >= 10)
		fp_query(msg, ddt);
#endif DEBUG
	if (qp->q_stream == QSTREAM_NULL) {
		if (sendto(ds, msg, msglen, 0,
		    &qp->q_from, sizeof(qp->q_from)) < 0) {
#ifdef DEBUG
			if (debug)
				fprintf(ddt,"sendto failed\n");
#endif
			return(1);
		}

	} else {
		(void) writemsg(qp->q_stream->s_rfd, msg, msglen);
		qp->q_stream->s_time = tt.tv_sec;
		qp->q_stream->s_refcnt--;
	}
	return(0);
}

prime(qclass, qtype, oqp)
	int qclass, qtype;
	register struct qinfo *oqp;
{
	char	dname[BUFSIZ];

	if (oqp->q_msg == NULL)
		return;
	if (dn_expand(oqp->q_msg, oqp->q_msg + oqp->q_msglen,
	    oqp->q_msg + sizeof(HEADER), dname, sizeof(dname)) < 0)
		return;
#ifdef DEBUG
	if (debug >= 2)
	       fprintf(ddt,"prime: %s\n", dname);
#endif

	sysquery(dname, qclass, qtype, oqp);
}

sysquery(dname, qclass, qtype, oqp)
	char *dname;
	int qclass, qtype;
	register struct qinfo *oqp;
{
	register struct qinfo *qp, *qlink;
	register HEADER *hp;
	char	*newmsg;
	int	newmsglen, buflen;

	/* build new qinfo struct */
	qp = qnew();
	qlink = qp->q_link;
	*qp = *oqp;
	qp->q_link = qlink;
	qp->q_curaddr = 0;
	qp->q_id = 0;
	qp->q_cmsglen = 0;
	qp->q_cmsg = NULL;
	qp->q_system++;

        if ((newmsg = malloc(BUFSIZ)) == NULL) {
        	qfree(qp);
        	return;
        }
        buflen = BUFSIZ;
	newmsglen = res_mkquery(QUERY, dname, qclass,
	    qtype, (char *)NULL, 0, NULL, newmsg, buflen);
	qp->q_msg = newmsg;
	qp->q_msglen = newmsglen;
	hp = (HEADER *) newmsg;
	hp->id = qp->q_nsid = htons((u_short)++nsid);
	hp->rd = 0;
	hp->ancount = 0;
	hp->nscount = 0;
	hp->arcount = 0;
	schedretry(qp, (time_t)RETRYTIME);

#ifdef DEBUG
	if (debug)
	    fprintf(ddt,"prime: forw -> %s (%d), id=%d\n",
	        inet_ntoa(qp->q_addr[0].sin_addr),
                ntohs(qp->q_addr[0].sin_port),
		ntohs(qp->q_id));
	if ( debug >= 10)
	    fp_query(newmsg, ddt);
#endif
	if (sendto(ds, newmsg, newmsglen, 0,
	    &qp->q_addr[0], sizeof(qp->q_addr[0])) < 0){
#ifdef DEBUG
	    if (debug)
	        fprintf(ddt,"sendto error \n");
#endif
	}
	return;
}

prime_cache()
{
	register struct qinfo *qp, *qlink;
	register HEADER *hp;
	char	*newmsg;
	int	newmsglen, buflen;
	int	n;
	struct namebuf *np;
	struct databuf *dp;
	extern int priming;

#ifdef DEBUG
	if (debug)
		fprintf(ddt,"prime_cache() priming = %d\n", priming);
#endif

	if (priming)
		return;
	priming++;
	nspp = nsp;	/* record ns records if forwarding */
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"nsp = %x nspp = %x\n", nsp, nspp);
#endif
	for (np = fcachetab->h_tab[0]; np != NULL; np = np->n_next) {
		if (np->n_dname[0] == '\0') {
			dp = np->n_data;
			while (dp != NULL) {
				if (match(dp, C_IN, T_NS)) {
					if (nspp < &nsp[MAXNS-1])
						*nspp++ = dp;
				}
				dp = dp->d_next;
			}
		}
	}
	*nspp = NULL;
	nspp = nsp;
	while ((dp = *nspp++) != NULL) {
#ifdef DEBUG
	    if (debug)
	       fprintf(ddt,"dname %s: class %x\n", dp->d_data, dp->d_class);
#endif
	}
	/* build new qinfo struct */
	qp = qnew();
	qlink = qp->q_link;
	qp->q_link = qlink;
	qp->q_naddr = 0;
	qp->q_curaddr = 0;
	qp->q_id = 0;
	qp->q_cmsglen = 0;
	qp->q_cmsg = NULL;
	qp->q_system = PRIMING_CACHE;
	n = nslookup(nsp, qp, fcachetab);
#ifdef DEBUG
	if (debug > 7) {
		int kjd;

		fprintf(ddt,"n = %d\n",n);
		for (kjd = 0; kjd < qp->q_naddr; kjd++ )
			fprintf(ddt,"list %d-> %s (%d)\n",  kjd,
				inet_ntoa(qp->q_addr[kjd].sin_addr),
				ntohs(qp->q_addr[kjd].sin_port));
	}
#endif DEBUG

        if ((newmsg = malloc(BUFSIZ)) == NULL) {
		qfree(qp);
        	return;
        }
        buflen = BUFSIZ;
	newmsglen = res_mkquery(QUERY, ".", C_ANY,
	    T_ANY, (char *)NULL, 0, NULL, newmsg, buflen);
	qp->q_msg = newmsg;
	qp->q_msglen = newmsglen;
	hp = (HEADER *) newmsg;
	hp->id = qp->q_nsid = htons((u_short)++nsid);
	hp->rd = 0;
	hp->ancount = 0;
	hp->nscount = 0;
	hp->arcount = 0;
	schedretry(qp, (time_t)RETRYTIME+10);

#ifdef DEBUG
	if (debug)
	    fprintf(ddt,"prime: forw -> %s (%d), id=%d\n",
	        inet_ntoa(qp->q_addr[0].sin_addr),
                ntohs(qp->q_addr[0].sin_port),
		ntohs(qp->q_id));
	if ( debug >= 10)
	    fp_query(newmsg, ddt);
#endif
	if (sendto(ds, newmsg, newmsglen, 0,
	    &qp->q_addr[0], sizeof(qp->q_addr[0])) < 0){
#ifdef DEBUG
	    if (debug)
	        fprintf(ddt,"sendto error \n");
#endif
	}
	return;
}
