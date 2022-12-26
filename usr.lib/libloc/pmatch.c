#define MATCH		0
#define NO_MATCH	1
#define CONTINUE	-1

static int	meta_found = 0;
static int	brkt_err = 0;

pmatch(str, pat)	/* does pat match str? using standard shell metas in pat */
char *str, *pat;
{
	register int range_cc, str_cc, in_range;
	int c, low_lim;
	int	answer;

	str_cc = *str;
	low_lim = 077777;
	switch (c = *pat) {
	case '[':
		in_range = 0;
		meta_found = 1;
		while (range_cc = *++pat) {
			if( range_cc == ']' ) {
				if( in_range )
					answer = pmatch( ++str, ++pat );
				else
					answer = NO_MATCH;
				break;
			}
			else if( range_cc == '-' ) {
				if( low_lim <= str_cc  & str_cc <= pat[1] )
					in_range++;
				if( pat[1] != ']' )
					range_cc = pat[1];
			}
			if (str_cc==(low_lim=range_cc)) 
				in_range++;
		}
		if( range_cc != ']' ) {
			brkt_err = 1;
			answer = NO_MATCH;
		}
		break;
	case '?':
		meta_found = 1;
		if(str_cc) 
			answer = pmatch( ++str, ++pat );
		else
			answer = CONTINUE;
		break;
	case '*':
		meta_found = 1;
		answer = star_match( str, ++pat );
		break;
	case 0:
		if( !str_cc )
			answer = MATCH;
		else
			answer = NO_MATCH;
		break;
	default:
		if(str_cc == c) 
			answer = pmatch( ++str, ++pat );
		else if(str_cc < c )
			answer = CONTINUE;
		else
			answer = NO_MATCH;
		break;
	}

	if( answer != MATCH )
		answer = meta_found?CONTINUE:answer;
	meta_found = 0;
	brkt_err = 0;
	return answer;
}

static
star_match(str, pat)
char *str, *pat;
{
	if(*pat==0) 
		return(MATCH);
	while(*str)
		if( pmatch(str++,pat) == MATCH ) 
			return(MATCH);
	return brkt_err?NO_MATCH:CONTINUE;
}
