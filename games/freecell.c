/* The Ace of Penguins - freecell.c
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

#include <X11/keysym.h>

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define F CARD_FAN_DOWN

Picture *xlogo, *splash, *youwin, *youlose;

Stack *deck;
Stack *freecells[4];
Stack *outcells[4];
Stack *maincells[8];

extern int table_width, table_height;

static int auto_move();

int
main(int argc, char **argv)
{
  xlogo = get_picture("xemboss.gif");
  splash = get_picture("freecell.gif");
  youwin = get_picture("youwin.gif");
  youlose = get_picture("youlose.gif");
  init_table(argc, argv, 8*W+9*M, H+M+19*F);
  table_loop();
}

static void
start_again()
{
  int i;
  for (i=0; i<4; i++)
  {
    stack_flip_stack(freecells[i], deck);
    stack_flip_stack(outcells[i], deck);
  }
  for (i=0; i<8; i++)
    stack_flip_stack(maincells[i], deck);
  stack_shuffle(deck);
  stack_shuffle(deck);
  stack_shuffle(deck);
  for (i=0; i<52; i++)
    stack_flip_card(deck, maincells[i%8]);

  stack_undo_reset();
}

void
init()
{
  int s, v;
  char name[30];
  Picture *empty;

  stack_load_standard_deck();
  empty = get_picture("empty.gif");

  set_centered_pic(splash);

  for (s=0; s<4; s++)
  {
    freecells[s] = stack_create(s*W, 0);
    stack_set_empty_picture(freecells[s], empty);

    outcells[s] = stack_create(4*W+9*M+s*W, 0);
    stack_set_empty_picture(outcells[s], empty);
  }

  for (s=0; s<8; s++)
  {
    maincells[s] = stack_create(M+s*(M+W), H+M);
    stack_set_offset(maincells[s], STACK_OFFSET_DOWN);
  }

  deck = stack_create(10*W, 0);
  for (s=0; s<4; s++)
    for (v=ACE; v<=KING; v++)
      stack_add_card(deck, MAKE_CARD(s, v, FACEUP));

  start_again();
}

void
redraw()
{
  stack_redraw();
  put_picture(xlogo, W*4+(M*9)/2-xlogo->w/2, H/2-xlogo->h/2,
	      0, 0, xlogo->h, xlogo->w);
}

extern char freecell_help[];

void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  if (k == XK_F1 || k == 'h')
  {
    set_centered_pic(0);
    help("freecell.html", freecell_help);
    return;
  }
  if (k == XK_F2)
  {
    set_centered_pic(0);
    start_again();
    while (auto_move());
  }
  if ((k == 8 || k == 127
       || k == XK_BackSpace || k == XK_Delete || k == XK_KP_Delete))
  {
    set_centered_pic(0);
    stack_undo();
  }
}

char *card_string(int c)
{
  static char names[5][5];
  static int n=0;
  static char suits[] = "HDCS";
  static char face[] = "A234567890JQK";
  n = (n+1)%5;
  sprintf(names[n], "%c%c", face[VALUE(c)], suits[SUIT(c)]);
  return names[n];
}

static Stack *src_stack = 0;
static int src_n = 0, src_maxn = 0;

static Stack *dest_stack;
static int last_n=-1;

static int
max_freecells(Stack *excl1, Stack *excl2)
{
  int num, empty, i;
  empty = 0;
  num = 1;
  for (i=0; i<4; i++)
    if (stack_count_cards(freecells[i]) == 0)
      num++;
  for (i=0; i<8; i++)
    if (stack_count_cards(maincells[i]) == 0
	&& maincells[i] != excl1
	&& maincells[i] != excl2)
      num *= 2;
  return num;
}

static int
n_droppable_s(Stack *dest_stack)
{
  int src_top, dest_top, src_count, dest_count, i, max_free;
  int src_value, src_suit, dest_value, dest_suit;
  src_count = stack_count_cards(src_stack);
  dest_count = stack_count_cards(dest_stack);
  dest_top = stack_get_card(dest_stack, dest_count-1);
  src_top = stack_get_card(src_stack, src_count-1);

  if (dest_stack == src_stack)
    return src_n;

  max_free = max_freecells(dest_stack, src_stack);

  for (i=0; i<4; i++)
    if (dest_stack == freecells[i])
    {
      if (stack_count_cards(freecells[i]) == 0)
	return src_count-1;
      return src_count;
    }

  for (i=0; i<4; i++)
    if (dest_stack == outcells[i])
    {
      if (dest_count == 0 && VALUE(src_top) == ACE)
	return src_count-1;
      if (dest_count > 0 && VALUE(src_top) == VALUE(dest_top)+1
	  && SUIT(src_top) == SUIT(dest_top))
	return src_count-1;
      return src_count;
    }

  for (i=0; i<8; i++)
    if (dest_stack == maincells[i])
    {
      int n = src_count-1;

      if (src_n == 0 && dest_count == 0)
	return 0;

      while (1)
      {
	int src_next;

	src_top = stack_get_card(src_stack, n);
	if ((COLOR(src_top) != COLOR(dest_top)
	     && VALUE(src_top) == VALUE(dest_top)-1)
	    || (n == src_n && dest_count == 0))
	  return n;

	src_next = stack_get_card(src_stack, n-1);
	if (n == 0 || COLOR(src_top) == COLOR(src_next)
	    || VALUE(src_top) != VALUE(src_next)-1)
	{
	  if (dest_count == 0)
	    return n;
	  break;
	}

	if (n <= src_count - max_free)
	  break;

	n--;
      }
      return src_count;
    }

  return src_count;
}

static int
n_droppable(int x, int y)
{
  if (!src_stack)
    return 0;
  if (!stack_drag_find(x, y, &dest_stack))
  {
    dest_stack = src_stack;
    return last_n != -1 ? last_n : src_n;
  }
  return n_droppable_s(dest_stack);
}

static int lowest[2];
static int pile_for[4];
static int top_card[4];

static int
auto_move_stack(Stack *s)
{
  int i, c, n = stack_count_cards(s);
  if (n == 0)
    return 0;
  c = stack_get_card(s, n-1);
  if (FACEDOWNP(c))
  {
    stack_flip_card(s, s);
    c = stack_get_card(s, n-1);
    return 1;
  }
  if (VALUE(c) == ACE)
  {
    for (i=0; i<4; i++)
      if (stack_count_cards(outcells[i]) == 0)
      {
	stack_animate(s, outcells[i]);
	return 1;
      }
    return 0; /* can't happen? */
  }
  if (pile_for[SUIT(c)] == -1)
    return 0;
  if (VALUE(c) == VALUE(top_card[pile_for[SUIT(c)]])+1
      && VALUE(c) <= lowest[COLOR(c)?0:1] + 2)
  {
    stack_animate(s, outcells[pile_for[SUIT(c)]]);
    return 1;
  }
  return 0;
}

