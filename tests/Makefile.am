noinst_PROGRAMS = test1 test3 test4 test5 test6
bin_PROGRAMS = penguins
test6_SOURCES = test6.c test6c.c

INCLUDES = -I../lib $(X_CFLAGS)
LDFLAGS = $(X_LIBS) $(XPM_LIBS)

LDADD = ../lib/libcards.a -lXpm -lX11

EXTRA_DIST = test6.gif

test6c.c : test6.gif ../lib/gif2pack.c
	cd ../lib ; $(MAKE) gif2pack
	../lib/gif2pack -b appimglib -o $@ test6.gif
