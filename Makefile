#  Makefile for gbII. Client for Galactic Bloodshed.
#
#  Written By Evan D. Koffler <evank@netcom.com>
#
#  Copyright (c) 1990, 1991, 1992, 1993
#
#  See Copyright file.

FILENAME = gbII
MV = mv -f
RM = rm -f

# main level files
MAIN = README setup Makefile COPYRIGHT

# source code files
CFILES = action.c args.c bind.c buffer.c command.c crypt.c csp.c gb.c fuse.c\
	 help.c icomm.c imap.c key.c load.c map.c option.c popn.c proc.c psmap.c\
	 socket.c status.c stmt.c str.c term.c util.c xmap.c md5.c\
	 args.h bind.h command.h csp.h csp_types.h csparr.h option.h str.h\
	 term.h types.h vars.h xmap.h 

SRCFILES = gb.h.default Makefile.default

# documentation files
DOCS =	 BUGREPORT README_XMAP Help Help_server History\
	 TODO TODO.XMAP CHANGES CHANGES.XMAP CLIENT_PROTOCOL\
	 sample.gbrc sample.macros

# object files
COBJS = action.o args.o bind.o buffer.o command.o crypt.o csp.o gb.o fuse.o\
	help.o icomm.o imap.o key.o load.o map.o option.o popn.o psmap.o\
	proc.o socket.o status.o stmt.o str.o term.o util.o xmap.o md5.o

# primary target of make
all: source/Makefile Makefile
	( cd source; make )
	mv ./source/$(FILENAME) .
	@echo ""
	@echo ""
	@echo $(FILENAME) located in `pwd`
	@echo ""
	@echo ""

# the shell 
install: source/$(FILENAME)
	cd source; make install

source/$(FILENAME):
	cd source; make $(FILENAME)

# Extras to make my life easier
tar:
	@echo Moving source/gb.h and source/Makefile and gb out of the way
	@echo ""
	chmod a+rx ./setup
	-@${MV} ./source/Makefile ..
	-@${MV} ./source/Makefile~ ..
	-@${MV} ./source/gb.h ..
	-@${MV} ./source/gb.h~ ..
	-@${MV} ./gb ..
	( cd .. ; tar -cfFF - gbII | compress > gbII.client.tar.Z )
	ls -l ../gbII.client.tar.Z
	@echo Moving source/gb.h and source/Makefile and gb back in.
	-@${MV} ../Makefile ./source/.
	-@${MV} ../Makefile~ ./source/.
	-@${MV} ../gb.h ./source/.
	-@${MV} ../gb.h~ ./source/.
	-@${MV} ../gb .

shar:
	@echo Moving source/gb.h and source/Makefile out of the way
	@echo ""
	-@${MV} ./source/Makefile ..
	-@${MV} ./source/Makefile~ ..
	-@${MV} ./source/gb.h ..
	-@${MV} ./source/gb.h~ ..
	-@${MV} ./gb ..
	( cd .. ; shar gbII | compress > gbII.client.shar.Z )
	ls -l ../gbII.client.shar.Z
	@echo Moving source/gb.h and source/Makefile back in.
	@echo ""
	-@${MV} ../Makefile ./source/.
	-@${MV} ../Makefile~ ./source/.
	-@${MV} ../gb.h ./source/.
	-@${MV} ../gb.h~ ./source/.
	-@${MV} ../gb .

post:
	@echo Moving source/gb.h and source/Makefile out of the way
	@echo ""
	-@${MV} ./source/Makefile ..
	-@${MV} ./source/Makefile~ ..
	-@${MV} ./source/gb.h ..
	-@${MV} ./source/gb.h~ ..
	-@${MV} ./gb ..
	( cd .. ; tar -cfFF gbII.client.tar gbII )
	ls -l ../gbII.client.tar
	( cd .. ; shar gbII > gbII.client.shar )
	ls -l ../gbII.client.shar
	@echo ""
	@echo Moving source/gb.h and source/Makefile back in.
	-@${MV} ../Makefile ./source/.
	-@${MV} ../Makefile~ ./source/.
	-@${MV} ../gb.h ./source/.
	-@${MV} ../gb.h~ ./source/.
	-@${MV} ../gb .

clean:
	( cd source ; ${RM} ${COBJS} gbII gb.h~ Makefile~)
	${RM} gbII

clear: clean
	( cd source ; ${RM} Makefile gb.h )

ultra:
	( cd source ; ${RM} ${COBJS} ${CFILES} ${SRCFILES} Makefile \
	Makefile~ gb.h gb.h~ gbz gbII)
	( cd docs ; ${RM} ${DOCS} )
	${RM} ${MAIN}

count:
	wc -l ${MAIN}
	( cd source ; wc -l ${CFILES} ${SRCFILES} )
	( cd docs ; wc -l ${DOCS} )
