#ifndef lint
static char sccsid[] = "@(#)db_dump.c	4.6 (Berkeley) 7/14/86";
#endif

/*
 * Copyright (c) 1986 Regents of the University of California
 *	All Rights Reserved
 */

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <syslog.h>
#include <arpa/nameser.h>
#include "ns.h"
#include "db.h"

extern	char *p_type(), *p_class();

#ifdef DUMPFILE
char	*dumpfile = DUMPFILE;
#else
char	*dumpfile = "/usr/tmp/named_dump.db";
#endif

/*
 * Dump current data base in a format similar to RFC 883.
 */

doadump()
{
	FILE	*fp;

#ifdef DEBUG
	if (debug >= 3)
		fprintf(ddt,"doadump()\n");
#endif

	if ((fp = fopen(dumpfile, "w")) == NULL)
		return;
	fprintf(fp, "$ORIGIN .\n");
	if (hashtab != NULL)
		db_dump(hashtab, fp, 0);
	(void) fclose(fp);
}

/* Create a disk database to back up zones for which we are secondary
 * servers
 */
zonedump(zp)
    register struct zoneinfo *zp;
{
	FILE	*fp;
	char *fname;
	struct hashbuf *htp;
	char *op;

	/* Only dump zone if there is a cache specified */
	if (zp->z_source && *(zp->z_source)) {
#ifdef DEBUG
	    if (debug)
		    fprintf(ddt, "zonedump(%s)\n", zp->z_source);
#endif

	    if ((fp = fopen(zp->z_source, "w")) == NULL)
		    return;
	    op = index(zp->z_origin, '.');
	    if (op == NULL)
		    fprintf(fp, "$ORIGIN .\n");
	    else
		    fprintf(fp, "$ORIGIN %s.\n", ++op);
	    htp = hashtab;
	    if (nlookup(zp->z_origin, &htp, &fname, 0) != NULL)
		    db_dump(htp, fp, zp-zones);
	    (void) fclose(fp);
	}
#ifdef DEBUG
	    else if (debug)
		fprintf(ddt, "zonedump: no zone to dump\n");
#endif
}

db_dump(htp, fp, zone)
	int zone;
	struct hashbuf *htp;
	FILE *fp;
{
	register struct databuf *dp;
	register struct namebuf *np;
	struct namebuf **npp, **nppend;
	char dname[MAXDNAME];
	long n;
	u_long addr;
	u_short i;
	int j;
	char *cp;
	char *proto;
	extern char *inet_ntoa(), *p_protocal(), *p_service();
	int found_data;

	npp = htp->h_tab;
	nppend = npp + htp->h_size;
	while (npp < nppend) {
	    for (np = *npp++; np != NULL; np = np->n_next) {
		if (np->n_data == NULL)
			continue;
		/* Blecch - can't tell if there is data here for the
		 * right zone, so can't print name yet
		 */
		found_data = 0;
		for (dp = np->n_data; dp != NULL; dp = dp->d_next) {
			/* Is the data for this zone? */
			if (zone && dp->d_zone != zone)
			    continue;
			if (!found_data) {
			    fprintf(fp, "%s\t", np->n_dname);
			    if (strlen(np->n_dname) < 8)
				(void) putc('\t', fp);
			    found_data++;
			} else
				fprintf(fp, "\t\t");
			if (dp->d_zone == 0) {
				if (gettimeofday(&tt, (struct timezone *)0) < 0)
					syslog(LOG_ERR, "gettimeofday: %m");
				if ((n = dp->d_ttl - tt.tv_sec) >= 0)
					fprintf(fp, "%d\t", n);
			} else if (dp->d_ttl > zones[dp->d_zone].z_minimum)
				fprintf(fp, "%d\t", dp->d_ttl);
			fprintf(fp, "%s\t%s\t", p_class(dp->d_class),
				p_type(dp->d_type));
			cp = dp->d_data;
			/*
			 * Print type specific data
			 */
			switch (dp->d_type) {
			case T_A:
				switch (dp->d_class) {
				case C_IN:
					n = htonl(getlong(cp));
					fprintf(fp, "%s\n",
					   inet_ntoa(*(struct in_addr *)&n));
					break;
				}
				break;
			case T_CNAME:
			case T_MB:
			case T_MG:
			case T_MR:
			case T_NS:
			case T_PTR:
				if (cp[0] == '\0')
					fprintf(fp, ".\n");
				else
					fprintf(fp, "%s.\n", cp);
				break;

			case T_HINFO:
				if (n = *cp++) {
					fprintf(fp, "\"%.*s\"", n, cp);
					cp += n;
				} else
					fprintf(fp, "\"\"");
				if (n = *cp++)
					fprintf(fp, " \"%.*s\"", n, cp);
				else
					fprintf(fp, "\"\"");
				(void) putc('\n', fp);
				break;

			case T_SOA:
				fprintf(fp, "%s.", cp);
				cp += strlen(cp) + 1;
				fprintf(fp, " %s.\n", cp);
				cp += strlen(cp) + 1;
				fprintf(fp, "\t\t( %d", getlong(cp));
				cp += sizeof(u_long);
				fprintf(fp, " %d", getlong(cp));
				cp += sizeof(u_long);
				fprintf(fp, " %d", getlong(cp));
				cp += sizeof(u_long);
				fprintf(fp, " %d", getlong(cp));
				cp += sizeof(u_long);
				fprintf(fp, " %d )\n", getlong(cp));
				break;

			case T_MX:
				fprintf(fp,"%d", getshort(cp));
				cp += sizeof(u_short);
				fprintf(fp," %s.\n", cp);
				break;


			case T_UINFO:
				fprintf(fp, "%s\n", cp);
				break;

			case T_UID:
			case T_GID:
				if (dp->d_size == sizeof(u_long)) {
					fprintf(fp, "%d\n", getlong(cp));
					cp += sizeof(u_long);
				}
				break;

			case T_WKS:
				addr = htonl(getlong(cp));	
				fprintf(fp,"%s ",
				    inet_ntoa(*(struct in_addr *)&addr));
				cp += sizeof(u_long);
				proto = p_protocal(*cp); /* protocal */
				cp += sizeof(char); 
				fprintf(fp, "%s ", proto);
				i = 0;
				while(cp < dp->d_data + dp->d_size) {
					j = *cp++;
					do {
						if(j & 0200)
							fprintf(fp," %s",
							   p_service(i, proto));
						j <<= 1;
					} while(++i & 07);
				} 
				fprintf(fp,"\n");
				break;

			default:
				fprintf(fp, "???\n", cp);
			}
		}
	    }
	}
	npp = htp->h_tab;
	nppend = npp + htp->h_size;
	while (npp < nppend) {
	    for (np = *npp++; np != NULL; np = np->n_next) {
		if (np->n_hash == NULL)
			continue;
		getname(np, dname, sizeof(dname));
		fprintf(fp, "$ORIGIN %s.\n", dname);
		db_dump(np->n_hash, fp, zone);
	    }
	}
}

/*  These next two routines will be moveing to res_debug.c */
/* They are currently here for ease of distributing the WKS record fix */
char *
p_protocal(num)
int num;
{
	struct protoent *pp;
	pp = getprotobynumber(num);
	if(pp == 0)  
		return("???");
	return(pp->p_name);
}

char *
p_service(port, proto)
u_short port;
char *proto;
{
	struct servent *ss;
	ss = getservbyport((int)htons(port), proto);
	if(ss == 0)  
		return("???");
	return(ss->s_name);
}

