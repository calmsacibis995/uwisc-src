#!/bin/csh -f
#
#	program to print the new lines added to a file
#	since the last time this script was run.  saves
#	the old linecount in file.delta if possible. you 
#	only want to use this on files that get new lines
#	appended to them.
#
#	written by tom christiansen 
#			   Wed Nov 12 12:43:39 CST 1986

if ( $#argv == 0 ) then
	echo  "usage: `basename $0` file ..."
	exit 1
endif

foreach file ( $argv )
	if ( ( -e $file.delta && ! -w $file.delta ) || ( ! -w $file:h ) ) then
		echo cannot make delta file for $file
		goto nextfor
	endif

	if ( ! -e $file ) then
		echo No file: $file
		goto nextfor
	endif

	set curlines = `wc -l $file`
	set curlines = $curlines[1]

	if ( -e $file.delta ) then
		set oldlines = `cat $file.delta`
		@ newlines = $curlines - $oldlines
		echo $curlines > $file.delta 
		tail -$newlines $file
	else
		echo $curlines > $file.delta 
	endif
nextfor:
end