static int
auto_move()
{
  int i, something_moved = 0;
  lowest[0] = lowest[1] = 99;
  pile_for[0] = pile_for[1] = pile_for[2] = pile_for[3] = -1;
  memset(top_card, 0, sizeof(top_card));
  for (i=0; i<4; i++)
  {
    int c, n = stack_count_cards(outcells[i]);
    if (n == 0) continue;
    c = stack_get_card(outcells[i], n-1);
    top_card[i] = c;
    pile_for[SUIT(c)] = i;
    if (lowest[COLOR(c)?1:0] > VALUE(c))
      lowest[COLOR(c)?1:0] = VALUE(c);
  }
  for (i=0; i<4; i++)
    if (pile_for[i] == -1)
      lowest[COLOR(MAKE_CARD(i, ACE, FACEUP))?1:0] = 0;
  for (i=0; i<4; i++)
    something_moved += auto_move_stack(freecells[i]);
  for (i=0; i<8; i++)
    something_moved += auto_move_stack(maincells[i]);
  return something_moved;
}

static void
check_for_end_of_game()
{
  int i, j;
  while (auto_move());
  if (stack_count_cards(outcells[0]) == 13
      && stack_count_cards(outcells[1]) == 13
      && stack_count_cards(outcells[2]) == 13
      && stack_count_cards(outcells[3]) == 13)
  {
    set_centered_pic(youwin);
  }

  /* look for losing configuration */
  for (i=0; i<4; i++)
    if (stack_count_cards(freecells[i]) == 0)
      return;
  for (i=0; i<8; i++)
    if (stack_count_cards(maincells[i]) == 0)
      return;
  for (i=0; i<8; i++)
  {
    int in = stack_count_cards(maincells[i]);
    int ic = stack_get_card(maincells[i], in-1);
    if (in > 1)
    {
      /* If this stack is already partially sorted, skip it */
      int inc = stack_get_card(maincells[i], in-2);
      if (COLOR(inc) != COLOR(ic) && VALUE(inc) == VALUE(ic)+1)
	continue;
    }
    for (j=0; j<8; j++)
    {
      /* Can we move a card to another stack? */
      int dc = stack_get_card(maincells[j], stack_count_cards(maincells[j])-1);
      if (COLOR(dc) != COLOR(ic) && VALUE(dc) == VALUE(ic)+1)
	return;
    }
    for (j=0; j<4; j++)
    {
      /* Can we move a card from a maincell to an outcell? */
      int dc = stack_get_card(outcells[j], stack_count_cards(outcells[j])-1);
      if (SUIT(dc) == SUIT(ic) && VALUE(dc) == VALUE(ic)-1)
	return;
    }
  }
  for (i=0; i<4; i++)
  {
    int in = stack_count_cards(freecells[i]);
    int ic = stack_get_card(freecells[i], in-1);
    for (j=0; j<8; j++)
    {
      /* Can we move a card from a freecell to a stack? */
      int dc = stack_get_card(maincells[j], stack_count_cards(maincells[j])-1);
      if (COLOR(dc) != COLOR(ic) && VALUE(dc) == VALUE(ic)+1)
	return;
    }
    for (j=0; j<4; j++)
    {
      /* Can we move a card from a freecell to an outcell? */
      int dc = stack_get_card(outcells[j], stack_count_cards(outcells[j])-1);
      if (SUIT(dc) == SUIT(ic) && VALUE(dc) == VALUE(ic)-1)
	return;
    }
  }
  set_centered_pic(youlose);
}

