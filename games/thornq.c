/* The Ace of Penguins - thornq.c
   Copyright (C) 1998, 2001 Martin Thornquist

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
#include <stdlib.h>
#include "imagelib.h"
#include "cards.h"

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define F CARD_FAN_DOWN

#define WIN_H H + M + 23*F
#define WIN_W 8*W + 9*M

extern int stack_fan_down;

static Picture *xlogo, *splash, *youwin, *youlose, *arrow, *no_arrow;
static Stack *deck, *outcells[4], *maincells[8];
static int hint_mode = 0;
static char values[] = " A23456789TJQK";
static char suits[] = "HDCS";

static int auto_move();
static void check_for_end_of_game();
static void set_arrows();

static int ax, adx, ay, ady;

static void
resize(int w, int h)
{
  int margin, offset, cw, ch, s, fd, fr, tfd, tfr;
  Picture *empty;

  stack_set_card_size (w/9, w/9*4/3);
  stack_get_card_size (&cw, &ch);

  empty = (Picture *)get_image("empty", cw, ch, 0);

  margin = (w - 8*cw) / 9;
  offset = (w - margin*9 - cw*8) / 2 + margin;

  for (s=0; s<4; s++)
    {
      stack_move(outcells[s], w - (4-s)*cw, 0);
      stack_set_empty_picture(outcells[s], empty);
    }
  for (s=0; s<8; s++)
    stack_move(maincells[s], offset + s*(cw+margin), ch + (offset<0?0:offset));

  stack_get_fans(&fd, &fr, &tfd, &tfr);

  ax = offset + cw/2;
  adx = margin + cw;
  ay = offset + ch*2 + 2;
  ady = fd;

  set_arrows();
}

static int supress_arrows = 0;
static int arrowsx[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static int arrowsy[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static void
set_arrow(int column, int on)
{
  int x = ax + adx*column;
  if (supress_arrows)
    return;
  if (arrowsy[column])
    put_picture(no_arrow, arrowsx[column]-no_arrow->w/2, arrowsy[column],
		0, 0, no_arrow->w, no_arrow->h);
  if (on)
  {
    arrowsx[column] = x;
    arrowsy[column] = ay+ady*(stack_count_cards(maincells[column])-1);
    put_picture(arrow, arrowsx[column]-arrow->w/2, arrowsy[column],
		0, 0, arrow->w, arrow->h);
  }
  else
    arrowsy[column] = 0;
}

static void
redraw_arrows()
{
  int i;
  for (i=0; i<8; i++)
  {
    if (arrowsy[i])
      put_picture(arrow, arrowsx[i]-arrow->w/2, arrowsy[i],
		  0, 0, arrow->w, arrow->h);
  }
}

static void
clear_arrows()
{
  int i;
  for (i=0; i<8; i++)
    set_arrow(i, 0);
}

static void
set_arrows()
{
  int i, j, k;
  for (i=0; i<8; i++)
  {
    Stack *s = maincells[i];
    int top_stack = stack_get_card(s, stack_count_cards(s)-1);
    set_arrow(i, 0);
    for (j=0; j<8; j++)
      if (j != i)
	for (k=stack_count_cards(maincells[j])-1; k>=0; k--)
	{
	  int match = stack_get_card(maincells[j], k);
	  if (SUIT(match) == SUIT(top_stack)
	      && VALUE(match) == VALUE(top_stack)-1
	      && ! FACEDOWNP(match))
	    set_arrow(i,1);
	}
  }
}

static void
start_again()
{
  int i, j;

  clear_arrows();

  for (i = 0; i < 4; i++)
    stack_flip_stack(outcells[i], deck, 0);

  for (i = 0; i < 8; i++)
    stack_flip_stack(maincells[i], deck, 0);

  stack_shuffle(deck);
  stack_shuffle(deck);
  stack_shuffle(deck);

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++)
      stack_move_card(deck, maincells[i], 0);
    for (j = 0; j < 3; j++)
      stack_flip_card(deck, maincells[i], 0);
  }

  for (i = 4; i < 8; i++)
    for (j = 0; j < 6; j++)
      stack_flip_card(deck, maincells[i], 0);

  stack_undo_reset();
  set_arrows();
}


static void
init()
{
  int s, v;
  Picture *empty;

  stack_load_standard_deck();
  empty = get_picture("empty");

  set_centered_pic(splash);

  for (s = 0; s < 4; s++) {
    outcells[s] = stack_create(4*W + 9*M + s*W, 0);
    stack_set_empty_picture(outcells[s], empty);
  }

  for (s = 0; s < 8; s++) {
    maincells[s] = stack_create(M+s*(M+W), H+M);
    stack_set_offset(maincells[s], STACK_OFFSET_DOWN);
  }

  deck = stack_create(10*W, 0);

  for (s = 0; s < 4; s++)
    for (v = ACE; v <= KING; v++)
      stack_add_card(deck, MAKE_CARD(s, v, FACEDOWN));

  start_again();
}


static void
redraw()
{
  int cw, ch;
  stack_get_card_size (&cw, &ch);
  if (xlogo->w < table_width - 8*cw && xlogo->h < ch)
      put_picture(xlogo, table_width/2-xlogo->w/2, ch/2-xlogo->h/2,
		  0, 0, xlogo->h, xlogo->w);
  stack_redraw();
  redraw_arrows();
}


extern char thornq_help[];

static void
key(int k, int x, int y)
{

  Picture *cp = get_centered_pic();

  switch (k) {
  case 3: case 27: case 'q':
    exit(0);

  case KEY_F(1): case 'h':
    set_centered_pic(0);
    help("thornq.html", thornq_help);
    return;

  case KEY_F(2): case 'r':
    set_centered_pic(0);
    start_again();
    while (auto_move());
    return;

  case 8: case 127: case KEY_DELETE:
    set_centered_pic(0);
    clear_arrows();
    stack_undo();
    set_arrows();
    return;
  }

  if (cp == youlose || cp == youwin)
    {
      set_centered_pic(0);
      start_again();
      while (auto_move());
      return;
    }

  switch (k) {
  case 'a':
    hint_mode = 1;
    check_for_end_of_game();
    hint_mode = 0;
    break;
  case 'A':
    hint_mode = 1-hint_mode;
    check_for_end_of_game();
    break;

  case ' ':
    double_click(x, y, 1);
    break;
  }
}


static char *
card_string(int c)
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
static int src_n = 0;

static Stack *dest_stack;
static int last_n = -1;

static int
n_droppable_s(Stack *dest_stack)
{
  int src_top, dest_top, src_count, dest_count, i, j;

  src_count = stack_count_cards(src_stack);
  dest_count = stack_count_cards(dest_stack);
  dest_top = stack_get_card(dest_stack, dest_count-1);
  src_top = stack_get_card(src_stack, src_count-1);

  if (dest_stack == src_stack)
    return src_n;

#if 0
  /* auto-move precludes the need for this */
  for (i = 0; i < 4; i++)
    if (dest_stack == outcells[i]) {
      if (dest_count == 0 && VALUE(src_top) == ACE)
	return src_count - 1;
      if (dest_count > 0 && VALUE(src_top) == VALUE(dest_top) + 1
	  && SUIT(src_top) == SUIT(dest_top))
	return src_count - 1;
      return src_count;
    }
