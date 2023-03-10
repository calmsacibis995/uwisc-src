#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	"$Header: getenv.c,v 1.3 86/09/08 14:42:51 tadl Exp $";
#endif
/*
 * RCS info
 *	$Locker:  $
 */
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getenv.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

/*
 *	getenv(name)
 *	returns ptr to value associated with name, if any, else NULL
 */
#define NULL	0
extern	char **environ;
char	*nvmatch();

char *
getenv(name)
register char *name;
{
	register char **p = environ;
	register char *v;

	while (*p != NULL)
		if ((v = nvmatch(name, *p++)) != NULL)
			return(v);
	return(NULL);
}

/*
 *	s1 is either name, or name=value
 *	s2 is name=value
 *	if names match, return value of s2, else NULL
 *	used for environment searching: see getenv
 */

static char *
nvmatch(s1, s2)
register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return(s2);
	if (*s1 == '\0' && *(s2-1) == '=')
		return(s2);
	return(NULL);
}
