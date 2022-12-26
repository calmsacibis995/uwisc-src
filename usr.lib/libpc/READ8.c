/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)READ8.c 1.9 11/6/83";

#include "h00vars.h"
#include <errno.h>
extern int errno;

double
READ8(curfile)
	register struct iorec	*curfile;
{
	double			data;
	int			retval;

	if (curfile->funit & FWRITE) {
		ERROR("%s: Attempt to read, but open for writing\n",
			curfile->pfname);
		return;
	}
	UNSYNC(curfile);
	errno = 0;
	retval = readreal(curfile, &data);
	if (retval == EOF) {
		ERROR("%s: Tried to read past end of file\n", curfile->pfname);
		return;
	}
	if (retval == 0) {
		ERROR("%s: Bad data found on real read\n", curfile->pfname);
		return;
	}
	if (errno == ERANGE) {
		if (data == 0.0)
			ERROR("%s: Underflow on real read\n", curfile->pfname);
		else
			ERROR("%s: Overflow on real read\n", curfile->pfname);
		return;
	}
	if (errno != 0) {
		PERROR("Error encountered on real read ", curfile->pfname);
		return;
	}
	return (data);
}

/*
 *	given a file pointer, read a sequence of characters of the
 *	syntax of section 6.1.5 and form them into a double.
 *
 *	the syntax of a signed-real is:
 *	    [-|+] digit {digit} [ . digit {digit} ] [ e [+|-] digit {digit} ]
 *
 *	returns:
 *		1	for success (with value in *doublep)
 *		0	on error (with *doublep unchanged)
 *	       -1	on end-of-file during read (with *doublep unchanged)
 *	side effects:
 *	      errno	may be set to ERANGE if atof() sets it.
 */
readreal(curfile, doublep)
	struct iorec	*curfile;
	double		*doublep;
{
	FILE	*filep = curfile->fbuf;	/* current file variable */
	char	*sequencep;		/* a pointer into sequence */
	int	read;			/* return value from fscanf() */
	char	sequence[BUFSIZ];	/* the character sequence */
	double	atof();

#define PUSHBACK(curfile, sequencep) \
	if (ungetc(*--(sequencep), (curfile)->fbuf) != EOF) { \
		*(sequencep) = '\0'; \
	} else if ((curfile)->funit & SYNC) { \
		(curfile)->funit &= ~SYNC; \
		*(curfile)->fileptr = *(sequencep); \
		*(sequencep) = '\0'; \
	} else { \
		return (0); \
	}

#define	RETURN_ON_EOF(read) \
	if (read == EOF) \
		return (EOF); \
	else \
		/* void */;

#define	PUSH_TO_NULL(sequencep) \
	while (*sequencep) \
		sequencep++;

	/* general reader of the next character */
#define	NEXT_CHAR(read, filep, format, sequencep) \
	read = fscanf(filep, "%c", sequencep); \
	RETURN_ON_EOF(read); \
	*++sequencep = '\0';

	/* e.g. use %[0123456789] for {digit}, and check read */
#define	SOME(read, filep, format, sequencep) \
	read = fscanf(filep, format, sequencep); \
	RETURN_ON_EOF(read); \
	PUSH_TO_NULL(sequencep);

	/* e.g. use %[0123456789] for digit {digit} */
#define	AT_LEAST_ONE(read, filep, format, sequencep) \
	read = fscanf(filep, format, sequencep); \
	RETURN_ON_EOF(read); \
	if (strlen(sequencep) < 1) \
		return (0); \
	PUSH_TO_NULL(sequencep);

#define	ANY_ONE_OF(read, filep, format, sequencep) \
	read = fscanf(filep, format, sequencep); \
	RETURN_ON_EOF(read); \
	if (strlen(sequencep) != 1) \
		return (0); \
	PUSH_TO_NULL(sequencep);

#define	AT_MOST_ONE(read, filep, format, sequencep) \
	read = fscanf(filep, format, sequencep); \
	RETURN_ON_EOF(read); \
	if (strlen(sequencep) > 1) \
		return (0); \
	PUSH_TO_NULL(sequencep);
		
	sequencep = &sequence[0];
	*sequencep = '\0';
	/*
	 *	skip leading whitespace
	 */
	SOME(read, filep, "%*[ \t\n]", sequencep);
	/*
	 *	this much is required:
	 *	[ "+" | "-" ] digit {digits}
	 */
	AT_MOST_ONE(read, filep, "%[+-]", sequencep);
	AT_LEAST_ONE(read, filep, "%[0123456789]", sequencep);
	/*
	 *	any of this is optional:
	 *	[ `.' digit {digit} ] [ `e' [ `+' | `-' ] digit {digits} ]
	 */
	NEXT_CHAR(read, filep, "%c", sequencep);
	switch (sequencep[-1]) {
	default:
		PUSHBACK(curfile, sequencep);
		goto convert;
	case '.':
		SOME(read, filep, "%[0123456789]", sequencep);
		if (!read) {
			PUSHBACK(curfile, sequencep);
			goto convert;
		}
		NEXT_CHAR(read, filep, "%c", sequencep);
		if (sequencep[-1] != 'e') {
			PUSHBACK(curfile, sequencep);
			goto convert;
		}
		/* fall through */
	case 'e':
		NEXT_CHAR(read, filep, "%c", sequencep);
		if (sequencep[-1] != '+' && sequencep[-1] != '-') {
			PUSHBACK(curfile, sequencep);
			SOME(read, filep, "%[0123456789]", sequencep);
			if (!read)
				PUSHBACK(curfile, sequencep);
			goto convert;
		}
		SOME(read, filep, "%[0123456789]", sequencep);
		if (!read) {
			PUSHBACK(curfile, sequencep);
			PUSHBACK(curfile, sequencep);
		}
	}

convert:
	/*
	 * convert sequence to double
	 */
	*doublep = atof(&sequence[0]);
	return (1);
}
