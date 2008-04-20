bin_PROGRAMS = \
	canfield \
	freecell \
	golf \
	mastermind \
	merlin \
	minesweeper \
	pegged \
	solitaire \
	spider \
	taipedit \
	taipei \
	thornq

noinst_HEADERS = taipeilib.h taipei.h

CLEANFILES = \
	canfield-img.c canfield-help.c \
	freecell-img.c freecell-help.c \
	golf-img.c golf-help.c \
	mastermind-img.c mastermind-help.c \
	merlin-img.c merlin-help.c \
	minesweeper-img.c minesweeper-help.c \
	pegged-img.c pegged-help.c \
	solitaire-img.c solitaire-help.c \
	spider-img.c spider-help.c \
	taipedit-img.c taipedit-help.c \
	taipei-img.c taipei-help.c \
	thornq-img.c thornq-help.c

STRIP = @STRIP@

INCLUDES = -I$(srcdir)/../lib $(X_CFLAGS) @PDA@
AM_LDFLAGS = $(X_LIBS)

canfield_SOURCES = canfield.c canfield-img.c
freecell_SOURCES = freecell.c freecell-help.c freecell-img.c
golf_SOURCES = golf.c golf-help.c golf-img.c
mastermind_SOURCES = mastermind.c mastermind-help.c mastermind-img.c
merlin_SOURCES = merlin.c merlin-help.c merlin-img.c
minesweeper_SOURCES = minesweeper.c minesweeper-help.c minesweeper-img.c
pegged_SOURCES = pegged.c pegged-help.c pegged-img.c
solitaire_SOURCES = solitaire.c solitaire-help.c solitaire-img.c
spider_SOURCES = spider.c spider-help.c spider-img.c
taipedit_SOURCES = taipedit.c taipeilib.c taipedit-help.c taipedit-img.c
taipei_SOURCES = taipei.c taipeilib.c taipei-help.c taipei-img.c
thornq_SOURCES = thornq.c thornq-help.c thornq-img.c

BUILT_SOURCES = canfield-img.c freecell-help.c freecell-img.c golf-help.c golf-img.c \
	mastermind-help.c mastermind-img.c merlin-help.c merlin-img.c \
	minesweeper-help.c minesweeper-img.c pegged-help.c pegged-img.c \
	solitaire-help.c solitaire-img.c spider-help.c spider-img.c \
	taipedit-help.c taipedit-img.c taipei-help.c taipei-img.c

LDADD = ../lib/libcards.la -lpng -lz -lX11 -lm

EXTRA_DIST = $(wildcard *.png) $(wildcard *.tp)



canfield-img.c : canfield.png ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ canfield.png



freecell-img.c : freecell.png ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ freecell.png



golf-img.c : golf.png golf-arrow.png golf-noarrow.png ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ golf.png golf-arrow.png golf-noarrow.png



MINESWEEPER_PNGS = minesweeper.png minesweeper-x.png minesweeper-t.png \
	minesweeper-c12.png minesweeper-c24.png minesweeper-c36.png \
	minesweeper-c48.png

minesweeper-img.c : $(MINESWEEPER_PNGS) ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ $(MINESWEEPER_PNGS)



merlin-img.c : merlin.png merlin-c.png merlin-b.png ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ merlin.png merlin-c.png merlin-b.png

MASTERMIND_PNGS=mastermind-b.png mastermind-c.png mastermind-e.png \
	mastermind-eb.png mastermind-g.png mastermind-k.png \
	mastermind-o.png mastermind-p.png mastermind-r.png \
	mastermind-w.png mastermind-y.png mastermind.png

mastermind-img.c : $(MASTERMIND_PNGS) ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ $(MASTERMIND_PNGS)



pegged-img.c : pegged.png pegged-h.png pegged-p.png ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ pegged.png pegged-h.png pegged-p.png



solitaire-img.c : solitaire.png ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ solitaire.png


spider-img.c : spider.png golf-arrow.png golf-noarrow.png 
	../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ spider.png golf-arrow.png golf-noarrow.png


taipei-img.c : ../lib/make-imglib taipei.png taipei-tiles.png Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ taipei taipei-tiles

TAIPEDITPNG = taipedit.png taipedit-bu.png taipedit-bs.png taipedit-bt.png taipei-tiles.png

taipedit-img.c : ../lib/make-imglib $(TAIPEDITPNG) Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ $(TAIPEDITPNG)



THORNQ_PNG = thornq.png thornq-arrow.png thornq-noarrow.png
thornq-img.c : $(THORNQ_PNG) ../lib/make-imglib Makefile.am
	../lib/make-imglib -i $(srcdir) -n appimglib - $@ $(THORNQ_PNG)



%-help.c : %.html ../lib/text2c
	../lib/text2c $*_help < $< > $@

strip :
	$(STRIP) $(bin_PROGRAMS)

bin-tar :
	$(STRIP) $(bin_PROGRAMS)
	tar cvf - $(bin_PROGRAMS) | gzip -9 > ../ace-bin.tar.gz
