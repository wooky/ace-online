/* The Ace of Penguins - golf.c
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

#include <X11/Xlib.h>
#include "cards.h"

#include <X11/keysym.h>

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define R 2
#define D CARD_FAN_DOWN

Picture *splash, *youwin, *youlose;
Picture *arrow, *no_arrow;

Stack *deck, *discard, *stacks[7];

static int table_width, table_height;

int supress_arrows = 0;

int
main(int argc, char **argv)
{
  int alt_width = 3*M+2*W+51*R;
  table_width = 8*M+7*W;
  if (table_width < alt_width) table_width = alt_width;
  table_height = 3*M+5*D+2*H;
  init_table(argc, argv, table_width, table_height);
  table_loop();
}

static int arrows[7] = { 0, 0, 0, 0, 0, 0, 0 };

static void
set_arrow(int column, int on)
{
  int x = (M+W)*column+M+W/2;
  if (supress_arrows)
    return;
  if (arrows[column])
    put_picture(no_arrow, x-no_arrow->w/2, arrows[column],
		0, 0, no_arrow->w, no_arrow->h);
  if (on)
  {
    arrows[column] = M+H+(stack_count_cards(stacks[column])-1)*D + 2;
    put_picture(arrow, x-arrow->w/2, arrows[column],
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
    int x = (M+W)*i+M+W/2;
    if (arrows[i])
      put_picture(arrow, x-arrow->w/2, arrows[i],
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

void
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

void
start_again()
{
  int i, j;

  clear_arrows();

  stack_flip_stack(discard, deck);
  for (i=0; i<7; i++)
    stack_flip_stack(stacks[i], deck);

  stack_shuffle(deck);
  stack_shuffle(deck);
  stack_shuffle(deck);

  for (i=0; i<7; i++)
    for (j=0; j<5; j++)
      stack_flip_card(deck, stacks[i]);

  stack_flip_card(deck, discard);

  stack_undo_reset();
  show_count();
  set_arrows();
}

void
init()
{
  int s, v;

  stack_load_standard_deck();
  splash = get_picture("golf.gif");
  youwin = get_picture("youwin.gif");
  youlose = get_picture("youlose.gif");
  arrow = get_picture("golf-arrow.gif");
  no_arrow = get_picture("golf-noarrow.gif");

  set_centered_pic(splash);

  for (s=0; s<7; s++)
  {
    stacks[s] = stack_create(M+s*(M+W), M);
    stack_set_offset(stacks[s], STACK_OFFSET_DOWN);
  }

  deck = stack_create(M, table_height-M-H);
  discard = stack_create(M*2+W, table_height-M-H);
  stack_set_offset(discard, STACK_OFFSET_TBRIGHT);

  for (s=0; s<4; s++)
    for (v=1; v<=13; v++)
      stack_add_card(deck, MAKE_CARD(s, v, FACEDOWN));

  start_again();
}

void
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

void
redraw()
{
  stack_redraw();
  show_count();
  redraw_arrows();
}

void
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
    stack_flip_card(deck, discard);
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

  stack_flip_card(s, discard);
  set_arrows();

  check_for_endgame();
}

extern char golf_help[];

void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  if (k == XK_F1 || k == 'h')
  {
    set_centered_pic(0);
    help("golf.html", golf_help);
  }
  if (k == XK_F2)
  {
    set_centered_pic(0);
    start_again();
  }
  if ((k == 8 || k == 127
       || k == XK_BackSpace || k == XK_Delete || k == XK_KP_Delete))
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
