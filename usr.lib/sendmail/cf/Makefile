######################################################
##                                                  ##
##  Wisconsin Sendmail Configuration file Makefile  ##
##                                                  ##
######################################################

RSCHUVAX= american amul asiago balder bleu boursin brick brie \
	cabrales camembert \
	caseus cheddar cheshire chevre cojack colby cottage cream curds edam \
	enmenthaler farmer feta fontina gjetost goat gorgonzola gouda gruyere \
	havarti head jack jalapeno kaese kasseri kraft-slices lowfat \
	manchego monterey mozzarella muenster nacho obatzter oka \
	parmesan provolone quark ricotta romano rondele roquefort \
	saga stilton string swiss tomb velveeta whiz

INSTUVAX= inst2
LABUVAX= frigg odin limburger
BIGINST= gumby pokey
BIGRSCH= spool crys db rsch ai pipe dream wisc-gateway topaz
DUMBVAX= stat
FUNNYINST= gosset puff garfield
IBMS= janeb angelina

ALL= $(BIGRSCH) $(RSCHUVAX) $(LABUVAX) $(BIGINST) $(INSTUVAX) $(DUMBVAX) $(FUNNYINST) $(IBMS)

all: $(ALL)

############################
#   special dependencies   #
############################

# pessimize -- basic dependencies
$(ALL): uwisc.std /etc/localhosts
	sed -n -e "/^1.*[ 	]$@/{" -e 's/^[^ 	]*[ 	][ 	]*//p' -e q -e '}' /etc/localhosts | \
		awk '{ printf "define(HOSTNAME,%s)", $$1 ; printf "define(ARPANAMES,%s)dnl\n", $$0}' \
		| m4 - uwisc.std > localcf/$@

#####################
#   administrivia   #
#####################

install: localcf/$(ALL)
	-for x in $(ALL) ; \
	 do /usr/ucb/rcp localcf/$$x $$x:/usr/lib/sendmail.cf ; \
	 done 
	-rshall -a cd /usr/lib \; ./sendmail -bz

clean:
	rm -f $(ALL) a.out core make.out
	rm -f ,*
