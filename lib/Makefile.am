lib_LTLIBRARIES = libcards.la

noinst_PROGRAMS = text2c make-imglib
noinst_HEADERS = cards.h

CLEANFILES = images.c images.d

INCLUDES = $(X_CFLAGS) @PDA@
LDFLAGS = $(X_LIBS)

BUILD_CC = @BUILD_CC@
AR = @AR@
STRIP = @STRIP@

libcards_la_LDFLAGS = -version-info 1:0:0 $(LDFLAGS)

libcards_la_SOURCES = table.c help.c stack.c imagelib.c xwin.c \
	images.c table_ai.c table_ck.c table_dc.c table_dg.c \
	table_dp.c table_in.c table_ky.c table_rd.c table_rn.c table_rs.c

images.c : make-imglib Makefile images.in
	./make-imglib -n cards -i $(srcdir)/png -d images.d $(srcdir)/images.in images.c

make-imglib : make-imglib.c
	$(BUILD_CC) $(srcdir)/make-imglib.c -o make-imglib -lpng -lz

text2c : text2c.c
	$(BUILD_CC) $(srcdir)/text2c.c -o text2c -lpng -lz
