: '@(#)install.sh	1.17	12/16/86'

if test "$#" != 6
then
	echo "usage: $0 spooldir libdir bindir nuser ngroup ostype"
	exit 1
fi
SPOOLDIR=$1
LIBDIR=$2
BINDIR=$3
NEWSUSR=$4
NEWSGRP=$5
OSTYPE=$6

: Get name of local system
case $OSTYPE in
	usg)	SYSNAME=`uname -n`
		if test ! -d $LIBDIR/history.d
		then
			mkdir $LIBDIR/history.d
			chown $NEWSUSR $LIBDIR/history.d
			chgrp $NEWSGRP $LIBDIR/history.d
		fi;;
	v7)	SYSNAME=`uuname -l`
		touch $LIBDIR/history.pag $LIBDIR/history.dir;;
	*)	echo "$0: Unknown Ostype"
		exit 1;;
esac

if test "$SYSNAME" = ""
then
	echo "$0: Cannot get system name"
	exit 1
fi

: Ensure SPOOLDIR exists
for i in $SPOOLDIR $SPOOLDIR/.rnews
do
	if test ! -d $i
	then
		mkdir $i
	fi
	chmod 777 $i
	chown $NEWSUSR $i
	chgrp $NEWSGRP $i
done

chown $NEWSUSR $LIBDIR
chgrp $NEWSGRP $LIBDIR

: Ensure certain files in LIBDIR exist
touch $LIBDIR/history $LIBDIR/active $LIBDIR/log $LIBDIR/errlog $LIBDIR/users
chmod 666 $LIBDIR/users

: If no sys file, make one.
if test ! -f $LIBDIR/sys
then
echo
echo Making a $LIBDIR/sys file to link you to oopsvax.
echo You must change oopsvax to your news feed.
echo If you are not in the USA, remove '"usa"' from your line in the sys file.
echo If you are not in North America, remove '"na"' from your line in the sys file.
	cat > $LIBDIR/sys << EOF
$SYSNAME:world,comp,sci,news,rec,soc,talk,misc,net,mod,na,usa,to::
oopsvax:world,comp,sci,news,rec,soc,talk,misc,net,mod,na,usa,to.oopsvax::
EOF
fi

: If no seq file, make one.
if test ! -s $LIBDIR/seq
then
	echo '100' >$LIBDIR/seq
fi

: If no mailpaths, make one.
if test ! -s $LIBDIR/mailpaths
then
	cat <<E_O_F >$LIBDIR/mailpaths
backbone	%s
internet	%s
E_O_F
echo "I have created $LIBDIR/mailpaths for you. The paths are certainly wrong."
echo "You must correct them manually to be able to post to moderated groups."
fi

sh makeactive.sh $LIBDIR $SPOOLDIR $NEWSUSR $NEWSGRP

for i in $LIBDIR/ngfile $BINDIR/inews $LIBDIR/localgroups $LIBDIR/moderators \
	$LIBDIR/cunbatch $LIBDIR/c7unbatch
do
	if test -f $i
	then
		echo "$i is no longer used. You should remove it."
	fi
done

for i in $LIBDIR/csendbatch $LIBDIR/c7sendbatch
do
	if test -f $i
	then
		echo "$i is no longer used. You should remove it after"
		echo "changing your crontab entry to use sendbatch [flags]"
	fi
done

if test -f $BINDIR/cunbatch
then
	echo "$BINDIR/cunbatch is not used by the new batching scheme."
	echo "You should remove it when all of your neighbors have upgraded."
fi

