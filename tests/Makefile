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
#CFLAGS = -O -MMD -I../lib
CFLAGS = -O -MMD -I../lib -L/usr/X11R6/lib

.SUFFIXES:
.PRECIOUS: %.o

TESTS = $(basename $(wildcard test?.c))

all : $(TESTS)

test% : test%.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

test6 : test6.o test6c.o ../lib/cardlib.a
	$(CC) $(CFLAGS) -o $@ $^ -lXpm -lX11

test6c.c : test6.gif ../lib/gif2pack
	../lib/gif2pack -b appimglib -o test6c.c test6.gif

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean :
	-@rm -f *.o *.d test[1-6] test6c.* *~

D=$(wildcard *.d)
ifneq ($D,)
include $D
endif
