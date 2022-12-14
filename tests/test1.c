/* The Ace of Penguins - test1.c
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

#include <stdio.h>
#include <string.h>

#include "table.h"
#include "cards.h"

static Picture *cards[4][13];
static char *suits = "hdcs";
static char *values = "a234567890jqk";

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN

static void
init()
{
  int s, v;
  char name[30];
  for (s=0; s<4; s++)
    for (v=0; v<13; v++)
    {
      sprintf(name, "%c%c", values[v], suits[s]);
      cards[s][v] = get_picture(name);
    }
}

static void
redraw()
{
  int s, v;

  for (s=0; s<4; s++)
    for (v=0; v<13; v++)
      put_picture(cards[s][v], M+v*CARD_FAN_RIGHT, M+s*(M+H), 0, 0, W, H);

  for (s=0; s<4; s++)
    for (v=0; v<13; v++)
      put_picture(cards[s][v], 300+s*(W+M), M+v*CARD_FAN_DOWN, 0, 0, W, H);

  for (s=0; s<4; s++)
    put_picture(cards[s][0], 300+s*(W+M), M+3*(M+H), 0, 0, W, H);
}

static FunctionMapping fmap[] = {
  { "init", (void *)init },
  { "redraw", (void *)redraw },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  init_ace(argc, argv, fmap);
  init_table(300+4*(W+M), 4*H+5*M);
  table_loop();
}
