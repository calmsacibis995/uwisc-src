/*
 *	Copyright 1985, Board of Regents of the University of Wisconsin.
 *	All rights reserved.
 *
 * find a string anywhere in another string and ignore case.
 * Originally by Tom Christiansen -- modified by Dave Cohrs to be more
 * general and efficient.
 * returns a pointer to where target appears in source -- NULL if no match.
 * If target is NULL, the start of source is returned.
 */

#define LC(x)	(((x) >= 'A' && (x) <= 'Z') ? (x)-'A'+'a' : (x))

char *
iinstr(source,target)
	register char *source,*target;
{
	register char *oldtarg = target;
	register char *oldsrc = source-1;
	register char *rval = source;

	for (;*target;) {
		if ( *source == '\0')
			return (char *) 0;

		source = ++oldsrc;	/* move to next starting point */
		target = oldtarg;

		if ( *source != *target ) {	/* find a starting match */
			while ( *source && (LC(*source) != LC(*target)) ) 
				source++;
			oldsrc = source-1;	/* efficiency */
			continue;	/* to catch case where we hit the end */
		}

		rval = source;
		while ( *source && *target && (LC(*source) == LC(*target))) {
			target++;
			source++;
		}
	}
	return rval;
}
