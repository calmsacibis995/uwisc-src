#ifndef lint
/* @(#)ypserv_xfr.c	2.1 86/04/16 NFSSRC */
static	char sccsid[] = "@(#)ypserv_xfr.c 1.1 86/02/05 Copyr 1984 Sun Micro";
#endif

/*
 * ypserv_xfr.c
 * This contains functions which manipulate the data structures associated with
 * the transfer of map databases from a peer yp server to this yp server.
 */

#include "ypsym.h"
#include <sys/file.h>

extern void clnt_perror();
extern bool silent;
extern bool log_transfers;		/* Defined and documented in ypserv.c */
int ypclnterr2exit();
char *ypclnterr2string();
void output_rpc_error();

static char logmsg_template[] = "  Map = %s, domain = %s, peer = %s.  Transfer ";
static char log_filename[MAXNAMLEN + 1];
static FILE *log_file = (FILE *) NULL;

/*
 * This allocates memory for a map_xfr_entry, initializes its fields to default
 * values, points it at its associated map, and calls ypenqueue_xfr to stick it
 * on the list of maps to be transferred. The fields within the new
 * map_xfr_entry are set as follows:
 * 	mx_xfr_pid is set to NULL
 * 	mx_xfr_succeeded is set to FALSE
 * 	mx_map is set to pmap
 * 	mx_temp_name will contain the null string
 * The map_xfr_entry will be queued to the map transfer list by lower levels.
 *
 * Note:  This will not add a second map transfer element to the list if one is
 * already there referring to the same map.
 */
bool
ypadd_xfr(pmap)
	struct map_list_item *pmap;
{
	struct map_xfr_entry *pxfr;

	if (!pmap) {
		return(FALSE);
	}

	if (yppoint_at_xfr(pmap)) {
		return(TRUE);
	}
	
	if ( (pxfr = (struct map_xfr_entry *) malloc((sizeof
	    (struct map_xfr_entry) ) ) ) == (struct map_xfr_entry *) NULL) {
		return(FALSE);
	}

	pxfr->mx_xfr_pid = NULL;
	pxfr->mx_map = pmap;
	pxfr->mx_temp_name[0] = '\0';
	pxfr->mx_temp_path[0] = '\0';
	ypenqueue_xfr(pxfr);
	return(TRUE);
}

/*
 * This sticks a map_xfr_entry onto the list of maps to be transferred, making
 * it the tail entry.
 */
void
ypenqueue_xfr(pxfr)
	struct map_xfr_entry *pxfr;
{
	struct map_xfr_entry *scan;

	if (!pxfr) {
		return;
	}

	if (map_xfr_list != (struct map_xfr_entry *) NULL) {

		for (scan = map_xfr_list;
		    scan->mx_pnext != (struct map_xfr_entry *) NULL;
		    scan = scan->mx_pnext) {		/* Null loop */
		}

		scan->mx_pnext = pxfr;
		
	} else {
		map_xfr_list = pxfr;
	}

	pxfr->mx_pnext = (struct map_xfr_entry *) NULL;
}

/*
 * This returns a pointer to a map transfer list entry associated with a given
 * map, or NULL.
 */
struct map_xfr_entry *
yppoint_at_xfr(pmap)
	struct map_list_item *pmap;
{
	struct map_xfr_entry *scan;

	if (!pmap) {
		return( (struct map_xfr_entry *) NULL);
	}

	for (scan = map_xfr_list;
	    ( (scan != (struct map_xfr_entry *) NULL) &&
	      (scan->mx_map != pmap) );
	    scan = scan->mx_pnext) {		/* Null loop */
	}

	return(scan);
}

/*
 * This deletes a map transfer element from the transfer list.
 *
 * Note:  If the map is currently getting transferred, this will null out the
 * mx_map field, but not free the map transfer element.  The transfer-done
 * routine should check for a null map pointer, and should call
 * yprelease_current_xfr to get rid of the map transfer element, and do
 * whatever it needs to do with the transferred map (or the error).
 */
