noinst_PROGRAMS = make-cards text2c gif2pack
noinst_LIBRARIES = libcards.a
noinst_DATA = nocards.o
noinst_HEADERS = cards.h

INCLUDES = $(GD_CFLAGS) $(X_CFLAGS) $(XPM_CFLAGS)

make_cards_LDADD = $(GD_LIBS) -lm
gif2pack_LDADD = $(GD_LIBS) -lm

BUILT_SOURCES = cards.c nocards.c

libcards_a_SOURCES = table.c cards.c help.c stack.c \
	table_ai.c table_ck.c table_dc.c table_dg.c \
	table_dp.c table_in.c table_ky.c table_rd.c table_rn.c

SRCGIFS = \
	blank.gif \
	pjack.gif \
	pqueen.gif \
	pking.gif \
	a-k.gif \
	suits.gif \
	smsuits.gif \
	bigsuits.gif \
	penguin.gif

GIFS = \
	xemboss.gif \
	no-drop.gif \
	splash.gif \
	youwin.gif \
	youlose.gif

CARDGIFS = \
	empty.gif \
	back.gif

card/as.gif : make-cards.c $(SRCGIFS)
	$(MAKE) make-cards
	./make-cards

cards.c : gif2pack.c card/as.gif $(CARDGIFS) $(GIFS)
	$(MAKE) gif2pack
	@echo Warning - compressing takes a long time.
	./gif2pack -b imglib -o cards.c card/*.gif $(CARDGIFS) $(GIFS)

nocards.c : gif2pack.c card/as.gif $(GIFS)
	$(MAKE) gif2pack
	./gif2pack -b imglib -o nocards.c card/as.gif $(GIFS)

EXTRA_DIST = $(SRCGIFS) $(GIFS) $(CARDGIFS)

dist-hook :
	mkdir $(distdir)/card
	ln $(srcdir)/card/* $(distdir)/card
