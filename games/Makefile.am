bin_PROGRAMS = \
	freecell \
	golf \
	mastermind \
	merlin \
	minesweeper \
	pegged \
	solitaire \
	taipedit \
	taipei \
	thornq

noinst_HEADERS = taipeilib.h taipei.h

INCLUDES = -I../lib $(X_CFLAGS)
LDFLAGS = $(X_LIBS) $(XPM_LIBS)

freecell_SOURCES = freecell.c freecell-help.c freecell-img.c
golf_SOURCES = golf.c golf-help.c golf-img.c
mastermind_SOURCES = mastermind.c mastermind-help.c mastermind-img.c
merlin_SOURCES = merlin.c merlin-help.c merlin-img.c
minesweeper_SOURCES = minesweeper.c minesweeper-help.c minesweeper-img.c
pegged_SOURCES = pegged.c pegged-help.c pegged-img.c
solitaire_SOURCES = solitaire.c solitaire-help.c solitaire-img.c
taipedit_SOURCES = taipedit.c taipeilib.c taipedit-help.c taipedit-img.c
taipei_SOURCES = taipei.c taipeilib.c taipei-help.c taipei-img.c
thornq_SOURCES = thornq.c thornq-help.c thornq-img.c

BUILT_SOURCES = freecell-help.c freecell-img.c golf-help.c golf-img.c \
	mastermind-help.c mastermind-img.c merlin-help.c merlin-img.c \
	minesweeper-help.c minesweeper-img.c pegged-help.c pegged-img.c \
	solitaire-help.c solitaire-img.c taipedit-help.c taipedit-img.c \
	taipei-help.c taipei-img.c

LDADD = ../lib/libcards.a -lXpm -lX11
freecell_LDADD = $(LDADD)
golf_LDADD = $(LDADD)
mastermind_LDADD = ../lib/nocards.o $(LDADD)
merlin_LDADD = ../lib/nocards.o $(LDADD)
minesweeper_LDADD = ../lib/nocards.o $(LDADD)
pegged_LDADD = ../lib/nocards.o $(LDADD)
solitaire_LDADD = $(LDADD)
taipedit_LDADD = ../lib/nocards.o $(LDADD)
taipei_LDADD = ../lib/nocards.o $(LDADD)
thornq_LDADD = $(LDADD)

EXTRA_DIST = $(wildcard *.gif) $(wildcard *.tp)

freecell-img.c : freecell.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ freecell.gif

golf-img.c : golf.gif golf-arrow.gif golf-noarrow.gif ../lib/gif2pack.c
	../lib/gif2pack -b appimglib -o $@ golf.gif golf-arrow.gif golf-noarrow.gif

MINESWEEPER_GIFS = minesweeper.gif minesweeper-x.gif minesweeper-t.gif \
	minesweeper-c12.gif minesweeper-c24.gif minesweeper-c36.gif \
	minesweeper-c48.gif

minesweeper-img.c : $(MINESWEEPER_GIFS) ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ $(MINESWEEPER_GIFS)

merlin-img.c : merlin.gif merlin-c.gif merlin-b.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ merlin.gif merlin-c.gif merlin-b.gif

mastermind-img.c : $(wildcard mastermind*.gif) ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ mastermind*.gif

pegged-img.c : pegged.gif pegged-h.gif pegged-p.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ pegged.gif pegged-h.gif pegged-p.gif

solitaire-img.c : solitaire.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ solitaire.gif

taipei-img.c : taipei.gif t/n1.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ taipei.gif t/*.gif

TAIPEDITGIF = t/xa.gif taipedit.gif taipedit-bu.gif taipedit-bs.gif \
	taipedit-bt.gif

taipedit-img.c : $(TAIPEDITGIF) t/n1.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ $(TAIPEDITGIF) t/n*.gif

THORNQ_GIF = thornq.gif thornq-arrow.gif thornq-noarrow.gif
thornq-img.c : $(THORNQ_GIF) ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ $(THORNQ_GIF)

%-help.c : %.html ../lib/text2c.c
	cd ../lib ; $(MAKE) text2c
	../lib/text2c $*_help < $< > $@

dist-hook :
	mkdir $(distdir)/t
	ln $(srcdir)/t/* $(distdir)/t

bin-tar :
	strip $(bin_PROGRAMS)
	tar cvf - $(bin_PROGRAMS) | gzip -9 > ../ace-bin.tar.gz
