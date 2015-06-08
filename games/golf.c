/* The Ace of Penguins - golf.c
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
#include <X11/Xlib.h>

#include "table.h"
#include "imagelib.h"
#include "cards.h"

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define R CARD_FAN_TBRIGHT
#define D stack_fan_down

static int stack_fan_down;

static Picture *splash, *youwin, *youlose;
static Picture *arrow, *no_arrow;

static Stack *deck, *discard, *stacks[7];

static int supress_arrows = 0;

static int arrow_offset, arrow_delta;

static int arrows[7] = { 0, 0, 0, 0, 0, 0, 0 };

static void
set_arrow(int column, int on)
{
  int x = column * arrow_delta + arrow_offset;
  if (supress_arrows)
    return;
  if (arrows[column])
    put_picture(no_arrow, x, arrows[column],
		0, 0, no_arrow->w, no_arrow->h);
  if (on)
  {
    arrows[column] = M+H+(stack_count_cards(stacks[column])-1)*D + 2;
    put_picture(arrow, x, arrows[column],
		0, 0, arrow->w, arrow->h);
  }
  else
    arrows[column] = 0;
}

static void
redraw_arrows()
{
  int i;
  for (i=0; i<7; i++)
  {
    int x = i * arrow_delta + arrow_offset;
    if (arrows[i])
      put_picture(arrow, x, arrows[i],
		  0, 0, arrow->w, arrow->h);
  }
}

static void
clear_arrows()
{
  int i;
  for (i=0; i<7; i++)
    set_arrow(i, 0);
}

static void
set_arrows()
{
  int i;
  int top_discard = stack_get_card(discard, stack_count_cards(discard)-1);
  for (i=0; i<7; i++)
  {
    Stack *s = stacks[i];
    int top_stack = stack_get_card(s, stack_count_cards(s)-1);
    if (VALUE(top_discard) != KING
      && (VALUE(top_discard) == VALUE(top_stack)+1
	  || VALUE(top_discard) == VALUE(top_stack)-1))
      set_arrow(i, 1);
    else
      set_arrow(i, 0);
  }
}

static void
show_count()
{
  char s[10];
  int c = stack_count_cards(deck);
  if (c > 0)
    sprintf(s, "%2d", c);
  else
    strcpy(s, "  ");
  text(s, M*2, table_height-M-H-2);
}

static void
start_again()
{
  int i, j;

  clear_arrows();

  stack_flip_stack(discard, deck, 0);
  for (i=0; i<7; i++)
    stack_flip_stack(stacks[i], deck, 0);

  stack_shuffle(deck);
  stack_shuffle(deck);
  stack_shuffle(deck);

  for (i=0; i<7; i++)
    for (j=0; j<5; j++)
      stack_flip_card(deck, stacks[i], 0);

  stack_flip_card(deck, discard, 0);

  stack_undo_reset();
  show_count();
  set_arrows();
}

static void
init()
{
  int s, v;

  arrow_offset = (table_width - 7 * W) / 8;
  arrow_delta = arrow_offset + W;
  arrow_offset = (table_width - 7*W - 6*arrow_offset)/2;

  stack_load_standard_deck();
  splash = get_picture("golf");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");
  arrow = get_picture("golf-arrow");
  no_arrow = get_picture("golf-noarrow");

  set_centered_pic(splash);

  for (s=0; s<7; s++)
  {
    stacks[s] = stack_create(arrow_offset+s*arrow_delta, M);
    stack_set_offset(stacks[s], STACK_OFFSET_DOWN);
  }

  deck = stack_create(M, table_height-M-H);
  discard = stack_create(M*2+W, table_height-M-H);
  stack_set_offset(discard, STACK_OFFSET_TBRIGHT);

  for (s=0; s<4; s++)
    for (v=1; v<=13; v++)
      stack_add_card(deck, MAKE_CARD(s, v, FACEDOWN));

  arrow_offset = arrow_offset + (W - arrow->w)/2;

  stack_get_fans(&stack_fan_down, NULL, NULL, NULL);

  start_again();
}

static void
check_for_endgame()
{
  int c, top_discard, top_stack;

  if (stack_count_cards(discard) == 52)
  {
    set_centered_pic(youwin);
    return;
  }

  if (stack_count_cards(deck) == 0)
  {
    top_discard = stack_get_card(discard, stack_count_cards(discard)-1);
    if (VALUE(top_discard) != KING)
      for (c=0; c<7; c++)
	if (stack_count_cards(stacks[c]) > 0)
	{
	  top_stack = stack_get_card(stacks[c], stack_count_cards(stacks[c])-1);
	  if (VALUE(top_discard) == VALUE(top_stack)+1
	      || VALUE(top_discard) == VALUE(top_stack)-1)
	    return;
	}
    set_centered_pic(youlose);
  }
}

static void
redraw()
{
  stack_redraw();
  show_count();
  redraw_arrows();
}

static void
click(int x, int y, int b)
{
  int c, f, n;
  Stack *s;
  int top_discard, top_stack;
  Picture *cp = get_centered_pic();

  if ((cp == youlose || cp == youwin)
      && (x > table_width/2-cp->w/2
	  && x < table_width/2+cp->w/2
	  && y > table_height/2-cp->h/2
	  && y < table_height/2+cp->h/2))
  {
    set_centered_pic(0);
    start_again();
    return;
  }
  if (cp == splash)
  {
    set_centered_pic(0);
    return;
  }

  if (b > 1)
    s = deck;
  else
  {
    f = stack_find(x, y, &s, &n);
    if (!f)
      return;
  }

  if (s == deck)
  {
    stack_flip_card(deck, discard, 0);
    show_count();
    set_arrows();
    check_for_endgame();
    return;
  }
  else if (s == discard)
    return;

  if (stack_count_cards(s) == 0)
    return;

  top_discard = stack_get_card(discard, stack_count_cards(discard)-1);
  top_stack = stack_get_card(s, stack_count_cards(s)-1);

  if (VALUE(top_discard) == KING)
    return;
  if (VALUE(top_discard) != VALUE(top_stack)+1
      && VALUE(top_discard) != VALUE(top_stack)-1)
    return;

  stack_move_card(s, discard, 0);
  set_arrows();

  check_for_endgame();
}

extern char golf_help[];

static void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  if (k == KEY_F(1) || k == 'h')
  {
    set_centered_pic(0);
    help("golf.html", golf_help);
  }
  if (k == KEY_F(2) || k == 'r')
  {
    set_centered_pic(0);
    start_again();
  }
  if (k == 8 || k == 127 || k == KEY_DELETE)
  {
    clear_arrows();
    supress_arrows = 1;
    set_centered_pic(0);
    stack_undo();
    show_count();
    supress_arrows = 0;
    set_arrows();
  }
  if (k == 'd')
  {
    int i;
    for (i=0; i<7; i++)
      printf("i=%d a=%d\n", i, arrows[i]);
  }
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
  { "init", (void *)init },
  { "key", (void *)key },
  { "redraw", (void *)redraw },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  int alt_width = 3*M+2*W+51*R;
  init_ace(argc, argv, fmap);
  register_imagelib(appimglib_imagelib);
  table_width = 8*M+7*W;
  if (table_width < alt_width) table_width = alt_width;
  table_height = 3*M+5*CARD_FAN_DOWN+2*H;
  init_table(table_width, table_height);
  table_loop();
}