cat >$LIBDIR/aliases.new <<EOF
net.audio	rec.audio
net.auto	rec.autos
net.auto.tech	rec.autos.tech
net.aviation	rec.aviation
net.bicycle	rec.bicycles
net.rec.birds	rec.birds
net.rec.boat	rec.boats
net.cooks	rec.food.cooking
net.wines	rec.food.drink
net.veg		rec.food.veg
net.games	rec.games.misc
net.games.board	rec.games.board
net.rec.bridge	rec.games.bridge
net.games.chess	rec.games.chess
net.games.emp	rec.games.empire
net.games.frp	rec.games.frp
net.games.go	rec.games.go
net.games.hack	rec.games.hack
net.games.pbm	rec.games.pbm
net.games.rogue	rec.games.rogue
net.games.trivia	rec.games.trivia
net.games.video	rec.games.video
net.garden	rec.gardens
net.ham-radio	 rec.ham-radio
net.ham-radio.packet rec.ham-radio.packet
net.jokes	rec.humor
net.jokes.d	rec.humor.d
mod.mag		rec.mag
net.mag		rec.mag
net.books	rec.arts.books
net.comics	rec.arts.comics
net.tv.drwho	rec.arts.drwho
mod.movies	rec.arts.movies
net.movies	rec.arts.movies
net.sf-lovers	rec.arts.sf-lovers
net.startrek	rec.arts.startrek
net.tv		rec.arts.tv
net.tv.soaps	rec.arts.tv.soaps
net.wobegon	rec.arts.wobegon
net.rec		rec.misc
net.cycle	rec.motorcycles
net.music.classical	rec.music.classical
net.music.folk	rec.music.folk
net.music.gdead	rec.music.gdead
net.music.makers	rec.music.makers
net.music	rec.music.misc
net.music.synth	rec.music.synth
net.rec.nude	rec.nude
net.pets	rec.pets
net.rec.photo	rec.photo
net.poems	rec.arts.poems
net.puzzle	rec.puzzles
net.railroad	rec.railroad
net.rec.scuba	rec.scuba
net.rec.ski	rec.skiing
net.rec.skydive	rec.skydiving
net.sport	rec.sport.misc
net.sport.baseball	rec.sport.baseball
net.sport.hoops	rec.sport.basketball
net.sport.football	rec.sport.football
net.sport.hockey	rec.sport.hockey
net.travel	rec.travel
net.video	rec.video
net.rec.wood	rec.woodworking
net.ai	comp.ai
net.arch	comp.arch
net.bugs.2bsd	comp.bugs.2bsd
net.bugs.4bsd	comp.bugs.4bsd
net.bugs.usg	comp.bugs.sys5
net.bugs.uucp	comp.bugs.misc
net.bugs.v7	comp.bugs.misc
net.bugs	comp.bugs.misc
net.cog-eng	comp.cog-eng
net.cse		comp.edu
net.database	comp.databases
net.dcom	comp.dcom.modems
net.decus	comp.org.decus
net.emacs	comp.emacs
net.eunice	comp.os.eunice
net.graphics	comp.graphics
net.info-terms	comp.terminals
net.internat	comp.std.internat
net.lan		comp.dcom.lans
net.lang	comp.lang.misc
net.lang.ada	comp.lang.ada
net.lang.apl	comp.lang.apl
net.lang.c	comp.lang.c
net.lang.c++	comp.lang.c++
net.lang.f77	comp.lang.fortran
net.lang.forth	comp.lang.forth
net.lang.lisp	comp.lang.lisp
net.lang.mod2	comp.lang.modula2
net.lang.pascal	comp.lang.pascal
net.lang.prolog	comp.lang.prolog
net.lang.st80	comp.lang.smalltalk
net.lsi		comp.lsi
net.mail	comp.mail.uucp
net.mail.headers	comp.mail.headers
net.micro	comp.sys.misc
net.micro.6809	comp.sys.m6809
net.micro.68k	comp.sys.m68k
net.micro.apple	comp.sys.apple
net.micro.amiga	comp.sys.amiga
net.micro.atari16	comp.sys.atari.st
net.micro.atari8	comp.sys.atari.8bit
net.micro.att	comp.sys.att
net.micro.cbm	comp.sys.cbm
net.micro.cpm	comp.os.cpm
net.micro.hp	comp.sys.hp
net.micro.mac	comp.sys.mac
net.micro.ns32k	comp.sys.nsc.32k
net.micro.pc	comp.sys.ibm.pc
net.micro.ti	comp.sys.ti
net.micro.trs-80	comp.sys.tandy
net.news	news.misc
net.news.adm	news.admin
net.news.b	news.software.b
net.news.config	news.config
net.news.group	news.groups
net.news.newsite	news.newsites
net.news.notes	news.software.notes
net.news.sa	news.sysadmin
net.news.stargate	news.stargate
net.periphs	comp.periphs
net.sources.d	comp.sources.d
net.text	comp.text
net.unix	comp.unix.questions
net.unix-wizards	comp.unix.wizards
net.usenix	comp.org.usenix
net.wanted.sources	comp.sources.wanted
net.chess		rec.games.chess
net.trivia		rec.games.trivia
net.rec.radio		rec.ham-radio
net.term		comp.terminals
net.joke		rec.humor
net.vlsi		comp.lsi
net.micro.16k		comp.sys.nsc.32k
net.music.gdea		rec.music.gdead
net.notes		news.software.notes
net.periph		comp.periphs
net.puzzles		rec.puzzles
net.unix.wizards	comp.unix.wizards
net.sources.wanted	comp.sources.wanted
net.consumers		misc.consumers
net.consumers.house	misc.consumers.house
net.house		misc.consumers.house
na.forsale		misc.forsale
net.forsale		misc.forsale
net.politics.terror	misc.headlines
net.invest		misc.invest
net.jobs		misc.jobs
net.kids		misc.kids
mod.legal		misc.legal
net.legal		misc.legal
net.followup		misc.misc
net.general		misc.misc
net.misc		misc.misc
net.suicide		misc.misc
net.taxes		misc.taxes
mod.test		misc.test
net.test		misc.test
net.wanted		misc.wanted
net.announce		mod.announce
net.announce.newusers	mod.announce.newusers
mod.map.uucp		mod.map
net.religion.christian	mod.religion.christian
net.religion.xian	mod.religion.christian
net.astro		sci.astro
net.astro.expert	sci.astro
net.bio			sci.bio
net.crypt		sci.crypt
net.analog		sci.electronics
net.nlang		sci.lang
net.math		sci.math
net.stat		sci.math.stat
net.math.stat		sci.math.stat
net.math.symbolic	sci.math.symbolic
net.med			sci.med
net.sci			sci.misc
net.physics		sci.physics
net.research		sci.research
net.space		sci.space
net.columbia		sci.space.shuttle
net.challenger		sci.space.shuttle
net.college		soc.college
net.nlang.africa	soc.culture.african
net.nlang.celts		soc.culture.celtic
net.nlang.greek		soc.culture.greek
net.nlang.india		soc.culture.indian
net.religion.jewish	soc.culture.jewish
net.social		soc.misc
mod.motss		soc.motss
net.motss		soc.motss
net.net-people		soc.net-people
net.roots		soc.roots
net.singles		soc.singles
net.women		soc.women
net.abortion		talk.abortion
net.bizarre		talk.bizarre
net.origins		talk.origins
net.philosophy		talk.philosophy.misc
net.politics		talk.politics.misc
net.politics.theory	talk.politics.theory
net.religion		talk.religion.misc
talk.religion		talk.religion.misc
net.rumor		talk.rumors
talk.rumor		talk.rumors
rec.skydive		rec.skydiving
comp.sources.games	net.sources.games
comp.sources.bugs	net.sources.bugs
comp.sources.unix	net.sources
comp.sources.mac	net.sources.mac
EOF
: if no aliases file, make one
if test ! -f $LIBDIR/aliases
then
	mv $LIBDIR/aliases.new $LIBDIR/aliases
