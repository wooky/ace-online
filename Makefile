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

all :
	$(MAKE) -C lib
	$(MAKE) -C tests
	$(MAKE) -C games

% :
	$(MAKE) -C lib $@
	$(MAKE) -C tests $@
	$(MAKE) -C games $@

SRC = \
	Makefile \
	COPYING \
	ChangeLog \
	*/ChangeLog \
	docs/* \
	lib/*.gif \
	lib/*.h \
	lib/Makefile \
	lib/[abd-t]*.c \
	tests/*.c \
	tests/*.gif \
	tests/Makefile \
	games/*.gif \
	games/*.html \
	games/*[a-fh-oq-z].c \
	games/*.h \
	games/*.tp \
	games/split-tiles \
	games/Makefile \
	games/t/* \
	$E

tar :
	tar cfz ace-src.tar.gz `ls -1 ${SRC} | sort`