#endif

  for (i = 0; i < 8; i++)
    if (dest_stack == maincells[i])
      {
	/* move entire stack to empty slot */
	if (0 == src_n && 0 == dest_count)
	  return 0;

	if (dest_count == 0)
	  {
	    /* look for kings */
	    for (j=src_n; j<src_count; j++)
	      {
		src_top = stack_get_card(src_stack, j);
		if (!FACEDOWNP(src_top) && VALUE(src_top) == KING)
		  return j;
	      }
	    for (j=src_n-1; j>=0; j--)
	      {
		src_top = stack_get_card(src_stack, j);
		if (!FACEDOWNP(src_top) && VALUE(src_top) == KING)
		  return j;
	      }
	    return 0;
	  }
	else
	  {
	    /* Look for moveable cards */
	    for (j=0; j<src_count; j++)
	      {
		src_top = stack_get_card(src_stack, j);
		if (!FACEDOWNP(src_top)
		    && SUIT(src_top) == SUIT(dest_top)
		    && VALUE(src_top) == VALUE(dest_top)-1)
		  return j;
	      }
	  }

	break;
      }

  return src_count;
}

static int
n_droppable(int x, int y)
{
  if (!src_stack)
    return 0;

  if (!stack_drag_find(x, y, &dest_stack)) {
    dest_stack = src_stack;

    return last_n != -1 ? last_n : src_n;
  }

  return n_droppable_s(dest_stack);
}


