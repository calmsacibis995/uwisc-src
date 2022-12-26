#include <ctype.h>

isblank( str )	/* true if the argument is a null or whitespace filled string */
register char *str;
{
	while( *str && isspace(*str) )
		str++;
	return( *str == '\0' );
}
