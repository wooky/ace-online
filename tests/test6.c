/* The Ace of Penguins - test6.c
   Copyright (C) 1998 DJ Delorie

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

#include "cards.h"

Picture *card, *test6;

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN

int
main(int argc, char **argv)
{
  init_table(argc, argv, 3*M+2*W, 2*M+H);
  table_loop();
}

void
init()
{
  card = get_picture("card/as.gif");
  test6 = get_picture("test6.gif");
}

void
redraw()
{
  put_picture(card, M, M, 0, 0, W, H);
  put_picture(test6, 2*M+W, M, 0, 0, W, H);
}
