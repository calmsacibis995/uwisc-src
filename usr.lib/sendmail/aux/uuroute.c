/*
 * uuroute: this program is similar to uux but first routes all
 * addresses according to the table.  Then it invokes uux.
 *
 * If ASSUMENEW is true, it will assume that all neighbors it sends
 * to understand internet format, and it won't send more than one
 * hop away.
 *
 * If SMARTERNEIGHBOR is defined, all sites that are not locally
 * adjacent will be passed off to the smarter neighbor.
 */

#define _DEFINE
#ifdef DUMB
#define SMARTERNEIGHBOR "uwvax"	/* the smarter neighbor */
#endif DUMB

#include "sendmail.h"

char *hosts[512];
char *rmtaddrs[512];

char *UUX = "/usr/bin/uux";

#define RMAIL "rmail"

char *routingfile = "/usr/uw/lib/palias";
char *lockfile = "/tmp/pathalias.lock";

char *inputtemp = "/tmp/uurouteXXXXXX";

int alreadydone = 0;

#define resetarg() newargc = resetargc
#define setarg(val) (newargv[newargc++] = (val))
int  newargc = 0;
int  resetargc = 0;
char *newargv[512];

char *map();
char *hostalias();
char *aliaslookup();
int pstrcmp();

#ifdef DBM
typedef struct
{
	char	*dptr;
	int	dsize;
} DATUM;
extern DATUM fetch();
#endif DBM

int exitstat = 0;

main(argc, argv)
int argc;
char **argv;
{
	register int i, j, k;
	register char *p;
	int s;
	char buf[512];
	int nhosts;

	initaliases(routingfile, 0);

	if (argv[0][0] == '.' && argv[0][1] == '/')	/* ./uuroute */
		Verbose = 1;

	nhosts = 0;
	resetarg();
	setarg(UUX);
	setarg("-");
	for (i=1; i<argc; i++) {
		if (*argv[i] == '-') {
			setarg(argv[i]);
		} else {
			p = map(argv[i]);
			hosts[nhosts++] = p;
		}
	}

	resetargc = newargc;
	
	if (Verbose)
		for (i=0; i<nhosts; i++)
			printf("hosts %d, val '%s'\n", i, hosts[i]);
	
	qsort(hosts, nhosts-1, sizeof hosts[0], pstrcmp);

	if (Verbose) {
		printf("after sort\n");
		for (i=0; i<nhosts; i++)
			printf("hosts %d, val '%s'\n", i, hosts[i]);
	}

	for (i=0; i<nhosts; i++) {
		p = index(hosts[i], '!');
		if (p) {
			*p++ = 0;
			rmtaddrs[i] = p;
			parenthesize(rmtaddrs[i]);
		} else {
			rmtaddrs[i] = hosts[i];
			hosts[i] = "";
		}
	}

	if (Verbose) {
		for (i=0; i<nhosts; i++)
			printf("hosts %d, host '%s', rmt '%s'\n", i, hosts[i], rmtaddrs[i]);
	}

	for (i=0; i<nhosts; i++) {
		for (j=i+1; j<nhosts; j++)
			if (strcmp(hosts[i], hosts[j]))
				break;
		resetarg();
		if (Verbose)
			printf("loop i %d, nhosts %d, j %d\n", i, nhosts, j);
		sprintf(buf, "%s!%s", hosts[i], RMAIL);
		setarg(buf);
		for (k=i; k<j; k++)
			setarg(rmtaddrs[k]);
		setarg((char *) 0);
		if (Verbose) {
			printf("exec ");
			for (k=0; newargv[k]; k++)
				printf("'%s' ", newargv[k]);
			printf("\n");
		}
		if (nhosts > 1 && !alreadydone) {
			register int fd, n;
			char buf[BUFSIZ];
			fd = creat(inputtemp, 0600);
			while ((n=read(0,buf,BUFSIZ)) > 0)
				write(fd,buf,n);
			close(fd);
			alreadydone = 1;
		}

		if (j >= nhosts || fork() == 0) {
			/* Redirect our stdin to point to the file. */
			if (nhosts > 1) {
				close(0);
				open(inputtemp, 0);
				if (j >= nhosts)
					unlink(inputtemp);
			}
			execv(UUX, newargv);
		} else
			wait(&s);
		i = j-1;
	}
	if (Verbose)
		printf("fell out bottom of loop\n");

	execv(UUX, hosts);
	perror(UUX);
}

