CC = gcc
CFLAGS=-Wall `pkg-config --cflags gtk+-3.0` -DG_DISABLE_DEPRECATED=1 -DGDK_DISABLE_DEPRECATED=1 -DGDK_PIXBUF_DISABLE_DEPRECATED=1 -DGTK_DISABLE_DEPRECATED=1
LIBS=-lm `pkg-config --libs gtk+-3.0`
bindir ?= /usr/bin
mandir ?= /usr/share/man

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

all: main.o colors.o sort.o drawatoms.o readinput.o init.o rotate.o setup.o Makefile
	$(CC) $(CFLAGS) -o gdpc2 main.o colors.o drawatoms.o init.o sort.o rotate.o setup.o readinput.o $(LIBS)

main.o: main.c parameters.h

colors.o: colors.c parameters.h

readinput.o: readinput.c parameters.h

drawatoms.o: drawatoms.c parameters.h

init.o: init.c parameters.h

sort.o: sort.c parameters.h

rotate.o: rotate.c parameters.h

setup.o: setup.c parameters.h tooltips.h

clean:
	rm *.o gdpc2

install:
	install -p -m 755 -D gdpc2 $(bindir)/gdpc2

uninstall:
	rm $(bindir)/gdpc2
