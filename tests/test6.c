/* The Ace of Penguins - test6.c
   Copyright (C) 1998, 2001 DJ Delorie

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "table.h"
#include "cards.h"

static Picture *card, *test6;

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN

static void
init()
{
  card = get_picture("as");
  test6 = get_picture("test6");
}

static void
redraw()
{
  put_picture(card, M, M, 0, 0, W, H);
  put_picture(test6, 2*M+W, M, 0, 0, W, H);
}

static FunctionMapping fmap[] = {
  { "init", (void *)init },
  { "redraw", (void *)redraw },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  register_imagelib(appimglib_imagelib);
  init_ace(argc, argv, fmap);
  init_table(3*M+2*W, 2*M+H);
  table_loop();
}