char *
map(path)
register char *path;
{
	register char *user, *result, *p;
	char buf[MAXNAME+2];

	user = index(path, '!');
	if (user == NULL)
		return path;
	
	*user++ = '\0';

	/* Now path is the host, and user is the user name, in host!user */
	result = hostalias(path, user);

#ifdef SMARTERNEIGHBOR
	if (result == NULL) {
		sprintf(buf, "%s!%s@%s.UUCP", SMARTERNEIGHBOR, user, path);
		if (Verbose)
			printf("pass off to smarter neighbor, hisname %s, user %s, returning %s\n", SMARTERNEIGHBOR, user, buf);
			return newstr(buf);
	}
#endif SMARTERNEIGHBOR
#ifdef ASSUMENEW
	p = index(result, '!');
	if (p)
		*p = '\0';
	sprintf(buf, "%s!%s", result, user);
	if (Verbose)
		printf("assume new, result %s, user %s, returning %s\n", result, user, buf);
	return newstr(buf);
#else ASSUMENEW
	return result;
#endif ASSUMENEW
}

/*
**  HOSTALIAS -- alias a host name
**
**	Given a host name, look it up in the host alias table
**	and return it's value.  If nothing, return NULL.
**
**	Parameters:
**		a -- address to alias.
**
**	Returns:
**		text result of aliasing.
**		NULL if none.
**
**	Side Effects:
**		none.
*/

char *
hostalias(host, user)
	register char *host, *user;
{
	char buf[MAXNAME+3];
#ifdef UW
	char buf2[MAXNAME];
#endif UW
	register char *p;

	strcpy(buf, host);
	makelower(buf);

	p = aliaslookup(buf);
#ifdef UW
	/* look for domain names */
	if (p == NULL) {
		register char* s;
		s = buf;
		while((s = index(s, '.')) != NULL) {
			p = aliaslookup(s++);
			if(p != NULL) {
				(void) sprintf(buf2, "%s!%s", host, user);
				user = buf2;
				break;
			}
		}
	}
#endif UW
	if (Verbose)
		printf("hostalias: lookup(%s): '%s'\n", buf, p ? p : "nothing");
	if (p == NULL) {
#ifdef SMARTERNEIGHBOR
		return NULL;
#else
		sprintf(buf, "%s!%s", host, user);
		return (newstr(buf));
#endif
	}
	(void) sprintf(buf, p, user);
	return (newstr(buf));
}

/*
 * The following code is adapted from alias.c, in order not to draw in
 * the whole world.
 */

# include <pwd.h>
# include <sys/stat.h>

