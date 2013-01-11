/* The Ace of Penguins - solitaire.c
   Copyright (C) 1998, 2001, 2002 DJ Delorie

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
#include <math.h>
#include "imagelib.h"
#include "cards.h"

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define F CARD_FAN_DOWN

static Picture *xlogo, *splash, *youwin, *youlose;

static Stack *deck, *hole;
static Stack *outcells[4];
static Stack *maincells[7];

static int auto_move();
static void update_status_text(int redraw);
static void check_for_end_of_game();

static int no_auto = 0;        /* boolean */
static int use_auto_moves = 1; /* boolean */
static int drag_active = 0;    /* boolean - set if card is being dragged */
static int flip_3s = 0;        /* boolean - zero means flip 1 at a time */
                               /*           non-zero means flip 3 at a time */
static int vegas = 0;          /* boolean - in vegas style, you pay $52 to play */
                               /*           and you win $5 for each card that you */
                               /*           place in the outdeck.  The deck is only */
                               /*           turned 3 times.  flip_3s is implied to */
                               /*           be true. */
static int winnings = 0;
static int vegas_deck_count = 0;
static int display_vegas_xlogo = 0; /* set to 1 to display xlogo in pile area at */
                                    /* end of vegas game. */
static OptionDesc new_options[] = {
  { "-noauto", OPTION_BOOLEAN, &no_auto }, 
  { "-flip3s", OPTION_BOOLEAN, &flip_3s },
  { "-vegas", OPTION_BOOLEAN, &vegas },
  { 0, 0, 0 }
};

static OptionDesc *app_options = new_options;

static void
start_again()
{
  int i, p, pc;
  stack_flip_stack(hole, deck, 0);
  for (i=0; i<4; i++)
    stack_flip_stack(outcells[i], deck, 0);
  for (i=0; i<7; i++)
    stack_flip_stack(maincells[i], deck, 0);

  stack_shuffle(deck);
  stack_shuffle(deck);
  stack_shuffle(deck);
  
  for (p=0; p<7; p++)
    for (pc=0; pc<=p; pc++)
      stack_move_card(deck, maincells[p], 0);
  for (p=0; p<7; p++)
    stack_flip_card(maincells[p], maincells[p], 0);

  stack_undo_reset();

  winnings -= 52;
  update_status_text(1);
  vegas_deck_count = 0;
  display_vegas_xlogo = 0;
}

static int xlogo_x;
static int xlogo_y;

static int vegas_xlogo_x;
static int vegas_xlogo_y;

static void
init()
{
  int s, v;
  Picture *empty;

  stack_load_standard_deck();
  empty = get_picture("empty");
  xlogo = get_picture("xemboss");
  splash = get_picture("solitaire");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");

  xlogo_x = 3*M+2*W+W/2-xlogo->w/2;
  xlogo_y = M+H/2-xlogo->h/2;

  vegas_xlogo_x = M+W/2-xlogo->w/2;
  vegas_xlogo_y = M+H/2-xlogo->h/2;

  set_centered_pic(splash);

  for (s=0; s<4; s++)
  {
    outcells[s] = stack_create(3*W+4*M+s*(W+M), M);
    stack_set_empty_picture(outcells[s], empty);
  }

  for (s=0; s<7; s++)
  {
    maincells[s] = stack_create(M+s*(M+W), H+2*M);
    stack_set_offset(maincells[s], STACK_OFFSET_DOWN);
  }

  deck = stack_create(M, M);
  hole = stack_create(2*M+W, M);

  for (s=0; s<4; s++)
    for (v=1; v<=13; v++)
      stack_add_card(deck, MAKE_CARD(s, v, FACEDOWN));
  start_again();
}

static void
resize(int w, int h)
{
  int margin, offset, cw, ch, s;
  Picture *empty;

  stack_set_card_size (w/8, w/8*4/3);
  stack_get_card_size (&cw, &ch);

  empty = (Picture *)get_image("empty", cw, ch, 0);

  margin = (w - 7*cw) / 8;
  offset = (w - margin*8 - cw*7) / 2 + margin;

  for (s=0; s<4; s++)
    {
      stack_move(outcells[s], w - (4-s)*(cw+margin), margin);
      stack_set_empty_picture(outcells[s], empty);
    }
  for (s=0; s<7; s++)
    stack_move(maincells[s], offset + s*(cw+margin), ch + (offset<0?0:offset) + margin);

  stack_move(deck, offset, margin);
  stack_move(hole, offset+margin+cw, margin);

  if (xlogo->w > cw)
    xlogo_x = xlogo_y = vegas_xlogo_x = vegas_xlogo_y = 0;
  else
    {
      xlogo_x = 3 * margin + 2 * cw + cw/2 - xlogo->w/2;
      xlogo_y = margin + ch/2 - xlogo->h/2;
      vegas_xlogo_x = margin + cw/2 - xlogo->w/2;
      vegas_xlogo_y = margin + ch/2 - xlogo->h/2;
    }
}

