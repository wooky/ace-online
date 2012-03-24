/* The Ace of Penguins - test5.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cards.h"

#define DOWN 0

static char *suits = "hdcs";
static char *values = "a234567890jqk";

static Picture *cards[52];

static Picture *empty, *back, *nodrop;

static Stack *st[4];

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN

static void
init()
{
  int s, v, t;
  char name[30];

  for (s=0; s<4; s++)
    for (v=0; v<13; v++)
    {
      sprintf(name, "%c%c", values[v], suits[s]);
      cards[v*4+s] = get_picture(name);
    }
  empty = get_picture("empty");
  back = get_picture("back");
  nodrop = get_picture("no-drop");
  stack_set_pictures(cards, back);

  for (s=0; s<4; s++)
  {
#if DOWN
    st[s] = stack_create(CARD_MARGIN+s*(CARD_MARGIN+CARD_WIDTH), CARD_MARGIN);
    stack_set_offset(st[s], STACK_OFFSET_DOWN);
#else
    st[s] = stack_create(CARD_MARGIN, CARD_MARGIN+s*(CARD_MARGIN+CARD_HEIGHT));
    stack_set_offset(st[s], STACK_OFFSET_RIGHT);
#endif
    for (v=0; v<13; v++)
      stack_add_card(st[s], MAKE_CARD(s, v, FACEUP));
  }
}

static void
redraw()
{
  int c;
  stack_redraw();
}

static void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
}

static Stack *src_stack = 0;
static int src_n = 0, src_maxn = 0;

static void
click(int x, int y, int b)
{
  if (stack_find(x, y, &src_stack, &src_n))
  {
    src_maxn = stack_count_cards(src_stack);
    stack_begin_drag(src_stack, src_n, x, y);
  }
  else
    src_stack = 0;
}

static void
drag(int x, int y, int b)
{
  stack_continue_drag((y/2)%(src_maxn+5), x, y);
}

static void
drop(int x, int y, int b)
{
  Stack *dest_stack;
  if (src_stack && stack_drag_find(x, y, &dest_stack))
    stack_drop(dest_stack, src_n);
  else
    stack_drop(src_stack, src_n);
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
  { "drag", (void *)drag },
  { "drop", (void *)drop },
  { "init", (void *)init },
  { "key", (void *)key },
  { "redraw", (void *)redraw },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  init_ace(argc, argv, fmap);
#if DOWN
  init_table(4*W+5*M, 2*M+51*CARD_FAN_DOWN+H);
#else
  init_table(2*M+51*CARD_FAN_RIGHT+W, 4*H+5*M);
#endif
  table_loop();
}
