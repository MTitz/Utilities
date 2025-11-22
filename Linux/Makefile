CC 	= cc
CFLAGS  = -O2
LFLAGS	=
LIBS    =
STRIP	= strip
GROFF	= groff

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/man/man1


.c.o:
	$(CC) $(CFLAGS) -c $<


all:	prog docs

install: prog docs
	cp -p winsize $(BINDIR)
	chown root:root $(BINDIR)/winsize
	chmod 755 $(BINDIR)/winsize

prog:	winsize

docs:

winsize: winsize.o
	$(CC) -o $@ winsize.o $(LFLAGS) $(LIBS)
	$(STRIP) $@

clean:
	rm -f *.o winsize
