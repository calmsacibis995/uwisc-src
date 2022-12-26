Source = any.c atob.c copy.c getword.c isnumber.c isblank.c \
	ltrunc.c pmatch.c strdup.c strecat.c stricmp.c to_lower.c \
	instr.c iinstr.c fsleep.c ptypair.c setenv.c bset.s

Objects = any.o atob.o copy.o getword.o isnumber.o isblank.o \
	ltrunc.o pmatch.o strdup.o strecat.o stricmp.o to_lower.o \
	instr.o iinstr.o fsleep.o ptypair.o setenv.o bset.o

CFLAGS = -O ${DEFS}

.s.o:
	/lib/cpp -E ${DEFS} $*.s | ${AS} -o $*.o
	-ld -x -r $*.o
	mv a.out $*.o

libloc.a :	$(Objects)	Makefile
	ar cvr libloc.a $(Objects)

install: libloc.a
	cp libloc.a /usr/lib
	ranlib /usr/lib/libloc.a
clean :
	rm -f *.o core libloc.a
