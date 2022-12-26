/* ascii to number given the base as a parameter */
/*	only good up to and including base 16 */
#include <ctype.h>


atob(str, base)
register char *str;
register int base;
{
	static char char_offset[] = "0123456789abcdef";
	int neg = 0;
	register int n = 0;
	register int offset;
	char *index();

	while(isspace(*str))	/* skip leading white space */
		str++;
	if(*str == '-') {	/* collect any sign */
		neg++;
		str++;
	} else if( *str == '+')
		str++;
	while(isspace(*str))	/* skip interior white space */
		str++;

	while(*str) {
		offset = index(char_offset,*str) - char_offset;
		if(offset < 0 || offset >= base)
			break;
		n = n*base + offset;
		str++;
	}
	return neg ? -n : n ;
}
