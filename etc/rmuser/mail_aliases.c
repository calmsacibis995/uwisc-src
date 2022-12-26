mail_aliases()
{
	FILE	*pipe, *popen();
#ifdef BSD4_2
#define HOSTLEN 20
	char	hostname[HOSTLEN];
#endif BSD4_2

	if((pipe = popen(ALIAS_CMD, "w")) == NULL) {
		printf("Error could not 'mail aliases'. Do it yourself!\n");
		quit(5);
	}
	fprintf(pipe, "To: aliases\n");
	fprintf(pipe, "Subject: Rmuser - %s %s\n\n", first, last);
	fprintf(pipe, "Please remove the mail alias for %s %s.\n", first, last);
#ifdef BSD4_2
	gethostname(hostname, HOSTLEN);
	fprintf(pipe, "Login name is %s@%s.\n", logname, hostname);
#else
	fprintf(pipe, "Login name is %s@%s.\n", logname, thisname());
#endif BSD4_2
	fprintf(pipe, "\t\t\tThe adduser program\n.\n");
	pclose(pipe);
}
