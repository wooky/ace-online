lib_LTLIBRARIES = libcards.la

noinst_PROGRAMS = text2c$(EXEEXT) make-imglib$(EXEEXT)
noinst_HEADERS = cards.h

CLEANFILES = images.c images.d

AM_CPPFLAGS = $(X_CFLAGS) @PDA@
AM_LDFLAGS = $(X_LIBS)

BUILD_CC = @BUILD_CC@
AR = @AR@
STRIP = @STRIP@

libcards_la_LDFLAGS = -version-info 1:0:0 $(AM_LDFLAGS)

libcards_la_SOURCES = table.c help.c stack.c imagelib.c xwin.c \
	images.c table_rn.c funcs.c

# table_ai.c table_ck.c table_dc.c table_dg.c \
#table_dp.c table_in.c table_ky.c table_rd.c table_rs.c

images.c : make-imglib Makefile images.in
	./make-imglib -n cards -i $(srcdir)/png -d images.d $(srcdir)/images.in images.c

make-imglib$(EXEEXT) : make-imglib.c
	$(BUILD_CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(srcdir)/make-imglib.c -o make-imglib -lpng -lz

text2c$(EXEEXT) : text2c.c
	$(BUILD_CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(srcdir)/text2c.c -o text2c -lpng -lz