# ifdef DBM
SCCSID(@(#)alias.c	3.30		2/15/82	(with DBM));
# else DBM
SCCSID(@(#)alias.c	3.30		2/15/82	(without DBM));
# endif DBM


/*
**  ALIASLOOKUP -- look up a name in the alias file.
**
**	Parameters:
**		name -- the name to look up.
**
**	Returns:
**		the value of name.
**		NULL if unknown.
**
**	Side Effects:
**		none.
**
**	Warnings:
**		The return value will be trashed across calls.
*/

char *
aliaslookup(name)
	char *name;
{
# ifdef DBM
	DATUM rhs, lhs;

	/* create a key for fetch */
	lhs.dptr = name;
	lhs.dsize = strlen(name) + 1;
	rhs = fetch(lhs);
	return (rhs.dptr);
# else DBM
	register STAB *s;

	s = stab(name, ST_ALIAS, ST_FIND);
	if (s == NULL)
		return (NULL);
	return (s->s_alias);
# endif DBM
}
/*
**  INITALIASES -- initialize for aliasing
**
**	Very different depending on whether we are running DBM or not.
**
**	Parameters:
**		aliasfile -- location of aliases.
**		init -- if set and if DBM, initialize the DBM files.
**
**	Returns:
**		none.
**
**	Side Effects:
**		initializes aliases:
**		if DBM:  opens the database.
**		if ~DBM: reads the aliases into the symbol table.
*/

# define DBMMODE	0666

initaliases(aliasfile, init)
	char *aliasfile;
	bool init;
{
	char buf[MAXNAME];
	struct stat stb;
	time_t modtime;

	/*
	**  See if the DBM version of the file is out of date with
	**  the text version.  If so, go into 'init' mode automatically.
	**	This only happens if our effective userid owns the DBM
	**	version or if the mode of the database is 666 -- this
	**	is an attempt to avoid protection problems.  Note the
	**	unpalatable hack to see if the stat succeeded.
	*/

	if (stat(aliasfile, &stb) < 0)
		return;
# ifdef DBM
	modtime = stb.st_mtime;
	(void) strcpy(buf, aliasfile);
	(void) strcat(buf, ".pag");
	stb.st_ino = 0;
	if ((stat(buf, &stb) < 0 || stb.st_mtime < modtime) && !init)
	{
		/* Create database first time, if we can. */
		if (stb.st_ino == 0) {
			int i;
			i = creat(buf, 0666);
			if (i > 0) {
				close(i);
				chmod(buf, 0666);
				stat(buf, &stb);
			}
		}
		/* Be fussy about whether we'll rebuild automatically. */
		if (stb.st_ino != 0 &&
		    ((stb.st_mode & 0666) == 0666 || stb.st_uid == geteuid()))
		{
			init = TRUE;
			if (Verbose)
				message("rebuilding alias database");
		}
		else
		{
			if (Verbose)
				message("Warning: alias database out of date");
		}
	}
# endif DBM

	if (stat(lockfile, &stb) == 0) {
		/* someone else is initializing the database */
		/* give them an hour to do it */
		if (stb.st_mtime+3600 > time((long *)0))
			exit(EX_TEMPFAIL);
		message("mtime %d, time %d\n",stb.st_mtime+3600,time(0));
		init++;
	}

	/*
	**  If initializing, create the new files.
	*/

# ifdef DBM
	if (init)
	{

		close(creat(lockfile,0));

		(void) strcpy(buf, aliasfile);
		(void) strcat(buf, ".dir");
		if (close(creat(buf, DBMMODE)) < 0)
		{
			syserr("cannot make %s", buf);
			return;
		}
		(void) strcpy(buf, aliasfile);
		(void) strcat(buf, ".pag");
		if (close(creat(buf, DBMMODE)) < 0)
		{
			syserr("cannot make %s", buf);
			return;
		}
	}

	/*
	**  Open and, if necessary, load the DBM file.
	**	If running without DBM, load the symbol table.
	*/

	dbminit(aliasfile);
	if (init)
		readaliases(aliasfile, TRUE);
# else DBM
	readaliases(aliasfile, init);
# endif DBM
	unlink(lockfile);
}
/*
**  READALIASES -- read and process the alias file.
**
**	This routine implements the part of initaliases that occurs
**	when we are not going to use the DBM stuff.
**
**	Parameters:
**		aliasfile -- the pathname of the alias file master.
**		init -- if set, initialize the DBM stuff.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Reads aliasfile into the symbol table.
**		Optionally, builds the .dir & .pag files.
*/

static
readaliases(aliasfile, init)
	char *aliasfile;
	bool init;
{
	char line[BUFSIZ];
	register char *p;
	char *lhs, *rhs;
	bool skipping;
	FILE *af;
	int lineno;
	register STAB *s;
	int naliases, bytes, longest;

	if ((af = fopen(aliasfile, "r")) == NULL)
	{
		errno = 0;
		NoAlias++;
		return;
	}

	/*
	**  Read and interpret lines
	*/

	lineno = 0;
	naliases = bytes = longest = 0;
	skipping = FALSE;
	while (fgets(line, sizeof (line), af) != NULL) {
		int lhssize, rhssize;

		lineno++;
		switch (line[0])
		{
		  case '#':
		  case '\n':
		  case '\0':
			skipping = FALSE;
			continue;

		  case ' ':
		  case '\t':
			if (!skipping)
				syserr("uubang: %d: Non-continuation line starts with space", lineno);
			skipping = TRUE;
			continue;
		}
		skipping = FALSE;


		/*
		**  Process the LHS
		**	Find the separating tab, and parse the address.
		*/

		lhs = line;
		while (isspace(*lhs))
			lhs++;
		if (*lhs == '/')
			lhs++;
		if (*lhs == '\0') {
		nosite:
			syserr("uuroute: %d: missing site and path", lineno);
			continue;
		}

		for (p = lhs; *p!='\0' && *p!='\t' && *p!=' ' && *p!='\n'; p++)
			continue;
		if (*p == '\0' || *p == '\n')
			goto nopath;
		*p++ = '\0';

		/*
		**  Skip the path cost
		**  Find the separating tab, and skip over the tab.
		*/

		while (isspace(*p)) {
			if (*p == '\n')
				goto nosite;
			p++;
		}
		for (; *p!='\0' && *p!='\t' && *p!=' ' && *p!='\n'; p++)
			continue;
		if (*p == '\0' || *p == '\n')
			goto nosite;
		while (isspace(*p)) {
			if (*p == '\n')
				goto nosite;
			p++;
		}

		/*
		**  Process the RHS.
		**	'p' points to the text of the RHS.
		*/

		rhs = p;
		while (isspace(*rhs))
			rhs++;
		if (*p == '\0')
		{
		nopath:
			syserr("uuroute: %d: missing path", lineno);
			continue;
		}
		while (*p)
			p++;
		--p;
		while (isspace(*p))
			--p;
		*++p = 0;

		/*
		**  Insert alias into symbol table or DBM file
		*/

		lhssize = strlen(lhs) + 1;
		rhssize = strlen(rhs) + 1;

# ifdef DBM
		if (init)
		{
			DATUM key, content;

			key.dsize = lhssize;
			key.dptr = lhs;
			content.dsize = rhssize;
			content.dptr = rhs;
			store(key, content);
		}
		else
# endif DBM
		{
			s = stab(lhs, ST_ALIAS, ST_ENTER);
			s->s_alias = newstr(rhs);
		}

		/* statistics */
		naliases++;
		bytes += lhssize + rhssize;
		if (rhssize > longest)
			longest = rhssize;
	}
	(void) fclose(af);
	if (Verbose)
		message("%d aliases, longest %d bytes, %d bytes total",
			naliases, longest, bytes);
}

/*
 * message - shorter version here to avoid sucking in all that other junk.
 */
message(fmt, a, b, c, d, e)
char *fmt;
int a, b, c, d, e;
{
	fprintf(stderr, fmt, a, b, c, d, e);
	putc('\n', stderr);
}

syserr(fmt, a, b, c, d, e)
char *fmt;
int a, b, c, d, e;
{
	message(fmt, a, b, c, d, e);
	exitstat = 1;
}

/*
 * If there are !'s in an argument, uux tries to get smart.
 * We put parens around the arg to turn off the smarts.
 */
parenthesize(p)
register char *p;
{
	char locbuf[256];	/* scratch */
	register char *firstbang;

	firstbang = index(p, '!');
	if (firstbang == NULL)
		return;
	strcpy(locbuf, p);
	*p++ = '(';
	strcpy(p, locbuf);
	strcat(p, ")");
}

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 * This is like libc's strcmp but takes pointers to char *'s,
 * which is what qsort will hand us.
 */
pstrcmp(ps1, ps2)
char **ps1, **ps2;
{
	register char *s1 = *ps1;
	register char *s2 = *ps2;

	while (*s1 == *s2++)
		if (*s1++=='\0') {
			return(0);
		}
	return(*s1 - *--s2);
}
