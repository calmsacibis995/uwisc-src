#include <ctype.h>

/*	case indepentent compare */
#define to_low(a) (((a)>='A'&&(a)<='Z')?((a)|040):(a))

stricmp(s1,s2)
register char *s1, *s2;
{

	while (to_low(*s1) == to_low(*s2)) {
		s2++;
		if (*s1++=='\0')
			return(0);
	}
	return(to_low(*s1) - to_low(*s2));
}

/*
 * Compare strings (at most n bytes):  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

strincmp(s1, s2, n)
register char *s1, *s2;
register n;
{

	while (--n >= 0 && to_low(*s1) == to_low(*s2)) {
		s2++;
		if (*s1++ == '\0')
			return(0);
	}
	return(n<0 ? 0 : to_low(*s1) - to_low(*s2) );
}