void
ypdel_xfr(pmap)
	struct map_list_item *pmap;
{
	struct map_xfr_entry *pxfr;
	struct map_xfr_entry *scan;

	if (!pmap) {
		return;
	}

	if (current_transfer && (current_transfer->mx_map == pmap) ) {
		current_transfer->mx_map = (struct map_list_item *) NULL;
	}

	/*
	 * The map may have been queued to the transfer list while it was
	 * the current entry, so keep trying to remove it.
	 */

	if ( (pxfr = yppoint_at_xfr(pmap) ) == (struct map_xfr_entry *) NULL) {
		return;				/* It's not on the list */
	}

	/* At this point we know that it is on the list */
	
	if (map_xfr_list == pxfr) {
		map_xfr_list = pxfr->mx_pnext;	/* It's the head entry */
	} else {
		
		for (scan = map_xfr_list;
		   ( (scan != (struct map_xfr_entry *) NULL) &&
	      	      (scan->mx_pnext != pxfr) );
		   scan = scan->mx_pnext) {		/* Null loop */
		}

		scan->mx_pnext = pxfr->mx_pnext;
	}

	free(pxfr);
}

/*
 * This dequeues the head entry from the list of maps to be transfered, and
 * points current_transfer at it.  The state of the map transfer queue will be
 * changed, and current_transfer will point to the old head entry.
 * 
 * Note:  The only case in which this returns FALSE is that in which
 * current_transfer is not NULL when this function is called.  That's  bad....
 */
bool
ypset_current_xfr()
{
  if (current_transfer != (struct map_xfr_entry *) NULL)
  	return(FALSE);

  current_transfer = map_xfr_list;
  map_xfr_list = map_xfr_list->mx_pnext;
  current_transfer->mx_pnext = (struct map_xfr_entry *) NULL;
  return(TRUE);
}

/*
 * This sets current_transfer to NULL, and returns any memory pointed to (that
 * is, a map_xfr_entry) to the system.
 */
void
yprelease_current_xfr()
{
	if (current_transfer) {
		free(current_transfer);
		current_transfer = (struct map_xfr_entry *) NULL;
	}
}

/*
 * This enqueues the element pointed to by current_transfer to the list of maps
 * to be transferred, and sets current_transfer to NULL .
 */
void
yprequeue_current_xfr()
{
	if (current_transfer) {
		ypenqueue_xfr(current_transfer);
		current_transfer = (struct map_xfr_entry *) NULL;
	}
}

/*
 * This shoves the transfer process' pid into the mx_xfr_pid field of the
 * current map transfer entry.
 *
 * Note:  This returns FALSE only if there is no current transfer map.
 */
bool
ypset_current_pid(pid)
	int pid;
{
	if (current_transfer == (struct map_xfr_entry *) NULL)
		return(FALSE);

	current_transfer->mx_xfr_pid = pid;
	return(TRUE);
}

/*
 * This generates a temp map name, and shoves it into the mx_temp_name field of
 * the current map transfer entry.  It also generates a full path name within
 * the defined temporary transfer directory, and puts that name into field
 * mx_temp_path.
 *
 * Note:  This returns FALSE only if there is no current transfer map.
 */
bool
ypset_current_tmpname()
{
  	if (current_transfer == (struct map_xfr_entry *) NULL)
  		return(FALSE);

	ypmk_tmpname(current_transfer->mx_temp_name);
	strcpy(current_transfer->mx_temp_path, YPTEMPDIRECTORY);
	strcat(current_transfer->mx_temp_path, "/");
	strcat(current_transfer->mx_temp_path, current_transfer->mx_temp_name);
  	return(TRUE);
}

/*
 * This does the special processing needed when a new copy of the special map
 * ypdomains in domain yp_private is successfully transferred.  The state of
 * the yp internal data base may be changed.
 *
 * Note:  The initial string comparison on the domain name is to filter for
 * yp_private domain.  Any other domain is not a special case.
 */
