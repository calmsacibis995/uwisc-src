rm -f Makefile
cp Makefile.dst Makefile
chmod u+w Makefile
ed - Makefile  <<'EOF'
g/^#V7 /s///
g/^#BSD4_3 /s///
g/^#USG /d
g/^#VMS /d
g/^#BSD4_1 /d
1,$s/#NOTVMS//
/^UUXFLAGS/s/-r -z/-r -z -n -gd/
/^SCCSID/s/ -DSCCSID//
/^CFLAGS/s/$/ -DUW/
w
q
EOF
rm -f defs.h
cp defs.dist defs.h
chmod u+w defs.h
ed - defs.h <<'EOF'
/ROOTID/s/10/6/
/N_UMASK/s/000/022/
/DFTXMIT/s/-z/-z -gd/
/UXMIT/s/-z/-z -gd/
/DFLTSUB/s/general,all.announce/all/
/MYDOMAIN/s/UUCP/WISC.EDU/
/INTERNET/s;/\* ;;
/GHNAME/s;/\* ;;
/UUNAME/s;/\* ;;
/DOXREFS/s;/\* ;;
/BSD4_2/s;/\* ;;
/SENDMAIL/s;/\* ;;
/MYORG/s;Frobozz Inc., St. Louis;/usr/lib/news/organization;
/LINES/s;512;768;
w
q
EOF
: echo "Make sure that /usr/new is in PATH in /usr/lib/uucp/L.cmds; read ../misc/L.cmds"
