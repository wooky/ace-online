# Makefile for the Ace of Penguins
# Copyright (C) 1998 DJ Delorie.  See the file COPYING for details

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

CC = gcc
CFLAGS = -g -O6 -MMD -I../lib -L/usr/X11R6/lib

.SUFFIXES:
.PRECIOUS: .o .c

GAMES = \
	freecell \
	golf \
	minesweeper \
	pegged \
	solitaire \
	taipei taipedit \
	$E

all : LIB $(GAMES)

% : %.o %-help.o %-img.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

%-help.c : %.html ../lib/text2c
	../lib/text2c $*_help < $< > $@

%-img.c : %.gif ../lib/gif2pack
	../lib/gif2pack -b appimglib -o $@ $<

golf-img.c : golf.gif golf-arrow.gif golf-noarrow.gif ../lib/gif2pack
	../lib/gif2pack -b appimglib -o $@ golf.gif golf-arrow.gif golf-noarrow.gif

MINESWEEPER_GIFS = minesweeper.gif minesweeper-x.gif minesweeper-t.gif \
	     minesweeper-c12.gif minesweeper-c24.gif minesweeper-c36.gif minesweeper-c48.gif

minesweeper-img.c : $(MINESWEEPER_GIFS) ../lib/gif2pack
	../lib/gif2pack -b appimglib -o $@ $(MINESWEEPER_GIFS)

minesweeper : minesweeper.o minesweeper-help.o minesweeper-img.o ../lib/nocards.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

pegged-img.c : pegged.gif pegged-h.gif pegged-p.gif ../lib/gif2pack
	../lib/gif2pack -b appimglib -o $@ pegged.gif pegged-h.gif pegged-p.gif

pegged : pegged.o pegged-help.o pegged-img.o ../lib/nocards.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

taipei : taipei.o taipeilib.o taipei-help.o taipei-img.o ../lib/nocards.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

taipedit : taipedit.o taipeilib.o taipedit-help.o taipedit-img.o ../lib/nocards.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

TAIPEDITGIF = t/xa.gif taipedit.gif taipedit-bu.gif taipedit-bs.gif taipedit-bt.gif

taipei-img.c : taipei.gif t/n1.gif ../lib/gif2pack
	../lib/gif2pack -b appimglib -o $@ taipei.gif t/*.gif

taipedit-img.c : $(TAIPEDITGIF) t/n1.gif ../lib/gif2pack
	../lib/gif2pack -b appimglib -o $@ $(TAIPEDITGIF) t/n*.gif

ifneq ($(wildcard /usr/local/bin/ppmtogif),)
t/n1.gif : tiles.gif split-tiles
	./split-tiles
endif

%.o : %.c
	$(CC) $(CFLAGS) -c $<

LIB :
	@$(MAKE) -C ../lib

clean :
	-rm -f *.o *.d $(GAMES) *-img.c *-help.c *~

tar :
	strip $(GAMES)
	tar cvf - $(GAMES) | gzip -9 > ../ace-bin.tar.gz

D=$(wildcard *.d)
ifneq ($D,)
include $D
endif
