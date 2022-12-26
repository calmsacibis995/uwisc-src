/*
** Just copy (nbytes) raw bytes from src to dest.
*/
copy( dest, src, nbytes )
register char	*dest;
register char	*src;
register int	nbytes;
{
	while( nbytes-- )
		*dest++ = *src++;
}