void
click(int x, int y, int b)
{
  int c, f;
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

  src_stack = 0;
  f = stack_find(x, y, &src_stack, &src_n);
  if (!f) return;

  if (b > 1)
  {
    stack_peek_card(src_stack, src_n, 1);
    return;
  }
  last_n = -1;

  for (c=0; c<4; c++)
    if (src_stack == outcells[c])
      return;

  if (f && stack_count_cards(src_stack) > 0)
    stack_begin_drag(src_stack, src_n, x, y);
  else
    src_stack = 0;
}

void
double_click(int x, int y, int b)
{
  int f, i;
  src_stack = 0;
  f = stack_find(x, y, &src_stack, &src_n);
  if (!f) return;

  if (b > 1) return;

  for (f=0; f<4; f++)
    if (stack_count_cards(freecells[f]) == 0)
    {
      for (i=0; i<8; i++)
	if (src_stack == maincells[i])
	{
	  int n = stack_count_cards(maincells[i]);
	  if (n > 0)
	  {
	    stack_flip_card(maincells[i], freecells[f]);
	    return;
	  }
	}
      return;
    }
}

void
drag(int x, int y, int b)
{
  if (b > 1) return;
  last_n = n_droppable(x, y);
  stack_continue_drag(last_n, x, y);
}

void
drop(int x, int y, int b)
{
  last_n = n_droppable(x, y); /* also sets dest_stack */

  if (b > 1)
  {
    if (src_stack)
      stack_peek_card(src_stack, src_n, 0);
    return;
  }

  stack_drop(dest_stack, last_n);
  check_for_end_of_game();
}
