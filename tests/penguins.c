/* The Ace of Penguins - test2.c
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
#include <time.h>
#include "cards.h"

static char *suits = "hdcs";
static char *values = "a234567890jqk";

static Picture *cards[52];
static int cx[52];
static int cy[52];

static Picture *empty;

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN

static void
init()
{
  int s, v, t;
  char name[30];
  int shuffle[52];

  for (s=0; s<4; s++)
    for (v=0; v<13; v++)
    {
      sprintf(name, "%c%c", values[v], suits[s]);
      cards[v*4+s] = get_picture(name);
      cx[v*4+s] = M+v*(M+W);
      cy[v*4+s] = M+s*(M+H);
    }
  empty = get_picture("empty");

  /* shuffle! */
  srand(time(0));
  for (s=0; s<52; s++)
  {
    v = (rand() % (52-s)) + s;
    t = cx[v];
    cx[v] = cx[s];
    cx[s] = t;
    t = cy[v];
    cy[v] = cy[s];
    cy[s] = t;
  }
}

static void
redraw()
{
  int c;
  put_picture(empty, M, M, 0, 0, W, H);
  for (c=0; c<52; c++)
    put_picture(cards[c], cx[c], cy[c], 0, 0, W, H);
}

static int selected_card = -1;
static int offset_x, offset_y;

static int
xy2card(int x, int y)
{
  int i;
  for (i=51; i>=0; i--)
    if (cx[i] <= x && x < cx[i]+W
	&& cy[i] <= y && y < cy[i]+H)
      return i;
  return -1;
}

static void
click(int x, int y, int b)
{
  int i;
  int old_x, old_y;
  Picture *c;
  selected_card = xy2card(x, y);
  if (selected_card == -1)
    return;
  c = cards[selected_card];
  old_x = cx[selected_card];
  old_y = cy[selected_card];
  for (i=selected_card; i<51; i++)
  {
    cards[i] = cards[i+1];
    cx[i] = cx[i+1];
    cy[i] = cy[i+1];
  }
  cards[51] = c;
  cx[51] = old_x;
  cy[51] = old_y;
  selected_card = 51;
  put_picture(cards[selected_card],
	      cx[selected_card], cy[selected_card],
	      0, 0, W, H);
  offset_x = x - cx[selected_card];
  offset_y = y - cy[selected_card];
}

static void
double_click(int x, int y, int b)
{
  int old_x, old_y, i;
  Picture *c;
  selected_card = xy2card(x, y);
  if (selected_card == -1)
    return;
  c = cards[selected_card];
  old_x = cx[selected_card];
  old_y = cy[selected_card];
  for (i=selected_card; i>0; i--)
  {
    cards[i] = cards[i-1];
    cx[i] = cx[i-1];
    cy[i] = cy[i-1];
  }
  cards[0] = c;
  cx[0] = old_x;
  cy[0] = old_y;
  selected_card = 0;
  invalidate(cx[0], cy[0], W, H);
}

static void
drag(int x, int y, int b)
{
  int old_x, old_y;
  if (selected_card == -1)
    return;
  old_x = cx[selected_card];
  old_y = cy[selected_card];
  cx[selected_card] = x - offset_x;
  cy[selected_card] = y - offset_y;
  invalidate_exposure(old_x, old_y, W, H,
		      cx[selected_card], cy[selected_card], W, H);
  if (selected_card == 51)
    put_picture(cards[selected_card], cx[selected_card], cy[selected_card], 0, 0, W, H);
  else
    invalidate_nc(cx[selected_card], cy[selected_card], W, H);
}

static void
drop(int x, int y, int b)
{
  if (selected_card == -1)
    return;
  snap_to_grid(&x, &y,
	       M+W, M+H,
	       M+offset_x, M+offset_y,
	       15);
  drag(x, y, b);
  selected_card = -1;
}

static void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
  { "double_click", (void *)double_click },
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
  init_table(13*W+14*M, 5*H+6*M);
  table_loop();
}
