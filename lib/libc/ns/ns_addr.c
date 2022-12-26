#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: ns_addr.c,v 1.2 86/09/08 14:48:15 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Includes material written at Cornell University, by J. Q. Johnson.
 * Used by permission.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ns_addr.c	6.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <sys/types.h>
#include <netns/ns.h>

static struct ns_addr addr, zero_addr;

struct ns_addr 
ns_addr(name)
	char *name;
{
	u_long net;
	u_short socket;
	char separator = '.';
	char *hostname, *socketname, *cp;
	char buf[50];
	extern char *index();

	addr = zero_addr;
	strncpy(buf, name, 49);

	/*
	 * First, figure out what he intends as a field separtor.
	 * Despite the way this routine is written, the prefered
	 * form  2-272.AA001234H.01777, i.e. XDE standard.
	 * Great efforts are made to insure backward compatability.
	 */
	if (hostname = index(buf, '#'))
		separator = '#';
	else {
		hostname = index(buf, '.');
		if ((cp = index(buf, ':')) &&
		    ( (hostname && cp < hostname) || (hostname == 0))) {
			hostname = cp;
			separator = ':';
		}
	}
	if (hostname)
		*hostname++ = 0;
	Field(buf, addr.x_net.c_net, 4);
	if (hostname == 0)
		return (addr);  /* No separator means net only */

	socketname = index(hostname, separator);
	if (socketname) {
		*socketname++ = 0;
		Field(socketname, &addr.x_port, 2);
	}

	Field(hostname, addr.x_host.c_host, 6);

	return (addr);
}

static
Field(buf, out, len)
char *buf;
u_char *out;
int len;
{
	register char *bp = buf;
	int i, ibase, base16 = 0, base10 = 0, clen = 0;
	int hb[6], *hp;
	char *fmt;

	/*
	 * first try 2-273#2-852-151-014#socket
	 */
	if ((*buf != '-') &&
	    (1 < (i = sscanf(buf, "%d-%d-%d-%d-%d",
			&hb[0], &hb[1], &hb[2], &hb[3], &hb[4])))) {
		cvtbase(1000, 256, hb, i, out, len);
		return;
	}
	/*
	 * try form 8E1#0.0.AA.0.5E.E6#socket
	 */
	if (1 < (i = sscanf(buf,"%x.%x.%x.%x.%x.%x",
			&hb[0], &hb[1], &hb[2], &hb[3], &hb[4], &hb[5]))) {
		cvtbase(256, 256, hb, i, out, len);
		return;
	}
	/*
	 * try form 8E1#0:0:AA:0:5E:E6#socket
	 */
	if (1 < (i = sscanf(buf,"%x:%x:%x:%x:%x:%x",
			&hb[0], &hb[1], &hb[2], &hb[3], &hb[4], &hb[5]))) {
		cvtbase(256, 256, hb, i, out, len);
		return;
	}
	/*
	 * This is REALLY stretching it but there was a
	 * comma notation separting shorts -- definitely non standard
	 */
	if (1 < (i = sscanf(buf,"%x,%x,%x",
			&hb[0], &hb[1], &hb[2]))) {
		hb[0] = htons(hb[0]); hb[1] = htons(hb[1]);
		hb[2] = htons(hb[2]);
		cvtbase(65536, 256, hb, i, out, len);
		return;
	}

	/* Need to decide if base 10, 16 or 8 */
	while (*bp) switch (*bp++) {

	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '-':
		break;

	case '8': case '9':
		base10 = 1;
		break;

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		base16 = 1;
		break;
	
	case 'x': case 'X':
		*--bp = '0';
		base16 = 1;
		break;

	case 'h': case 'H':
		base16 = 1;
		/* fall into */

	default:
		*--bp = 0; /* Ends Loop */
	}
	if (base16) {
		fmt = "%3x";
		ibase = 4096;
	} else if (base10 == 0 && *buf == '0') {
		fmt = "%3o";
		ibase = 512;
	} else {
		fmt = "%3d";
		ibase = 1000;
	}

	for (bp = buf; *bp++; ) clen++;
	if (clen == 0) clen++;
	if (clen > 18) clen = 18;
	i = ((clen - 1) / 3) + 1;
	bp = clen + buf - 3;
	hp = hb + i - 1;

	while (hp > hb) {
		sscanf(bp, fmt, hp);
		bp[0] = 0;
		hp--;
		bp -= 3;
	}
	sscanf(buf, fmt, hp);
	cvtbase(ibase, 256, hb, i, out, len);
}

static
cvtbase(oldbase,newbase,input,inlen,result,reslen)
	int oldbase, newbase;
	int input[];
	int inlen;
	unsigned char result[];
	int reslen;
{
	int d, e;
	long sum;

	e = 1;
	while (e > 0 && reslen > 0) {
		d = 0; e = 0; sum = 0;
		/* long division: input=input/newbase */
		while (d < inlen) {
			sum = sum*oldbase + (long) input[d];
			e += (sum > 0);
			input[d++] = sum / newbase;
			sum %= newbase;
		}
		result[--reslen] = sum;	/* accumulate remainder */
	}
	for (d=0; d < reslen; d++)
		result[d] = 0;
}
