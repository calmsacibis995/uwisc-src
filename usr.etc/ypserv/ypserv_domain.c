#ifndef lint
/* @(#)ypserv_domain.c	2.1 86/04/16 NFSSRC */
static	char sccsid[] = "@(#)ypserv_domain.c 1.1 86/02/05 Copyr 1984 Sun Micro";
#endif

/*
 * This contains all yellow pages server code which knows about and manipulates
 * the server's domain data bases.
 */

#include "ypsym.h"

struct domain_list_item *ypdomains_known = NULL;


/*
 * Loads the runtime list of domains served. The elements on the list of 
 * known domains pointed to by global cell ypdomains_known will be have the
 * dom_supported field set TRUE for each supported domain.
 */
void
ypget_supported_domains()
{
	DIR *dirp;
	struct direct *dp;
	char path[MAXNAMLEN + 1];
	struct stat filestat;
	bool found;
	struct domain_list_item *pdom;

	if ( (dirp = opendir(ypdbpath) ) == NULL) {
		fprintf(stderr, "ypserv:  ypget_supported_domains can't open yp data base directory.\n");
		return;
	}

	/*
	 * For each directory entry, stat the associated file.  If the file is a
	 * directory, check the list of known domains, and, if found, mark the
	 * domain as supported.  Directories "." and ".." will be filtered out,
	 * therefore aren't valid domain names.  Directories which are not on
	 * the list of known domains will be skipped.  Domains which are already
	 * supported will be skipped in this pass.  Domains which contain at
	 * least the map of maps in this domain (ypmaps at the time of this
	 * writing) will be marked as supported.  In addition, a check for the
	 * map of peer servers (ypservers) and the mapping of host names to
	 * IP addresses (hosts.byname) will be made.  Map list entries will be
	 * made for each of the required/desired maps.
	 */

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp) ) {
		strcpy(path, ypdbpath);
		strcat(path, "/");
		strcat(path, dp->d_name);

		if (stat(path, &filestat) != -1) {

			if ( (filestat.st_mode & S_IFDIR) &&
			    (strcmp(dp->d_name, ".") ) &&
			    (strcmp(dp->d_name, "..") ) ) {

				if (pdom = yppoint_at_domain(dp->d_name) ) {

					if (pdom->dom_supported) {
						continue;
					}

					/*
					 * Provisionally say the domain's
					 * supported, then check to see if the
					 * required maps did get supported and
					 * decide on a real status for the
					 * domain.
					 */

					pdom->dom_supported = TRUE;
					(void) ypadd_named_map(YPMAPS, pdom);
					(void) ypadd_named_map(YPSERVERS, pdom);
					(void) ypadd_named_map(YPHOSTS_BYNAME, pdom);

					if (!ypcheck_map(YPMAPS, pdom->dom_name) ) {
						pdom->dom_supported = FALSE;
					}
				}
			}

		} else {
			fprintf(stderr, "ypserv:  ypget_supported_domains can't stat domain file %s.\n",
			     dp->d_name);
		}
	}
	closedir(dirp);
}

/*
 * This returns a pointer to the domain_list_item associated with a named
 * domain, or NULL.
 */
struct domain_list_item *
yppoint_at_domain(pname)
	char *pname;
{
	int len;
	struct domain_list_item *plist;

	if ( (pname == (char *) NULL) || ( (len = strlen(pname) ) == 0) ||
	    (len > YPMAXDOMAIN) ) {
		return( (struct domain_list_item *) NULL);
	}

	for  (plist= ypdomains_known; plist != NULL; plist= plist->dom_pnext) {

		if (strcmp(plist->dom_name, pname) == 0) {
			return(plist);
		}
	}

	return( (struct domain_list_item *) NULL);

}

/*
 * This checks to see if a domain is supported.  
 */
struct domain_list_item *
ypcheck_domain(pname)
	char *pname;
{
	int len;
	struct domain_list_item *pdom;

	if (!pname || ((len = strlen(pname)) == 0) || (len > YPMAXDOMAIN) ) {
		return( (struct domain_list_item *) NULL);
	}

	if ( ( (pdom = yppoint_at_domain(pname) ) !=
	    (struct domain_list_item *) NULL) &&
	    (pdom->dom_supported == TRUE) ) {
		return(pdom);
	} else {
		return( (struct domain_list_item *) NULL);
	}

}

