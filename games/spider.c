/* The Ace of Penguins - freecell.c
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

#include "imagelib.h" 
#include "cards.h"
void   get_image_set_display_type (int type);

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define F CARD_FAN_DOWN

static Picture *splash, *youwin, *youlose, *arrow, *no_arrow;

static Stack *deck;
static Stack *outcells[8];
static Stack *maincells[10];

static int suit_mask = 0; // 0 = one suit, 2 = two suit, 3 = four suit 

static void auto_move();
static void check_for_end_of_game();
static void set_arrows();
static void clear_arrows();
static int ax, adx, ay, ady;

static void
start_again()
{
  int i, d, s, v;

  clear_arrows();

  for (i=0; i<8; i++)
    stack_flip_stack(outcells[i], deck);
  for (i=0; i<10; i++)
    stack_flip_stack(maincells[i], deck);

  while(stack_take_card(deck) != -1);
  for (d=0; d<2; d++)
    for (s=0; s<4; s++)
      for (v=ACE; v<=KING; v++)
	stack_add_card(deck, MAKE_CARD(s & suit_mask, v, FACEDOWN));

  stack_shuffle(deck);
  stack_shuffle(deck);
  stack_shuffle(deck);
  for (i=0; i<40; i++)
    stack_move_cards(deck, stack_count_cards(deck)-1, maincells[i%10]);
  for (i=0; i<4; i++)
    stack_move_cards(deck, stack_count_cards(deck)-1, maincells[i*3]);
  for (i=0; i<10; i++)
    stack_flip_card(deck, maincells[i]);

  stack_undo_reset();

  set_arrows();
}

static void
init()
{
  int s;
  Picture *empty;

  stack_load_standard_deck();
  empty = get_picture("empty");

  set_centered_pic(splash);

  for (s=0; s<8; s++)  {
    outcells[s] = stack_create((2+s)*(W+M), M);
    stack_set_empty_picture(outcells[s], empty);
  }

  for (s=0; s<10; s++)  {
    maincells[s] = stack_create(M+s*(M+W), H+M);
    stack_set_offset(maincells[s], STACK_OFFSET_DOWN);
  }

  deck = stack_create(M, M);

  start_again();
}

static void
resize(int w, int h)
{
  int margin, offset, cw, ch, s, fd, fr, tfd, tfr;

  Picture *empty;

  stack_set_card_size (w/11, w/11*4/3);
  stack_get_card_size (&cw, &ch);

  empty = (Picture *)get_image("empty", cw, ch, 0);

  margin = (w - 10*cw) / 11;
  offset = (w - margin*11 - cw*10) / 2 + margin;

  for (s=0; s<8; s++) {
    stack_move(outcells[s], (s+2)*(cw+margin)+offset, margin);
    stack_set_empty_picture(outcells[s], empty);
  }
  for (s=0; s<10; s++)
    stack_move(maincells[s], offset + s*(cw+margin), ch + (offset<0?0:offset));
  stack_move(deck, margin+offset, margin);

  stack_get_fans(&fd, &fr, &tfd, &tfr);

  ax = offset + cw/2;
  adx = margin + cw;
  ay = offset + ch*2 + 2;
  ady = fd;

  set_arrows();
}


static int no_arrows = 0;
static int supress_arrows = 0;
static int arrowsx[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int arrowsy[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void
set_arrow(int column, int on)
{
  int x = ax + adx*column;
  if (supress_arrows || no_arrows) return;
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
  if(no_arrows) return;
  for (i=0; i<10; i++)
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
  for (i=0; i<10; i++)
    set_arrow(i, 0);
}

static void
set_arrows()
{
  int i, j, k, src_next;
  for (i=0; i<10; i++)
  {
    Stack *dest = maincells[i]; 
    int dest_count = stack_count_cards(dest);
    int dest_top = stack_get_card(dest, dest_count-1);
    int n = 0;
    int add = 0;

    set_arrow(i, 0);
    if (dest_count == 0) { set_arrow(i,1); add=1; };

    for (j=0; j<10 && add==0; j++) {
      Stack *src = maincells[j];
      int src_count = stack_count_cards(src);
      int src_top;
      int src_next;
      n = src_count;
      while (n--) {
	if ((i == j) || (src_count == 0)) n = 0;
	else {
	  src_top = stack_get_card(src, n);
	  src_next = stack_get_card(src, n-1);
	  if (VALUE(src_top) == VALUE(dest_top)-1) {
	    int better = 0;
	    //possible let's see if better
	    if(n == 0) better = 1;
	    if (!better) {
	      if (VALUE(src_top) != VALUE(src_next)-1) better = 1;
	    }
	    if (!better) {
	      if (SUIT(src_top) != SUIT(src_next)) better = 1;
	    }
	    if (!better) {
	      if (FACEDOWNP(src_next)) better = 1;
	    }
	    if (better) add = 1;
	  }
	  if (n == 0 || VALUE(src_top) != VALUE(src_next)-1 ||
	      SUIT(src_top) != SUIT(src_next) ||
	      FACEDOWNP(src_next) == FACEDOWN ) n = 0;
	}
      }
    }
    if (add) set_arrow(i,1);
  }
}


static void
redraw()
{
  stack_redraw();
  redraw_arrows();
}

extern char spider_help[];

static void
key(int k, int x, int y)
{
  Picture *p = get_centered_pic();

  set_centered_pic(0);
  if (p == splash) return;

  switch(k) {
  case 3:  case 27: case 'q': case 'Q':
    exit(0);
    break;
  case KEY_F(1): case 'h': case 'H':
    help("bj.html", spider_help);
    break;
  case KEY_F(2):  case 'r':
    start_again();
    break;
  case 8:  case 127:  case KEY_DELETE:
    set_centered_pic(0);
    stack_undo();
    set_arrows();
    break;
  case '1': 
    suit_mask = 0;
    start_again();
    break;
  case '2': 
    suit_mask = 2;
    start_again();
    break;
  case '4':
    suit_mask = 3;
    start_again();
    break;
  case 'a':
    if (no_arrows) {
      no_arrows = 0;
      set_arrows();
    } else {
      clear_arrows();
      no_arrows = 1;
    }
    break;
  }
}

static Stack *src_stack = 0;
static int src_n = 0;

static Stack *dest_stack;
static int last_n=-1;

static int
n_droppable_s(Stack *dest_stack)
{
  int src_top, dest_top, src_count, dest_count, i, src_next;

  src_count = stack_count_cards(src_stack);
  dest_count = stack_count_cards(dest_stack);
  dest_top = stack_get_card(dest_stack, dest_count-1);

  for (i=0; i<10; i++) {
    if (dest_stack == maincells[i]) {
      int n = src_count;

      while (n--)
      {
	src_top = stack_get_card(src_stack, n);
	if ((VALUE(src_top) == VALUE(dest_top)-1)
	    || (n == src_n && dest_count == 0)) return n;

	src_next = stack_get_card(src_stack, n-1);
	if (n == 0 || VALUE(src_top) != VALUE(src_next)-1 ||
	    SUIT(src_top) != SUIT(src_next) ||
	    FACEDOWNP(src_next) == FACEDOWN ) {
	  if ((dest_count == 0) || (dest_stack == src_stack))return n;
	  else return src_count;
	}
      }
      return src_count;
    }
  }
  return src_count;
}

static int
n_droppable(int x, int y)
{
  if (!src_stack)
    return -1;
  if (!stack_drag_find(x, y, &dest_stack))
  {
    dest_stack = src_stack;
    return last_n != -1 ? last_n : src_n;
  }
  return n_droppable_s(dest_stack);
}


static void
auto_move()
{
  int i, c, s, v, f, n;

  for (i=0; i<10; i++) {
    n = stack_count_cards(maincells[i]);
    c = stack_get_card(maincells[i],n-1);
    if (FACEDOWNP(c)) stack_flip_card(maincells[i], maincells[i]);
    if ((VALUE(c) == ACE) && n >= 13) {
      s = SUIT(c);
      v = 2;
      f = FACEUP;
      n -= 2;
      while((n>=0)&&(stack_get_card(maincells[i],n) == MAKE_CARD(s,v,f))) {
	n--; v++;
      }
      if (v == 14) {
	//Full suit to move, first find empty outcell
	c =0;
	while(stack_count_cards(outcells[c++]) != 0);
	c--;
	for(v=ACE; v<=KING; v++) stack_animate(maincells[i],outcells[c]);
	i--; // to flip any cards in this column
      }
    }
  }
}

static void
check_for_end_of_game()
{
  auto_move();
  if (stack_count_cards(outcells[7]) == 13) set_centered_pic(youwin);
  if (stack_count_cards(deck) == 0) {
    int i;
    int end = 1;
    set_arrows();
    for (i=0; i<10; i++) {
      if (arrowsy[i] != 0) end = 0;
    }
    //    if (end) set_centered_pic(youlose);
  }
}

static void
click(int x, int y, int b)
{
  int c, f;
  Picture *cp = get_centered_pic();

  if (cp == splash){
    set_centered_pic(0);
    return;
  }

  if (cp == youwin) {
    set_centered_pic(0);
    start_again();
    return;
  }

  if (cp == youlose) {
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

  for (c=0; c<8; c++)
    if (src_stack == outcells[c])
      return;

  if (src_stack == deck) {
    int empty_stack=0;
    for (c=0; c<10; c++) {
      if (stack_count_cards(maincells[c]) == 0) empty_stack = 1;
    }
    clear_arrows();
    if (empty_stack == 0) {
      for (c=0; c<10; c++) {
	stack_flip_card(deck, maincells[c]);
      }
    }
  }

  if (stack_count_cards(src_stack) > 0)
    stack_begin_drag(src_stack, src_n, x, y);
  else
    src_stack = 0;
}

static void
drag(int x, int y, int b)
{
  if (b > 1) return;
  last_n = n_droppable(x, y);
  stack_continue_drag(last_n, x, y);
}

static void
drop(int x, int y, int b)
{
  last_n = n_droppable(x, y); /* also sets dest_stack */

  if (b > 1)
  {
    if (src_stack)
      stack_peek_card(src_stack, src_n, 0);
    return;
  }
  clear_arrows();
  stack_drop(dest_stack, last_n);
  check_for_end_of_game();
  set_arrows();
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
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
  splash = get_picture("spider");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");
  arrow = get_picture("golf-arrow");
  no_arrow = get_picture("golf-noarrow");
  if (table_width == 0 || table_height == 0)
    {
      table_width = 10*(W+M) + M;
      table_height = 600;
    }
  init_table(table_width, table_height);
  table_loop();
  return 0;
}
