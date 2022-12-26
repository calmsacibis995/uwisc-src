#include <ctype.h>

char *
to_lower(string)	/* convert all uppercase chars to lower. returns arg */
char *string;
{
	register char *str = string;
	while(*str) {
		if(isupper(*str))
			*str = tolower(*str);
		str++;
	}
	return( string );
}

char *
to_upper(string)	/* convert all lowercase chars to upper. returns arg */
char *string;
{
	register char *str = string;
	while(*str) {
		if(islower(*str))
			*str = toupper(*str);
		str++;
	}
	return( string );
}