/*
 * This points at the first domain_list_item in the list of known domains, or
 * NULL if there are none.  If parameter supportedis TRUE, return the first
 * supported domain; else the first whether or not it is supported.
 */
struct domain_list_item *
yppoint_at_first_domain(supported)
	bool supported;
{
	struct domain_list_item *pdom;

	pdom = ypdomains_known;

	if (! supported) {
		return(pdom);
	}

	while ( (pdom != (struct domain_list_item *) NULL) &&
	    (pdom->dom_supported == FALSE) ) {
		pdom = pdom->dom_pnext;
	}

	return(pdom);
}

/*
 * This points at the next domain_list_item in the list of known domains, or
 * NULL if there are none.
 */
struct domain_list_item *
yppoint_at_next_domain(pcurr, supported)
	struct domain_list_item *pcurr;
	bool supported;
{
	struct domain_list_item *pdom;

	if (!pcurr) {
		return( (struct domain_list_item *) NULL);
	}

	pdom = pcurr->dom_pnext;

	if (! supported) {
		return(pdom);
	}

	while ( (pdom != (struct domain_list_item *) NULL) &&
	    (pdom->dom_supported == FALSE) ) {
		pdom = pdom->dom_pnext;
	}

	return(pdom);
}

/*
 * This returns a ptr to the list of the maps in a supported domain, or NULL if
 * there are none.
 * 
 * Note:  This will return NULL if there are no maps on the list, or if the
 * domain name is bogus, or if the domain name is real but the domain is
 * unsupported.
 */
struct map_list_item *
yppoint_at_maplist(pdom)
	struct domain_list_item *pdom;
{
	if (pdom) {
		return(pdom->dom_pmaplist);
	} else {
		return( (struct map_list_item *) NULL);
	}
}

/*
 * This returns a ptr to the list of the peers in a supported domain, or NULL
 * if there are none.
 *
 * Note:  This will return NULL if there are no peers on the list, or if the
 * domain name is bogus, or if the domain name is real but the domain is
 * unsupported.
 */
struct peer_list_item *
yppoint_at_peerlist(pdom)
	struct domain_list_item *pdom;
{
	if (pdom) {
		return(pdom->dom_ppeerlist);
	} else {
		return( (struct peer_list_item *) NULL);
	}
}

/*
 * This returns a ptr to the domain name string from aninput domain_list_item
 * pointer.
 */
char *
yppoint_at_domname(pdom)
	struct domain_list_item *pdom;

{
	if (pdom) {
		return(pdom->dom_name);
	} else {
		return( (char *) NULL);
	}
}

/*
 * This builds a list of all known domains, using the map ypdomains in the
 * yp_private domain.
 *
 * Note:  This may be called to	reprocess ypdomains after startup:  new domains
 * will get picked up, existing domains will get marked "in the new map".
 */
