#!/bin/csh -f
#
# 	program to invoke its args iff its input 
#	file (stdin) is non-null.
#

if ( $#argv == 0 ) then
	echo "usage: `basename $0` command [ args ]"
	exit 1
endif

umask 2
set tmp = /tmp/zmail.$$

cat > $tmp

if ( ! -z $tmp ) then
	cat $tmp | eval $argv 
endif

/bin/rm -f $tmp