else
	: see whats missing
	sort $LIBDIR/aliases | sed -e 's/  */	/g'  -e 's/		*/	/g' >/tmp/$$aliases
	sort $LIBDIR/aliases.new | sed -e 's/  */	/g'  -e 's/		*/	/g' >/tmp/$$aliases.new
	comm -23 /tmp/$$aliases.new /tmp/$$aliases >/tmp/$$comm
	if test -s /tmp/$$comm
	then
		echo "The following suggested aliases are missing or incorrect in your"
		echo "$LIBDIR/aliases file. It is suggested you add them."
		echo ""
		cat /tmp/$$comm
		echo ""
		echo "A suggested aliases file has been left in $LIBDIR/aliases.new"
		echo "for your convenience."
		rm /tmp/$$comm /tmp/$$aliases
	else
		rm /tmp/$$comm /tmp/$$aliases $LIBDIR/aliases.new
	fi
fi

: if no distributions file, make one
if test ! -f $LIBDIR/distributions
then
	cat >$LIBDIR/distributions <<EOF
local		Local to this site
regional	Everywhere in this general area
usa		Everywhere in the USA
na		Everywhere in North America
world		Everywhere on Usenet in the world
EOF
echo
echo You may want to add distributions to $LIBDIR/distributions if your
echo site particpates in a regional distribution such as '"ba"' or '"dc"'.
fi

chown $NEWSUSR $LIBDIR/[a-z]*
chgrp $NEWSGRP $LIBDIR/[a-z]*

echo
echo Reminder: uux must permit rnews if running over uucp.
rm -f /tmp/$$*
