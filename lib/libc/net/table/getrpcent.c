#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: getrpcent.c,v 1.2 86/09/08 14:47:04 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static  char sccsid[] = "@(#)getrpcent.c 1.1 86/02/03  Copyr 1984 Sun Micro";
/* @(#)getrpcent.c	2.1 86/04/11 NFSSRC */
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rpcsvc/ypclnt.h>

/*
 * Internet version.
 */
#define	MAXALIASES	35
#define	MAXADDRSIZE	14

static char domain[256];
static int stayopen;
static char *current = NULL;	/* current entry, analogous to hostf */
static int currentlen;
static struct rpcent *interpret();
struct hostent *gethostent();
char *inet_ntoa();
static char *any();
static char RPCDB[] = "/etc/rpc";
static FILE *rpcf = NULL;
static int usingyellow;		/* are yellow pages up? */

struct rpcent *
getrpcbynumber(number)
	register int number;
{
	register struct rpcent *p;
	int reason;
	char adrstr[10], *val;
	int vallen;

	setrpcent(0);
	while (p = getrpcent()) {
		if (p->r_number == number)
			break;
	}
	endrpcent();
	return (p);
}

struct rpcent *
getrpcbyname(name)
	char *name;
{
	struct rpcent *rpc;
	char **rp;

	setrpcent(0);
	while(rpc = getrpcent()) {
		if (strcmp(rpc->r_name, name) == 0)
			return (rpc);
		for (rp = rpc->r_aliases; *rp != NULL; rp++) {
			if (strcmp(*rp, name) == 0)
				return (rpc);
		}
	}
	endrpcent();
	return (NULL);
}

setrpcent(f)
	int f;
{
	if (rpcf == NULL)
		rpcf = fopen(RPCDB, "r");
	else
		rewind(rpcf);
	if (current)
		free(current);
	current = NULL;
	stayopen |= f;
}

endrpcent()
{
	if (current && !stayopen) {
		free(current);
		current = NULL;
	}
	if (rpcf && !stayopen) {
		fclose(rpcf);
		rpcf = NULL;
	}
}

struct rpcent *
getrpcent()
{
	struct rpcent *hp;
	int reason;
	char *key, *val;
	int keylen, vallen;
	static char line1[BUFSIZ+1];

	if (rpcf == NULL && (rpcf = fopen(RPCDB, "r")) == NULL)
		return (NULL);
		if (fgets(line1, BUFSIZ, rpcf) == NULL)
		return (NULL);
	return interpret(line1, strlen(line1));
}

static struct rpcent *
interpret(val, len)
{
	static char *rpc_aliases[MAXALIASES];
	static struct rpcent rpc;
	static char line[BUFSIZ+1];
	char *p;
	register char *cp, **q;

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	if (*p == '#')
		return (getrpcent());
	cp = any(p, "#\n");
	if (cp == NULL)
		return (getrpcent());
	*cp = '\0';
	cp = any(p, " \t");
	if (cp == NULL)
		return (getrpcent());
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
	rpc.r_name = line;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	rpc.r_number = atoi(cp);
	q = rpc.r_aliases = rpc_aliases;
	cp = any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &rpc_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&rpc);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
