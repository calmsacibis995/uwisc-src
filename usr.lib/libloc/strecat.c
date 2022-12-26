char *
strecat(s1,s2)
register char *s1,*s2;
{
	while(*s1)
		s1++;
	while(*s1++ = *s2++)
		;
	return( --s1 );
}

char *
strecpy(s1,s2)
register char *s1,*s2;
{
	while(*s1++ = *s2++)
		;
	return( --s1 );
}
