
#
# Makefile for NNTP intstallation
#

install:
	@ echo "If you want to install a server version or"
	@ echo "a client version, type \"make install_server\""
	@ echo "or \"make install_client\" respectively."
	@ echo "See README for more details."

install_server:
	cd server; make install
	cd support; make install

install_client:
	cd rrn; make install
	cd inews; make install

distrib:
	rm -rf client etc rrn.src
	cd server; make distrib
	cd support; make distrib
	cd inews; make distrib
	cd common; rm -rf SCCS clientlib.o