void
ypnew_ypdomains(pmap)
	struct map_list_item *pmap;
{
	struct domain_list_item *pdom;
	bool out_of_date_in_list = TRUE;
	
	if (!pmap || strcmp(pmap->map_domain->dom_name, YPPRIVATE_DOMAIN_NAME) ) 
		return;

	/*
	 * Mark all domains as "not in the new map"
	 */

	for (pdom = yppoint_at_first_domain(FALSE);
	    pdom != (struct domain_list_item *) NULL;
	    pdom = yppoint_at_next_domain(pdom, FALSE) ) {
		pdom->dom_in_new_map = FALSE;
	}

	/*
	 * Pick up new domains, mark existing domains which are in the new
	 * map so they will be seen.
	 */

	ypget_all_domains ();

	/*
	 * Throw away domains which were in the old world, but are not in the
	 * new one.
	 */

	while (out_of_date_in_list) {

		out_of_date_in_list = FALSE;
			
		for (pdom = yppoint_at_first_domain(FALSE);
		    pdom != (struct domain_list_item *) NULL;
		    pdom = yppoint_at_next_domain(pdom, FALSE) ) {

			if (!pdom->dom_in_new_map) {
				ypdel_one_domain(pdom->dom_name);
				out_of_date_in_list = TRUE;
				break;
			}
		}
	}

	/*
	 * Find out which of the new domains are supportable.  This will leave
	 * already supported domains unchanged.
	 */
	 
	ypget_supported_domains();
	
	for (pdom = yppoint_at_first_domain(TRUE);
	    pdom != (struct domain_list_item *) NULL;
	    pdom = yppoint_at_next_domain(pdom, TRUE) ) {
		ypbld_dom_peerlist(pdom);
		ypget_dom_all_maps(pdom);
		ypget_dom_supported_maps(pdom);
	}
}

/*
 * This does the special processing needed when a new
 * copy of the special map is ypservers successfully transferred.
 * The state of the yp internal data base may be changed.
 */
void
ypnew_ypservers(pmap)
	struct map_list_item *pmap;
{
	struct peer_list_item *ppeer;
	bool out_of_date_in_list = TRUE;

	if (!pmap) {
		return;
	}

	/*
	 * Mark all peers as "not in the new map"
	 */

	for (ppeer = yppoint_at_peerlist(pmap->map_domain);
	    ppeer != (struct peer_list_item *) NULL;
	    ppeer = ypnext_peer(ppeer) ) {
		ppeer->peer_in_new_map = FALSE;
	}

	/*
	 * Pick up any new peers from the map, recheck all the peer addresses.
	 */
	
	ypbld_dom_peerlist(pmap->map_domain);

	/*
	 * Knock out peers which still show up as "not in the new map", and
	 * NULL-out any references to the peer by the maps within the domain.
	 */
	 
	while (out_of_date_in_list) {

		out_of_date_in_list = FALSE;
			
		for (ppeer = yppoint_at_peerlist(pmap->map_domain);
		    ppeer != (struct peer_list_item *) NULL;
		    ppeer = ypnext_peer(ppeer) ) {

			if (!ppeer->peer_in_new_map) {
				ypdel_one_peer(ppeer, pmap->map_domain);
				out_of_date_in_list = TRUE;
				break;
			}
		}
	}
}

/*
 * This does the special processing needed when a new copy of the special map
 * ypmaps is successfully transferred. The state of the yp internal data base
 * may be changed.
 */
void
ypnew_ypmaps(pmap)
	struct map_list_item *pmap;
{
	struct map_list_item *scan;
	bool out_of_date_in_list = TRUE;

	if (!pmap) {
		return;
	}

	/*
	 * Mark all maps as "not in the new map"
	 */

