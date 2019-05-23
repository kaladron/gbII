#  Makefile for gbII. Client for Galactic Bloodshed.
#
#  Written By Evan D. Koffler <evank@netcom.com>
#
#  Copyright (c) 1990, 1991, 1992, 1993
#
#  See Copyright file.

FILENAME = gbII
INSTALL = install -s -m 755

# Path where the compiled client should be located.
# if you use this option, you can type 'make install'
INSTALLNAME = /usr/games
HELPDIR = /usr/games/lib
HELPC = Help
HELPS = Help_server

# XMAP options:
XLIB = -L/usr/local/X11/lib -lX11
XINC = -I/usr/local/X11/include

# Compiler to be used. cc or gcc are most common.
CC = gcc
#CC = cc

# Compiler options. -g is for debugging, and -O is optimization
CFLAGS =  -g ${XINC} -Wno-implicit-function-declaration -Wno-implicit-int
#CFLAGS = -O

# Libraries needed for compiling
#
# SYSV uses: -ltermcap -lm -lsocket
# BSD uses:  -ltermcap -lm
# NOTE: This client makes use of termcap, socket, and math libraries
LIBS = -ltermcap -lm

# objects for client
CFILES = action.c args.c bind.c buffer.c command.c crypt.c csp.c gb.c fuse.c\
	 help.c icomm.c imap.c key.c load.c map.c option.c popn.c psmap.c\
	 proc.c socket.c status.c stmt.c str.c term.c util.c xmap.c md5.c

COBJS = action.o args.o bind.o buffer.o command.o crypt.o csp.o gb.o fuse.o\
	help.o icomm.o imap.o key.o load.o map.o option.o popn.o psmap.o\
	proc.o socket.o status.o stmt.o str.o term.o util.o xmap.o md5.o

all: $(FILENAME)

# the shell 
install: $(FILENAME)
	${INSTALL} $(FILENAME) $(INSTALLNAME)
	-cp ../docs/$(HELPC) $(HELPDIR)/$(HELPC)
	-cp ../docs/$(HELPS) $(HELPDIR)/$(HELPS)

$(FILENAME): ${COBJS}
	$(CC) -o $(FILENAME) ${COBJS} ${LIBS} ${XLIB};

gb: ${COBJS}
	$(CC) -o $(FILENAME) ${COBJS} ${LIBS} ${XLIB};

# The dependancies for object files.
vars.h:     types.h
action.o:   gb.h args.h option.h str.h vars.h
args.o:     gb.h str.h vars.h
bind.o:     gb.h bind.h str.h vars.h
buffer.o:   gb.h option.h str.h vars.h
comm.o:     gb.h str.h vars.h
command.o:  gb.h args.h command.h csp.h str.h term.h vars.h
crypt.o:    gb.h option.h str.h vars.h
csp.o:      gb.h csp.h csp_types.h csparr.h str.h term.h vars.h
gb.o:       gb.h option.h str.h term.h vars.h
help.o:     gb.h str.h vars.h
imap.o:     gb.h csp.h str.h vars.h
key.o:      gb.h args.h option.h str.h term.h vars.h
load.o:     gb.h str.h vars.h
map.o:      gb.h str.h option.h term.h vars.h
option.o:   gb.h args.h option.h str.h term.h vars.h
popn.o:     gb.h csp.h str.h vars.h
proc.o:     gb.h str.h vars.h
socket.o:   gb.h csp.h option.h str.h term.h vars.h
status.o:   gb.h option.h str.h term.h vars.h
stmt.o:     gb.h args.h str.h vars.h
str.o:      gb.h option.h term.h vars.h
term.o:     gb.h option.h str.h term.h vars.h
util.o:     gb.h args.h str.h vars.h

# special case for Xmap.. need to include some stuff.
xmap.o:     gb.h xmap.h vars.h
	$(CC) ${CFLAGS} ${XINC} -c xmap.c

clean:
	-rm -f ${COBJS} ${FILENAME}

.PHONE: clean
