#include <strings.h>

char *malloc(), *realloc();
extern char **environ;

/*
 * places the given variable/value pair into the environment.
 *
 * "var" must be of the form "VAR=" and "value" may contain anything
 * except nul's.  If value is a NULL ptr, "VAR" is removed from the
 * environment.
 *
 * setenv returns a pointer to the new environment entry.
 * it returns NULL if it can't get memory for the new value.
 */
char *
setenv(var, value)
	char *var, *value;
{
	static int virgin = 1;	/* true while "environ" is a virgin */
	register index = 0;
	register varlen = strlen(var);
	register vallen;
	register char **newenv;

	if(value != (char *) 0)
		vallen = strlen(value);
	else
		vallen = -1;

	/* look for 'var' in the environment, replace it if we find it */
	for (index = 0; environ[index] != (char *) 0; index++) {
		if (strncmp(environ[index], var, varlen) == 0) {
			if(vallen >= 0) {
				/* found it */
				environ[index] = malloc((unsigned) (varlen + vallen + 1));

				if(environ[index] == (char *) 0)
					return (char *) 0;		/* oops!  we killed it */

				(void) strcpy(environ[index], var);
				(void) strcpy(environ[index]+varlen, value);
				return environ[index];
			} else {
				do {
					environ[index] = environ[index+1];
					index++;
				} while(environ[index] != (char *) 0);
				return (char *) 0;
			}
		}
	}

	if(vallen < 0)	/* if not there and they want it removed, already done */
		return (char *) 0;

	/* if environ has never been had, must malloc it's virginity away */
	if(virgin) {
		register i;

		newenv = (char **) malloc((unsigned) (sizeof (char *) * (index + 2)));
		for(i = index; i >= 0; i--)
			newenv[i] = environ[i];

		virgin = 0;	/* you're not a virgin anymore, sweety */
	} else
		/* it's already used, just make it bigger :-) */
		newenv = (char **) realloc((char *) environ,
			(unsigned) (sizeof (char *) * (index + 2)));

	/* find out if we killed it */
	if (newenv == (char **) 0) {
		return (char *) 0;
	}
	environ = newenv;

	/* make space for environ's latest conquest */
	environ[index] = malloc((unsigned) (varlen + vallen + 1));
	(void) strcpy(environ[index], var);
	(void) strcpy(environ[index]+varlen, value);
	environ[index+1] = (char *) 0;

	return environ[index];
}
