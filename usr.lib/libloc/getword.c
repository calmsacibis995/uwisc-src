#include <ctype.h>

char *					/* return pointer to char after extracted word */
getword( line, word )	/* extract a whitespace deliniated word from line */
register char *line;	/* from string */
register char *word;	/* to buffer */
{
	while(isspace(*line))
		line++;
	while(*line && !isspace(*line))
		*word++ = *line++;
	*word = '\0';

	return line;
}
