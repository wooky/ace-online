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
CFLAGS = -g -O6 -MMD -L/usr/X11R6/lib

.PRECIOUS: %.o

all : make-cards text2c gif2pack cardlib.a nocards.o

make-cards : make-cards.c
	$(CC) $(CFLAGS) -o $@ $^ -lgd -lm

text2c : text2c.c
	$(CC) $(CFLAGS) -o $@ $^

gif2pack : gif2pack.c
	$(CC) $(CFLAGS) -o $@ $^ -lgd -lm

SRCGIFS = \
	blank.gif \
	pjack.gif \
	pqueen.gif \
	pking.gif \
	a-k.gif \
	suits.gif \
	smsuits.gif \
	bigsuits.gif \
	penguin.gif \
	$E

GIFS = \
	xemboss.gif \
	no-drop.gif \
	splash.gif \
	youwin.gif \
	youlose.gif \
	$E

CGIFS = \
	empty.gif \
	back.gif \
	$(GIFS) \
	$E

card/as.gif : make-cards $(SRCGIFS)
	./make-cards

# Note that this works correctly only when "make" makes gif2pack first.
ifeq ($(wildcard empty.gif),empty.gif)
cards.c : gif2pack.c card/as.gif $(CGIFS)
	./gif2pack -b imglib -o cards.c card/*.gif $(CGIFS)
nocards.c : gif2pack.c card/as.gif $(GIFS)
	./gif2pack -b imglib -o nocards.c card/as.gif $(GIFS)
endif

CARDOBJS = \
	table.o \
	cards.o \
	help.o \
	stack.o \
	table_ai.o \
	table_ck.o \
	table_dc.o \
	table_dg.o \
	table_dp.o \
	table_in.o \
	table_ky.o \
	table_rd.o \
	$E

cardlib.a : $(CARDOBJS)
	-@rm -f $@
	ar rvs $@ $^

clean :
	-@rm -f *.o *.a *.d *~ text2c gif2pack make-cards

D=$(wildcard *.d)
ifneq ($D,)
include $D
endif
