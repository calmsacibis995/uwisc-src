#include <ctype.h>

/*
 *	Get rid of trailing spaces (and newlines and tabs ), by moving in the null;
 *	as well as beginning ones, by returning a pointer interior to the string.
 */
char *
ltrunc(str)
register char *str;
{
	register char * ptr;

	ptr = str;

	while(*ptr)ptr++;
	while(isspace(*--ptr))
		if(ptr < str)
			break;
	ptr[1] = '\0';
	while(isspace(*str))str++;

	return(str);
}