	for (scan = yppoint_at_maplist(pmap->map_domain);
	    scan != (struct map_list_item *) NULL;
	    scan = yppoint_at_next_map(scan) ) {
		scan->map_in_new_map = FALSE;
	}

	/*
	 * Pick up any new maps, recheck all the map masters
	 */

	ypget_dom_all_maps(pmap->map_domain);

	/*
	 * KO any maps which still show up as not in the new map
	 */
	 
	while (out_of_date_in_list) {

		out_of_date_in_list = FALSE;
			
		for (scan = yppoint_at_maplist(pmap->map_domain);
		    scan != (struct map_list_item *) NULL;
		    scan = yppoint_at_next_map(scan) ) {

			if (!scan->map_in_new_map) {
				ypdel_one_map(scan);
				out_of_date_in_list = TRUE;
				break;
			}
		}
	}

	/*
	 * Check to see if the newly-added maps are supportable.  Maps which
	 * are already supported won't be altered.
	 */

	ypget_dom_supported_maps(pmap->map_domain);

}

/*
 * This does the special processing needed when a new copy of the special map
 * hosts.byname is successfully transferred.  This consists of looking at every
 * peer on the peerlist of the domain, and resetting the internet address.  Just
 * for safety's sake, the port will also be reset to an out-of-range value, to
 * force them to be rechecked at their next use.  The state of the yp internal
 * data base may be changed.
 */
void
ypnew_hosts(pmap)
	struct map_list_item *pmap;
{
	struct peer_list_item *ppeer;

	if (!pmap) {
		return;
	}

	for (ppeer = yppoint_at_peerlist(pmap->map_domain);
	    ppeer != (struct peer_list_item *) NULL;
	    ppeer = ypnext_peer(ppeer) ) {
		ypget_peer_addr(ppeer->peer_pname, pmap->map_domain->dom_name,
		    &(ppeer->peer_addr) );
		ppeer->peer_port = htons( (unsigned short) YPNOPORT);
	    }
}

/*
 * This is the body of the map transfer child process, forked off in ypxfr_map.
 * It uses the data structures pointed to through global cell current_xfr.
 * The main "functional value" of this is a process exit code:  YPXFR_EXIT_SUCC
 * (0) if the transfer was completed successfully, otherwise one of the transfer
 * error codes defined in ypsym.h.  In addition, in the success case, the new
 * map will be created under the temporary name which has been set up in the
 * current transfer entry.
 */
void
ypdo_xfr()
	
