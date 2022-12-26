/*
 * RCS Info	
 *	$Header: assert.h,v 1.1 86/09/05 08:02:06 tadl Exp $
 *	$Locker: tadl $
 */
/*	assert.h	4.2	85/01/21	*/

# ifndef NDEBUG
# define _assert(ex)	{if (!(ex)){fprintf(stderr,"Assertion failed: file \"%s\", line %d\n", __FILE__, __LINE__);exit(1);}}
# define assert(ex)	_assert(ex)
# else
# define _assert(ex)
# define assert(ex)
# endif