static void
redraw()
{
  if (xlogo_x || xlogo_y)
    put_picture(xlogo, xlogo_x, xlogo_y, 0, 0, xlogo->h, xlogo->w);

  if (display_vegas_xlogo && (vegas_xlogo_x || vegas_xlogo_y))
    put_picture(xlogo, vegas_xlogo_x, vegas_xlogo_y, 0, 0, xlogo->h, xlogo->w);
  update_status_text(0);

  stack_redraw();
}

static void check_for_end_of_game();

extern char solitaire_help[];

static void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q' || k == 'Q')
    exit(0);
  if (k == KEY_F(1) || k == 'h' || k == 'H')
  {
    set_centered_pic(0);
    help("solitaire.html", solitaire_help);
  }
  if (k == ' ')
    {
      double_click(x, y, 1);
      check_for_end_of_game();
    }

  if (k == 'w')
    set_centered_pic(youwin);
  if (k == 'l')
    set_centered_pic(youlose);

  if (k == KEY_F(2) || k == 'r' || k == 'R')
  {
    set_centered_pic(0);
    start_again();

    if (use_auto_moves)
      while (auto_move());

  }
  if ((k == 8 || k == 127 || k == KEY_DELETE))
  {
    stack_undo();
  }
}

static Stack *src_stack = 0;
static int src_n = 0, src_maxn = 0;

static Stack *dest_stack;
static int last_n=-1;