{
	struct peer_list_item *ppeer;
	struct map_list_item *pmap;
	unsigned long bin_ordernum_before;
	unsigned long bin_ordernum_after;
	unsigned long bin_ordernum_map;
	unsigned int ypclnt_status;
	bool order_found = FALSE;
	datum key;
	char *inkey;
	int inkeylen;
	datum val;
	char dbm_filename[MAXNAMLEN + 1];
	int dbm_file;
	struct timeval time;
	char *asctime;
	char *tmpptr;
	char logmsg[YPMAXMAP + YPMAXDOMAIN + sizeof(logmsg_template) + 256];

	/*
	 * Figure out whether we should be talking to the master, or we should
	 * try to assign an alternate.  If we can't find a transfer peer,
	 * ppeer will remain NULL.
	 */

	ppeer = (struct peer_list_item *) NULL;
	pmap = current_transfer->mx_map;

	if (pmap->map_master) {

		ypping_peer(pmap->map_master);

		if (pmap->map_master->peer_reachable == TRUE) {
			ppeer = pmap->map_master;
		}
	}

	if (!ppeer) {
		
		if (ypfind_alternate(pmap) ) {
			ppeer = yppoint_at_map_alternate(pmap);
		}
	}

	/*
	 * If we're supposed to be logging transfers, open the log file.
	 * A failure of the fopen/freopen is equivalent to no logging.
	 */
	 
	if (log_transfers) {
		strcpy(log_filename, ypdbpath);
		strcat(log_filename, "/YP_MAP_TRANSFER.LOG");

		if (silent) {
			log_file = fopen(log_filename, "a+");
		} else {
			log_file = freopen(log_filename, "a+", stderr);
		}
	}

	/*
	 * If we are logging, build a timestamp, then write the timestamp and
	 * the first part of the message to the log file.  The message will
	 * be finished at any point at which the process decides to exit.
	 */

	if (log_file) {
		gettimeofday(&time, NULL);

		if (asctime = ctime(&(time.tv_sec) ) ) {
			strcpy(logmsg, asctime);

			for (tmpptr = logmsg;
			    (*tmpptr != '\n') && (*tmpptr != '\0');
			    tmpptr++) {
			}

			*tmpptr = '\0';		/* Replace newline with null */
			    
		} else {
			strcpy(logmsg, "<NO TIMESTAMP>"); 
		}

		strcat(logmsg, logmsg_template);
		fprintf(log_file, logmsg,
		    current_transfer->mx_map->map_name,
		    current_transfer->mx_map->map_domain->dom_name,
		    (ppeer ? ppeer->peer_pname : "<NO PEERNAME>"));
	}

	if (!ppeer) {
		
		if (log_file) {
			fprintf(log_file,
			    "failed:  Can't find transfer partner.\n");
		}
		
		exit(YPXFR_EXIT_PEER);
	}
		
	/*
	 * Bind to the transfering peer.  If we can't do it, exit.  Before we
	 * bind, free any resources allocated in us because of our parent's
	 * current binding.
	 */

	ypclr_xfr_peer();
		
	if (!ypset_xfr_peer(ppeer) ) {
		
		if (log_file) {
			fprintf(log_file,
			    "failed:  Can't set (or bind to) partner.\n");
		}
		
		exit(YPXFR_EXIT_PEER);
	}

	/*
	 * Get the map's order number.  If there are problems, bag the rest of
	 * the work, and exit.  If the peer's version of the map is <= the one
	 * we have already, let's not go through all the work.
	 */

	if (ypclnt_status = yppoll_for_order_number(current_transfer->mx_map,
	    &bin_ordernum_before) ) {
		
		if (log_file) {
			fprintf(log_file,
			    "failed:  First poll for order failed; /n/treason %s.\n",
			    ypclnterr2string(ypclnt_status));
		}

		output_rpc_error(ypclnt_status);
		exit(ypclnterr2exit(ypclnt_status) );
	}
	
	if (bin_ordernum_before <= current_transfer->mx_map->map_order) {
		
		if (log_file) {
			fprintf(log_file,
			    "not attempted - peer's copy not newer.\n");
		}
		
		exit(YPXFR_EXIT_OLD);
	}

	/* Make a new pair of empty dbm files for the temp map. */
	
	strcpy(dbm_filename, current_transfer->mx_temp_path);
	strcat(dbm_filename, ".pag");
	dbm_file = open(dbm_filename, (O_RDWR | O_CREAT | O_TRUNC), 0666);
	
	if (dbm_file == -1 ) {
		
		if (log_file) {
			fprintf(log_file, "failed:  Can't create dbm file %s.\n",
			dbm_filename);
		}
		
		exit (YPXFR_EXIT_FILE);
	} else {
		close(dbm_file);
	}

	strcpy(dbm_filename, current_transfer->mx_temp_path);
	strcat(dbm_filename, ".dir");
	dbm_file = open(dbm_filename, (O_RDWR | O_CREAT | O_TRUNC), 0666);
	
	if (dbm_file == -1 ) {
		
		if (log_file) {
			fprintf(log_file, "failed:  Can't create dbm file %s.\n",
			    dbm_filename);
		}
		
		exit (YPXFR_EXIT_FILE);
	} else {
		close(dbm_file);
	}

	/*
	 * Initialize the dbm data base.  Notice that we aren't going through
	 * ypset_current_map here.  We have to check to see if there is a
	 * current map (which was set up by our parent), then reinitialize the
	 * dbm private data base to point to our temporary map.
	 */
	 

	if (current_map[0] != '\0') {
		dbmclose(current_map);
	};

	if (dbminit(current_transfer->mx_temp_path) < 0) {
		
		if (log_file) {
			fprintf(log_file, "failed:  Can't dbminit temp map %s\n",
			    current_transfer->mx_temp_path);
		}
		
		exit(YPXFR_EXIT_DBM);
	}
		
	/*
	 * Get the map from the peer, while there are no errors, and there are
	 * more key-value pairs.  Any error condition will make us close up
	 * shop.
	 */

	ypclnt_status =
	    _ypclnt_dofirst (current_transfer->mx_map->map_domain->dom_name,
	    current_transfer->mx_map->map_name, &xfr_binding, yptimeout,
	    &(key.dptr), &(key.dsize), &(val.dptr), &(val.dsize));

	if (ypclnt_status) {
		
		if (log_file) {
			fprintf(log_file,
	"failed:  Can't get first key-value pair from peer; /n/treason %s.\n",
			    ypclnterr2string(ypclnt_status));
		}

		output_rpc_error(ypclnt_status);
		exit(ypclnterr2exit(ypclnt_status) );
	}

	while (TRUE) {
		
		if (store(key, val) < 0) {
		
			if (log_file) {
				fprintf(log_file,
			 	   "failed:  dbm store operation failed.\n");
			}
		
			exit(YPXFR_EXIT_DBM);
		}

		free(val.dptr);
		inkey = key.dptr;
		inkeylen = key.dsize;
		ypclnt_status =
		    _ypclnt_donext (
		        current_transfer->mx_map->map_domain->dom_name,
		    current_transfer->mx_map->map_name, inkey, inkeylen,
		    &xfr_binding, yptimeout, &(key.dptr), &(key.dsize),
		    &(val.dptr), &(val.dsize));

		if (ypclnt_status) {

			/*
			 * If the error code is "no more", we have completed
			 * successfully; otherwise, this is a hard error.
			 */

			if (ypclnt_status == YPERR_NOMORE) {
		
				if (store(key, val) < 0) {
		
					if (log_file) {
						fprintf(log_file,
				    "failed:  dbm store operation failed.\n");
					}
		
					exit(YPXFR_EXIT_DBM);
				}

				free(inkey);
				free(val.dptr);
				free(key.dptr);
				break;
			} else {
		
				if (log_file) {
					fprintf(log_file,
		"failed:  Can't get key-value pair from peer; /n/treason %s.\n",
					    ypclnterr2string(ypclnt_status));
				}

				output_rpc_error(ypclnt_status);
				exit(ypclnterr2exit(ypclnt_status) );
			}
		}

		free(inkey);
	}
	
	/*
	 * Get the map's order number again. 
	 */
	 

	if (ypclnt_status= yppoll_for_order_number(current_transfer->mx_map,
	    &bin_ordernum_after) ) {
		
		if (log_file) {
			fprintf(log_file,
			   "failed:  Second poll for order failed; /n/treason %s.\n",
			   ypclnterr2string(ypclnt_status));
		}

		output_rpc_error(ypclnt_status);
		exit(ypclnterr2exit(ypclnt_status) );
	}
	
	if (bin_ordernum_after != bin_ordernum_before) {
		
		if (log_file) {
			fprintf(log_file,
			  "failed:  Version changed at peer during transfer.\n");
		}

		exit(YPXFR_EXIT_SKEW);
	}

	/*
	 * Sanity-check the map by retrieving the order number directly from
	 * the newly transferred map.  If it's not there, exit indicating a
	 * badly formed map.  If it is there, but isn't the same as the value
	 * the peer has told us of, exit indicating a skew error.  If everything
	 * is OK, exit with a success code.
	 */

	key.dptr = order_key;
	key.dsize = ORDER_KEY_LENGTH;
	val = fetch(key);

	if (val.dptr == NULL) {
		
		if (log_file) {
			fprintf(log_file,
			    "failed:  Can't retrieve order number from map.\n");
		}
		
 		exit(YPXFR_EXIT_FORM);
	}

	/*
	 * Recopy the value from dbm into some local memory, so we can
	 * correctly terminate it.  (It's not null-terminated, and we don't
	 * know what garbage characters follow the value characters in
	 * dbm's private memory.  If the garbage characters are numeric,
	 * atol will return a garbage value.)  We'll reuse the char array
	 * dbm_filename, which we no longer need.
	 */

	bcopy(val.dptr, dbm_filename, val.dsize);
	dbm_filename[val.dsize] = '\0';
	bin_ordernum_map = (unsigned long) atol(dbm_filename);

	if (bin_ordernum_map == bin_ordernum_before) {
		
		if (log_file) {
			fprintf(log_file, "succeeded.\n");
		}
		
		exit(YPXFR_EXIT_SUCC);
	} else {
		
		if (log_file) {
			fprintf(log_file,
	"failed:  Order number in map differs from that claimed by peer.\n");
		}
		
		exit(YPXFR_EXIT_SKEW);
	}
}

