
any(ch, str)	/* does character ch occur in string str ? */
register int ch;
register char *str;
{
	while (*str)
		if (ch == *str++)
			return(1);
	return(0);
}

anyof( str1, str2)	/* do the strings str1 and str2 have any chars in common */
char *str1, *str2;
{
	while(*str1)
		if( any(*str1++,str2) )
			return( 1 );
	return( 0 );
}
