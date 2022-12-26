#include <ctype.h>

isnumber( str )	/* tell whether the string is all digits, with opt sign */
register char *str;
{
	if(*str == '-' || *str == '+')	/* allow signed numbers */
		str++;
	if(!*str)
		return 0;
	while(*str)
		if(!isdigit(*str++))
			return 0;
	return 1;
}