bool
ypget_all_domains()
{
	char mapname[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
	datum key;

	/*
	 * This function MAY NOT use ypset_current_map, which checks to see
	 * if a domain is supported.  Since this function is used in initially
	 * setting up domains, that won't work.
	 */

	ypmkfilename(ypprivate_domain_name, ypdomains, mapname);

	if (ypcheck_map_existence(mapname) ) {

		if (current_map[0] != '\0') {
			dbmclose(current_map);
			current_map[0] = '\0';
		}

		if (dbminit(mapname) >= 0) {

			strcpy(current_map, mapname);

			for (key = firstkey(); key.dptr != (char *) NULL;
			    key = nextkey(key) ) {

				/*
				 * Knock out key-value pairs from the
				 * map file which are yp private symbols.
				 */

				if (key.dsize >= YPSYMBOL_PREFIX_LENGTH &&
				    (!bcmp(key.dptr, YPSYMBOL_PREFIX,
				    YPSYMBOL_PREFIX_LENGTH) ) ) {
					continue;
				}
					
				ypadd_one_domain(&key);
			}
			
		} else {
			fprintf(stderr, "ypserv:  can't dbminit yp domain map.\n");
			return(FALSE);
		}

	} else {
		fprintf(stderr, "ypserv:  yp domain map not present.\n");
		return(FALSE);
	}

	return(TRUE);
}

/*
 * This builds one domain_list_item containing the domain name in the input
 * datum, marks it "not supported", and links it to the list of known domains
 * as the head entry.  If the domain already exists, dom_in_new_map will be set
 * TRUE, and nothing else will be changed.   Errors are reported here, but no
 * value is returned to the caller.  The domain name in field dom_name will be
 * null terminated, so string functions (such as strcmp, strcpy, etc.) may used
 * on the information.
 */
void
ypadd_one_domain(pdatum)
	datum *pdatum;
{
	struct domain_list_item *pnewdomain;
	char dom_name[YPMAXDOMAIN + 1];

	if (!pdatum || pdatum->dsize == 0 || pdatum->dsize > YPMAXDOMAIN ||
	    (pdatum->dptr == (char *) NULL) ) {
		return;
	}

	bcopy(pdatum->dptr, dom_name, pdatum->dsize);
	dom_name[pdatum->dsize] = '\0';

	if (pnewdomain = yppoint_at_domain(dom_name) ) {
		pnewdomain->dom_in_new_map = TRUE;
		return;
	}

	pnewdomain = (struct domain_list_item *)
	    malloc(sizeof(struct domain_list_item) );

	if (pnewdomain != NULL) {
		strcpy(pnewdomain->dom_name, dom_name);
		pnewdomain->dom_supported = FALSE;
		pnewdomain->dom_in_new_map = TRUE;
		pnewdomain->dom_pmaplist = (struct map_list_item *) NULL;
		pnewdomain->dom_ppeerlist = (struct peer_list_item *) NULL;

		/* Add the domain as the head entry */

		pnewdomain->dom_pnext = ypdomains_known;
		ypdomains_known = pnewdomain;
	} else {
		fprintf(stderr, "ypserv:  Can't get memory for a new domain list entry.\n");
	}

}

/*
 * This finds a domain_list_item associated with a passed domain name on the
 * list of known domains, and removes it from the list.  The list will be
 * well-formed after the call if it was before the call, and the memory
 * associated with the domain_list_item, the domain's map list, and the domain's
 * peer list will be returned to the system.
 */
void
ypdel_one_domain(pdomain)
	char *pdomain;
{
	struct domain_list_item *pdom = ypdomains_known;
	struct domain_list_item *trail;

	if (!pdomain || ypdomains_known == (struct domain_list_item *) NULL) {
	    return;
	}

	pingpeer_curr_domain = (struct domain_list_item *) NULL;
	pingmap_curr_domain = (struct domain_list_item *) NULL;

	if ( strcmp(pdom->dom_name, pdomain) == 0) { /* It's the head entry */
		ypdomains_known = pdom->dom_pnext;
		ypdel_domain_lists(pdom);
		free(pdom);
	} else {				/* Not the head entry */
		trail = pdom;
		pdom = pdom->dom_pnext;
		
		while (pdom != (struct domain_list_item *) NULL) {
			    
			if ( strcmp(pdom->dom_name, pdomain) == 0) {
				trail->dom_pnext  = pdom->dom_pnext;
				ypdel_domain_lists(pdom);
				free(pdom);
				break;
			} else {
				trail = pdom;
				pdom = pdom->dom_pnext;
			}
		}
	}
}

/*
 * This deletes the map list and peer list associated with a domain, returning
 * all memory to the system.
 */
void
ypdel_domain_lists(pdom)
	struct domain_list_item *pdom;
{
	struct map_list_item *pmap;
	struct peer_list_item *ppeer;

	if (!pdom) {
		return;
	}

	for (pmap = yppoint_at_maplist(pdom);
	    pmap != (struct map_list_item *) NULL;
	    pmap = yppoint_at_maplist(pdom) ) {
		    ypdel_one_map(pmap);
	    }
	    
	for (ppeer = yppoint_at_peerlist(pdom);
	    ppeer != (struct peer_list_item *) NULL;
	    ppeer = yppoint_at_peerlist(pdom) ) {
		    ypdel_one_peer(ppeer, pdom);
	    }
	    
}