static int
n_droppable_s(Stack *dest_stack)
{
  int src_top, dest_top, src_count, dest_count, i;
  int src_value, src_suit, dest_value, dest_suit;
  src_count = stack_count_cards(src_stack);
  dest_count = stack_count_cards(dest_stack);
  dest_top = stack_get_card(dest_stack, dest_count-1);
  src_top = stack_get_card(src_stack, src_count-1);

  if (dest_stack == src_stack)
    return src_n;

  if (dest_stack == deck || dest_stack == hole)
    return src_count;

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

  for (i=0; i<7; i++)
    if (dest_stack == maincells[i])
    {
      int n = src_count-1;

      if (src_stack != hole && src_n == 0 && dest_count == 0)
	return 0;

      if (dest_count && FACEDOWNP(dest_top))
	return src_count;

      while (1)
      {
	int src_next;
	src_top = stack_get_card(src_stack, n);
	if ((COLOR(src_top) != COLOR(dest_top)
	     && VALUE(src_top) == VALUE(dest_top)-1)
	    || VALUE(src_top)==13 && dest_count == 0)
	  return n;

	src_next = stack_get_card(src_stack, n-1);
	if (n == 0 || COLOR(src_top) == COLOR(src_next)
	    || VALUE(src_top) != VALUE(src_next)-1
	    || FACEDOWNP(src_next)
	    || src_stack == hole)
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
    stack_flip_card(s, s, 1);
    c = stack_get_card(s, n-1);
    return 1;
  }
  if (VALUE(c) == ACE)
  {
    for (i=0; i<4; i++)
      if (stack_count_cards(outcells[i]) == 0)
      {
	stack_animate(s, outcells[i]);

        winnings += 5;
        update_status_text(1);

	return 1;
      }
    return 0; /* can't happen? */
  }
  if (pile_for[SUIT(c)] == -1)
    return 0;
  if (VALUE(c) == VALUE(top_card[pile_for[SUIT(c)]])+1
      && VALUE(c) <= lowest[COLOR(c)?0:1] + 2
      && VALUE(c) <= lowest[COLOR(c)?1:0] + 3)
  {
    stack_animate(s, outcells[pile_for[SUIT(c)]]);

    winnings += 5;
    update_status_text(1);

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
  if (stack_count_cards(hole) == 0 && stack_count_cards(deck) > 0)
  {
    stack_flip_card(deck, hole, 1);

    if (flip_3s)
    {
      for (i=0; i<2; i++)
        if (stack_count_cards(deck))
          stack_flip_card(deck, hole, 1);
    }

    something_moved += 1;
  }
  something_moved += auto_move_stack(hole);
  for (i=0; i<7; i++)
    something_moved += auto_move_stack(maincells[i]);
  return something_moved;
}

static void
check_for_end_of_game()
{
  if (use_auto_moves)
    while (auto_move());

  if (stack_count_cards(outcells[0]) == 13
      && stack_count_cards(outcells[1]) == 13
      && stack_count_cards(outcells[2]) == 13
      && stack_count_cards(outcells[3]) == 13)
  {
    set_centered_pic(youwin);
  }
}

static void
click(int x, int y, int b)
{
  int c, f;

  Picture *cp = get_centered_pic();
  src_stack = 0;

  // abort drag on click of other mousebutton
  if (drag_active) {
    stack_drop(dest_stack, last_n);
    drag_active = 0;
    return;
  };
  
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
  f = stack_find(x, y, &src_stack, &src_n);
  if (!f)
    return;

  if (b > 1)
  {
    if (src_stack && src_n>=0 && src_n <= stack_count_cards(src_stack)-1)
      stack_peek_card(src_stack, src_n, 1);
    return;
  }

  last_n = -1;

  if (src_stack == deck)
  {
    if (stack_count_cards(deck) == 0)
    {
      if (vegas)
      {
        vegas_deck_count++;
        if (vegas_deck_count >= 3)
	{
          start_again();
	  return;
	}
      }
      stack_flip_stack(hole, deck, 0);
    }
    else
    {
      stack_flip_card(deck, hole, 0);
      if (flip_3s)
        for (c=0; c<2; c++)
          if (stack_count_cards(deck) != 0)
	    stack_flip_card(deck, hole, 1);
      if (vegas && vegas_deck_count == 2 && stack_count_cards(deck) == 0)
      {
        display_vegas_xlogo = 1;
        invalidate (vegas_xlogo_x, vegas_xlogo_y,
                    xlogo->w, xlogo->h);
      }
    }
    return;
  }

  if (vegas)
    for (c=0; c<4; c++)
      if (src_stack == outcells[c])
      {
        winnings -= 5;
	update_status_text(1);
      }

  if (src_n>=0 && stack_count_cards(src_stack) > 0)
  {
    c = stack_get_card(src_stack, src_n);
    if (FACEDOWNP(c))
      stack_flip_card(src_stack, src_stack, 0);

    stack_begin_drag(src_stack, src_n, x, y);
  }
}

static void
double_click(int x, int y, int b)
{
  int c, f, n, sc;
  Stack *dest_stack=0;
  int dest_n;
  src_stack = 0;
  /*printf("double click\n");*/

  f = stack_find(x, y, &src_stack, &src_n);
  if (!f)
    return;

  if (b > 1) return;

  if (src_stack == deck)
  {
    click(x, y, b);
    return;
  }

  sc = stack_count_cards(src_stack);

  if (VALUE(stack_get_card(src_stack, sc-1)) > ACE)
    for (c=0; c<7; c++)
    {
      if (src_stack == maincells[c]) continue;
      n = n_droppable_s(maincells[c]);
      if (stack_count_cards(maincells[c]) == 0
	  && (VALUE(stack_get_card(src_stack, n)) != KING
	      || FACEDOWNP(stack_get_card(src_stack, n))
	      || (src_stack != hole && n == 0)))
	continue;
      /*printf("- maincell %d = %d\n", c, n);*/
      if (stack_count_cards(maincells[c]) == 0)
      {
	if (n < sc)
	{
	  dest_stack = maincells[c];
	  dest_n = n;
	  /*break;*/
	}
	continue;
      }
      stack_card_posn(maincells[c], 0, &x, &y);
      if (n < sc)
      {
	if (dest_stack)
	  if (dest_n < n || (dest_n == n && stack_count_cards(dest_stack)<stack_count_cards(maincells[c])))
	    break;
	dest_stack = maincells[c];
	dest_n = n;
      }
    }

  if (dest_stack)
  {
    /*printf("- move 0x%08x %d 0x%08x\n", src_stack, dest_n, dest_stack);*/
    stack_move_cards(src_stack, dest_n, dest_stack);
    check_for_end_of_game();
    return;
  }

  /*printf("- checking stacks, sc=%d\n", sc);*/
  for (c=0; c<4; c++)
  {
    if (src_stack == outcells[c]) return;
    stack_card_posn(outcells[c], 0, &x, &y);
    n = n_droppable_s(outcells[c]);
    /*printf("- outcell %d = %d\n", c, n);*/
    if (n < sc)
    {
      dest_stack = outcells[c];
      dest_n = n;
      break; /* we don't care which outcell it is */
    }
  }

  if (dest_stack)
  {
    /*printf("- move 0x%08x %d 0x%08x\n", src_stack, dest_n, dest_stack);*/
    stack_move_cards(src_stack, dest_n, dest_stack);
    check_for_end_of_game();
    return;
  }

  check_for_end_of_game();
}

static void
drag(int x, int y, int b)
{
  drag_active = 1;
  if (b > 1) return;
  drag_active = 1;
  last_n = n_droppable(x, y);
  stack_continue_drag(last_n, x, y);
}

static void
drop(int x, int y, int b)
{
  int i;

  last_n = n_droppable(x, y); /* also sets dest_stack */

  if (b > 1)
  {
    if (src_stack && src_n>=0 && src_n <= stack_count_cards(src_stack)-1)
      stack_peek_card(src_stack, src_n, 0);
    return;
  }

  if (vegas)
    for (i=0; i<4; i++)
      if (dest_stack == outcells[i])
      {
        winnings += (stack_count_cards(src_stack) - last_n) * 5;
	update_status_text(1);
      }

  drag_active = 0;

  stack_drop(dest_stack, last_n);

  drag_active = 0;

  check_for_end_of_game();
}

static void
update_status_text(int redraw)
{
  char buffer[10] = "";

  if (vegas)
  {
    sprintf (buffer, "$%d", winnings);
    text (buffer, M, table_height - M);
    if (redraw)
      invalidate (M, table_height - M - font_height, 
                  10 * font_width, font_height);
  }
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

  use_auto_moves = !no_auto;
  if (vegas)
    flip_3s = 1;

  if (table_width == 0 || table_height == 0)
    {
      table_width = W*7+M*8;
      table_height = H+3*M+19*F+font_height;
    }
  init_table(table_width, table_height);
  table_loop();
}
