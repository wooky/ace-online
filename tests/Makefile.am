noinst_PROGRAMS = test1 test3 test4 test5 test6
bin_PROGRAMS = penguins
test6_SOURCES = test6.c test6c.c

INCLUDES = -I$(srcdir)/../lib $(X_CFLAGS) @PDA@
LDFLAGS = $(X_LIBS)
LDADD = ../lib/libcards.la -lpng -lz -lX11 -lm

EXTRA_DIST = test6.png

CLEANFILES = test6c.c

test6c.c : test6.png ../lib/make-imglib
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ test6.png