static int
auto_move()
{
  int i, j, c, f, m = 0;

  for (i = 0; i < 8; ++i) {
    if (stack_count_cards(maincells[i]) == 0)
      continue;
    c = stack_get_card(maincells[i], stack_count_cards(maincells[i])-1);

    if (FACEDOWNP(c))
      stack_flip_card(maincells[i], maincells[i], 1);
    /*continue;*//* use this to turn over cards yourself */

    if (ACE == VALUE(c)) {
      for (j = 0; stack_count_cards(outcells[j]); ++j)
	;

      stack_animate(maincells[i], outcells[j]);
      m = 1;
      continue;
    }

    for (j = 0; j < 4; ++j) {
      f = stack_get_card(outcells[j], stack_count_cards(outcells[j])-1);

      if (SUIT(f) == SUIT(c) &&
	  VALUE(f) == VALUE(c) - 1) {
	stack_animate(maincells[i], outcells[j]);
	m = 1;
	break;
      }
    }
  }

  return m;
}


static void
check_for_end_of_game()
{
  int i, j, k, available_moves=0;

  set_arrows();

  if (stack_count_cards(outcells[0]) == 13 &&
      stack_count_cards(outcells[1]) == 13 &&
      stack_count_cards(outcells[2]) == 13 &&
      stack_count_cards(outcells[3]) == 13)
  {
    set_centered_pic(youwin);
    return;
  }

  if (hint_mode)
    printf("Available moves:\n");
  for (i=0; i<8; i++)
  {
    Stack *d = maincells[i];
    int dc = stack_count_cards(d);
    int topcard = stack_get_card(d, dc-1);
    for (j=0; j<8; j++)
      if (j != i)
      {
	Stack *s = maincells[j];
	for (k=stack_count_cards(s)-1; k>=0; k--)
	{
	  int one = stack_get_card(s, k);
	  if (!FACEDOWNP(one))
	  {
	    if (dc == 0)
	    {
	      if (VALUE(one) == KING && k > 0)
	      {
		if (!hint_mode)
		  return;
		printf("  Move K%c from stack %d to stack %d\n",
		       suits[SUIT(one)], j+1, i+1);
		available_moves++;
	      }
	    }
	    else
	    {
	      if (SUIT(one) == SUIT(topcard)
		  && VALUE(one) == VALUE(topcard)-1)
	      {
		if (!hint_mode)
		  return;
		printf("  move %c%c from stack %d onto stack %d\n",
		       values[VALUE(one)], suits[SUIT(one)],
		       j+1, i+1);
		available_moves++;
	      }
	    }
	  }
	}
      }
  }
  if (!available_moves)
    set_centered_pic(youlose);
}

static void
click(int x, int y, int b)
{
  int c, f;

  Picture *cp = get_centered_pic();

  if (cp == youlose || cp == youwin)
    {
      set_centered_pic(0);
      start_again();
      while (auto_move());
      return;
    }

  if (cp == splash) {
    set_centered_pic(0);

    while (auto_move())
      ;

    return;
  }

  src_stack = 0;
  if (!(f = stack_find(x, y, &src_stack, &src_n)))
    return;

  if (FACEDOWNP(stack_get_card(src_stack, stack_count_cards(src_stack)-1))
      == FACEDOWN) {
    stack_flip_card(src_stack, src_stack, 0);
    return;
  }

  if (b > 1) {
    stack_peek_card(src_stack, src_n, 1);

    return;
  }

  last_n = -1;

  for (c=0; c<4; c++)
    if (src_stack == outcells[c])
      return;

  if (f && stack_count_cards(src_stack) > 0)
  {
    clear_arrows();
    stack_begin_drag(src_stack, src_n, x, y);
  }
  else
    src_stack = 0;
}

