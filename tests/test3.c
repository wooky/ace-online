/* The Ace of Penguins - test3.c
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

#include <stdlib.h>
#include <time.h>
#include "cards.h"

char *suits = "hdcs";
char *values = "a234567890jqk";

Picture *cards[52];
Picture *back;
int cx[52];
int cy[52];

Picture *empty;

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN

Stack *st;

int
main(int argc, char **argv)
{
  init_table(argc, argv, W+2*M+51*CARD_FAN_RIGHT, 2*M+H);
  table_loop();
}

void
init()
{
  int s, v, t;
  char name[30];

  for (s=0; s<4; s++)
    for (v=0; v<13; v++)
    {
      sprintf(name, "card/%c%c.gif", values[v], suits[s]);
      cards[v*4+s] = get_picture(name);
      cx[v*4+s] = M+v*(M+W);
      cy[v*4+s] = M+s*(M+H);
    }
  empty = get_picture("empty.gif");
  back = get_picture("back.gif");

  stack_set_pictures(cards, back);

  st = stack_create(CARD_MARGIN, CARD_MARGIN);
  stack_set_offset(st, STACK_OFFSET_RIGHT);
  stack_shuffle(st);
  for (s=0; s<52; s++)
  {
    if (rand() & 64)
      stack_add_card(st, s);
    else
      stack_add_card(st, s | 64);
  }
  stack_shuffle(st);
}

void
redraw()
{
  stack_redraw();
}