/*
 * This maps an error code coming back from the ypcommon client layer into a
 * map transfer process exit code.
 */
static int
ypclnterr2exit(client_error)
	int client_error;
{
	int exit_code;

	switch (client_error) {

	case YPERR_BADARGS: {
		exit_code = YPXFR_EXIT_ERR;
		break;
	}
	
	case YPERR_RPC: {
		exit_code = YPXFR_EXIT_RPC;
		break;
	}
	
	case YPERR_DOMAIN: {
		exit_code = YPXFR_EXIT_DOMAIN;
		break;
	}
	
	case YPERR_MAP: {
		exit_code = YPXFR_EXIT_MAP;
		break;
	}
	
	case YPERR_KEY: {
		exit_code = YPXFR_EXIT_FORM;
		break;
	}
	
	case YPERR_YPERR: {
		exit_code = YPXFR_EXIT_ERR;
		break;
	}
	
	case YPERR_RESRC: {
		exit_code = YPXFR_EXIT_RSRC;
		break;
	}
	
	case YPERR_NOMORE: {
		exit_code = YPXFR_EXIT_SUCC;
		break;
	}

	default: {
		exit_code = YPXFR_EXIT_ERR;
		break;
	}
	
	}

	return(exit_code);
}
/*
 * This maps a ypclnt error code into a printable string for inclusion in
 * a logging error message.
 */
