#define WORDSIZE	16
#define NULL		(char *)0

char *
strdup(str)	/* return malloc'd copy of argument, or NULL if out of mem */
char *str;
{
	register char *ptr, *start;
	int len;

	len = strlen(str) + 1;
	if(!(start = (char *)malloc( ((len/WORDSIZE)+1)*WORDSIZE ))) {
		return( NULL );
	}
	ptr = start;
	while (*ptr++ = *str++)
		;
	return( start );
}