static int
try_moving_to(int i)
{
  int c = stack_get_card(src_stack, src_n);
  for (i=0; i<8; i++)
    if (maincells[i] != src_stack)
      {
	int j = stack_count_cards(maincells[i]) - 1;
	int f = stack_get_card(maincells[i], j);
	if (SUIT(f) == SUIT(c)
	    && VALUE(c) == VALUE(f) - 1
	    && ! FACEDOWNP(c)) {
	  clear_arrows();
	  stack_move_cards(src_stack, src_n, maincells[i]);
	  while (auto_move()) ;
	  set_arrows();
	  check_for_end_of_game();
	  return 1;
	}
      }
  return 0;
}

static int
try_moving_from(int i)
{
  int c = stack_get_card(src_stack, stack_count_cards(src_stack)-1);
  for (i=0; i<8; i++)
    if (maincells[i] != src_stack)
      {
	int j, n = stack_count_cards(maincells[i]);
	for (j=0; j<n; j++)
	  {
	    int f = stack_get_card(maincells[i], j);
	    if (SUIT(f) == SUIT(c)
		&& VALUE(f) == VALUE(c) - 1
		&& ! FACEDOWNP(f)) {
	      clear_arrows();
	      if (j == n-1)
		stack_animate(maincells[i], src_stack);
	      else
		stack_move_cards(maincells[i], j, src_stack);
	      while (auto_move()) ;
	      set_arrows();
	      check_for_end_of_game();
	      return 1;
	    }
	  }
      }
  return 0;
}

static void
double_click(int x, int y, int b)
{
  int c, f, i, j;

  src_stack = 0;

  if (!(stack_find(x, y, &src_stack, &src_n)))
    return;

  if (b > 1)
    return;

  /* verify that it's a main stack we double clicked on */
  for (i=0; i<8; i++)
    if (maincells[i] == src_stack)
      break;
  if (i == 8)
    return;

  if (stack_count_cards(src_stack) == 0)
    {
      /* find a king, and move it */
      for (i=0; i<8; i++)
	for (j=stack_count_cards(maincells[i])-1; j>0; j--)
	  {
	    c=stack_get_card(maincells[i], j);
	    if (!FACEDOWNP(c) && VALUE(c) == KING)
	      {
		clear_arrows();
		stack_move_cards(maincells[i], j, src_stack);
		while (auto_move()) ;
		set_arrows();
		check_for_end_of_game();
		return;
	      }
	  }
    }

  /* clicked on top card or below it */
  if (src_n >= stack_count_cards(src_stack) - 1 || src_n < 0)
    if (try_moving_from(i))
      return;

  /* clicked on underlying card */
  if (try_moving_to(i))
    return;

  /* this didn't make sense to me - DJ */
#if 0
  /* oh well, try top card anyway */
  try_moving_from(i);
#endif
}

static void
drag(int x, int y, int b)
{
  if (b > 1)
    return;

  last_n = n_droppable(x, y);

  stack_continue_drag(last_n, x, y);
}

static void
drop(int x, int y, int b)
{
  last_n = n_droppable(x, y); /* also sets dest_stack */

  if (b > 1) {
    if (src_stack)
      stack_peek_card(src_stack, src_n, 0);

    return;
  }

  clear_arrows();
  stack_drop(dest_stack, last_n);

  while (auto_move())
    ;

  set_arrows();
  check_for_end_of_game();
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
  { "double_click", (void *)double_click },
  { "drag", (void *)drag },
  { "drop", (void *)drop },
  { "init", (void *)init },
  { "key", (void *)key },
  { "redraw", (void *)redraw },
  { "resize", (void *)resize },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  register_imagelib(appimglib_imagelib);
  init_ace(argc, argv, fmap);

  xlogo = get_picture("xemboss");
  splash = get_picture("thornq");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");
  arrow = get_picture("thornq-arrow");
  no_arrow = get_picture("thornq-noarrow");

  if (table_width == 0 || table_height == 0)
    {
      table_width = WIN_W;
      table_height = WIN_H;
    }
  init_table(table_width, table_height);
  table_loop();

  return 0;
}