static char *
ypclnterr2string(err)
	unsigned int err;
	
{
	char *p;

	switch (err) {

	case YPERR_BADARGS:  {
		p = "args to function are bad";
		break;
	}

	case YPERR_RPC:  {
		p = "RPC failure";
		break;
	}

	case YPERR_DOMAIN:  {
		p = "can't bind to a server which serves this domain.";
		break;
	}

	case YPERR_MAP:  {
		p = "no such map in server's domain";
		break;
	}

	case YPERR_KEY:  {
		p = "no such key in map";
		break;
	}

	case YPERR_YPERR:  {
		p = "internal yp server or client interface error";
		break;
	}

	case YPERR_RESRC:  {
		p = "local resource allocation failure";
		break;
	}

	case YPERR_NOMORE:  {
		p = "no more records in map database";
		break;
	}

	default: {
		p = "unknown error code";
		break;
	}
	}

	return(p);
	
}

/*
 * This checks to see if we are logging and we still have stderr open, then
 * calls clnt_perror to dump the RPC error message into the log file.
 * Because clnt_perror uses stderr, this error message is only available if the
 * global boolean "silent" is FALSE.
 */
void
output_rpc_error(err)
	unsigned int err;

{
	if (log_file && !silent && (err == YPERR_RPC) ) {
		clnt_perror(xfr_binding.dom_client, "ypserv:  output_rpc_error");
	}
}
