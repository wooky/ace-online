/* The Ace of Penguins - stack.c
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

#include <stdio.h>
#include <sys/time.h>

typedef struct Stack {
  struct Stack *prev, *next;
  int x, y;
  int w, h;
  int num_cards;
  int max_cards;
  int *cards;
  int dx, dy;
  struct Picture *empty_picture;
} Stack;

#define STACK_DEF
#include "cards.h"

static Picture **fronts=0;
static Picture *back=0;
static Picture *nodrop=0;

#define INIT_NUM	10
#define STEP_NUM	10

static Stack *stacks=0;

static Stack *dragging_os = 0;
static Stack *dragging_s = 0;
static int dragging_n = 0;
static int dragging_dx, dragging_dy;

typedef struct Undo {
  Stack *src;
  short count;
  short facedown;
  Stack *dest;
} Undo;

static Undo *undo=0;
static int max_undo=0, num_undo=0;
static int doing_undo = 0;

static void stack_note_undo(Stack *src, int n, Stack *dest);

Stack *
stack_create(int x, int y)
{
  Stack *s = (Stack *)malloc(sizeof(Stack));
  if (s == 0)
    return 0;
  memset(s, 0, sizeof(Stack));
  s->max_cards = INIT_NUM;
  s->cards = (int *)malloc(s->max_cards * sizeof(int));
  if (s->cards == 0)
  {
    free(s);
    return 0;
  }

  s->x = x;
  s->y = y;

  s->next = stacks;
  stacks = s;
  if (s->next)
    s->next->prev = s;

  return s;
}

void
stack_destroy(Stack *s)
{
  if (s->next)
    s->next->prev = s->prev;
  if (s->prev)
    s->prev->next = s->next;
  else
    stacks = s->next;
  free(s->cards);
  free(s);
}

static void
stack_show_change(Stack *s, int a, int b)
{
  int w, h, num_cards;
  num_cards = s->num_cards;
  if (s == dragging_os && dragging_n < num_cards)
    num_cards = dragging_n;
  if (a > b)
    w = a, a = b, b = w;
  if (s->dx == 0 && s->dy == 0 && num_cards>0)
    invalidate_nc(s->x, s->y, CARD_WIDTH, CARD_HEIGHT);
  else
    invalidate(s->x+a*s->dx, s->y+a*s->dy,
	       (b-a)*s->dx+CARD_WIDTH, (b-a)*s->dy+CARD_HEIGHT);
}

static void
stack_expand(Stack *s, int n)
{
  if (s->max_cards <= n+1)
  {
    s->max_cards = n + STEP_NUM;
    s->cards = (int *)realloc(s->cards, s->max_cards * sizeof(int));
  }
}

void
stack_set_pictures(Picture **f, Picture *b)
{
  fronts = f;
  back = b;
  nodrop = get_picture("no-drop.gif");
}


void
stack_load_standard_deck()
{
  static char *suits = "hdcs";
  static char *values = " a234567890jqk";
  int s, v, t;
  char name[30];
  Picture **cards;
  Picture *back;

  cards = (Picture **)malloc(56 * sizeof(Picture *));
  for (s=0; s<4; s++)
    for (v=1; v<=13; v++)
    {
      sprintf(name, "card/%c%c.gif", values[v], suits[s]);
      cards[v*4+s] = get_picture(name);
    }
  back = get_picture("back.gif");
  nodrop = get_picture("no-drop.gif");
  stack_set_pictures(cards, back);
}

void
stack_move(Stack *s, int x, int y)
{
  int ox = x;
  int oy = y;
  s->x = x;
  s->y = y;
  invalidate_exposure(ox, oy, s->w, s->h, x, y, s->w, s->h);
  invalidate(x, y, s->w, s->h);
}

static void
stack_recalculate_size(Stack *s)
{
  if (s->num_cards > 0)
  {
    s->w = (s->num_cards-1) * s->dx + CARD_WIDTH;
    s->h = (s->num_cards-1) * s->dy + CARD_HEIGHT;
  }
  else
  {
    s->w = CARD_WIDTH;
    s->h = CARD_HEIGHT;
  }
}

void
stack_set_offset(Stack *s, int which_offset)
{
  int ow = s->w;
  int oh = s->h;
  switch (which_offset)
  {
  case STACK_OFFSET_RIGHT:
    s->dx = CARD_FAN_RIGHT;
    s->dy = 0;
    break;
  case STACK_OFFSET_DOWN:
    s->dx = 0;
    s->dy = CARD_FAN_DOWN;
    break;
  case STACK_OFFSET_TBRIGHT:
    s->dx = CARD_FAN_TBRIGHT;
    s->dy = 0;
    break;
  case STACK_OFFSET_TBDOWN:
    s->dx = 0;
    s->dy = CARD_FAN_TBDOWN;
    break;
  default:
    s->dx = 0;
    s->dy = 0;
    break;
  }
  stack_recalculate_size(s);
  invalidate_exposure(s->x, s->y, ow, oh, s->x, s->y, s->w, s->h);
  invalidate(s->x, s->y, s->w, s->h);
}


int
stack_count_cards(Stack *s)
{
  return s->num_cards;
}

int
stack_get_card(Stack *s, int n)
{
  if (n >= 0 && n < s->num_cards)
    return s->cards[n];
  return -1;
}

void
stack_add_card(Stack *s, int c)
{
  stack_expand(s, s->num_cards+1);
  put_picture(FACEDOWNP(c) ? back : fronts[c],
	      s->x+s->dx*s->num_cards, s->y+s->dy*s->num_cards,
	      0, 0, CARD_WIDTH, CARD_HEIGHT);
  s->cards[s->num_cards] = c;
  s->num_cards++;
  stack_recalculate_size(s);
}

int
stack_take_card(Stack *s)
{
  int rv = -1;
  if (s->num_cards > 0)
  {
    rv = s->cards[--s->num_cards];
    stack_show_change(s, s->num_cards, s->num_cards-1);
  }
  return rv;
}

void
stack_change_card(Stack *s, int n, int c)
{
  if (n < 0 || n>= s->num_cards)
    return;
  put_picture(FACEDOWNP(c) ? back : fronts[c],
	      s->x+s->dx*n, s->y+s->dy*n,
	      0, 0, CARD_WIDTH, CARD_HEIGHT);
  s->cards[n] = c;
}

void
stack_shuffle(Stack *s)
{
  int n, c, t;
  int randomized = 0;
  if (! randomized)
  {
    srand(time(0));
    randomized = 1;
  }
  for (n=0; n<s->num_cards; n++)
  {
    c = (rand() % (s->num_cards-n)) + n;
    t = s->cards[c];
    s->cards[c] = s->cards[n];
    s->cards[n] = t;
  }
}


void
stack_set_empty_picture(Stack *s, Picture *p)
{
  s->empty_picture = p;
  if (s->num_cards == 0)
    put_picture(p, s->x, s->y, 0, 0, CARD_WIDTH, CARD_HEIGHT);
}

#define P(n) (FACEDOWNP(s->cards[n]) ? back : fronts[s->cards[n] & 63])

static void
stack_redraw_stack(Stack *s)
{
  int n;
  int num_cards = s->num_cards;
  if (s == dragging_os && dragging_n < num_cards)
    num_cards = dragging_n;

  if (num_cards == 0)
  {
    if (s->empty_picture)
      put_picture(s->empty_picture,
		  s->x, s->y, 0, 0, CARD_WIDTH, CARD_HEIGHT);
  }
  else
  {
    if (s->dx)
      for (n=0; n<num_cards-1; n++)
	put_picture(P(n), s->x+s->dx*n, s->y,
		    0, 0, s->dx, CARD_HEIGHT);
    if (s->dy)
      for (n=0; n<num_cards-1; n++)
	put_picture(P(n), s->x, s->y+s->dy*n,
		    0, 0, CARD_WIDTH, s->dy);

    n = num_cards-1;
    put_picture(P(n), s->x+s->dx*n, s->y+s->dy*n,
		0, 0, CARD_WIDTH, CARD_HEIGHT);
  }
}

int nodrop_x, nodrop_y, nodrop_showing=0;

void
stack_show_nodrop(int x, int y)
{
  int oldx = nodrop_x;
  int oldy = nodrop_y;
  nodrop_x = x-dragging_dx+CARD_WIDTH/2-nodrop->w/2;
  nodrop_y = y-dragging_dy+CARD_HEIGHT/2-nodrop->h/2;
  if (nodrop_showing)
  {
    nodrop_showing = 0; /* avoid loops! */
    invalidate_exposure(oldx, oldy, nodrop->w, nodrop->h,
			nodrop_x, nodrop_y, nodrop->w, nodrop->h);
  }
  nodrop_showing = 1;
  put_picture(nodrop, nodrop_x, nodrop_y, 0, 0, nodrop->w, nodrop->h);
}

void
stack_hide_nodrop()
{
  if (nodrop_showing)
  {
    nodrop_showing = 0;
    invalidate(nodrop_x, nodrop_y, nodrop->w, nodrop->h);
  }
}

void
stack_redraw()
{
  Stack *s;

  for (s=stacks; s; s=s->next)
    stack_redraw_stack(s);
  if (nodrop_showing)
    put_picture(nodrop, nodrop_x, nodrop_y, 0, 0, nodrop->w, nodrop->h);
}

void
stack_peek_card(Stack *s, int n, int show)
{
  int x, y;
  if (n < 0 || n > s->num_cards)
    return;

  x = s->x + s->dx * n;
  y = s->y + s->dy * n;
  if (show)
    put_picture(P(n), x, y, 0, 0, CARD_WIDTH, CARD_HEIGHT);
  else
    invalidate(x, y, CARD_WIDTH, CARD_HEIGHT);
}

int
stack_find(int x, int y, Stack **stack_ret, int *n_ret)
{
  Stack *s;
  int n;

  for (s=stacks; s; s=s->next)
  {
    if (s == dragging_s) continue;
    for (n=s->num_cards-1; n>=0; n--)
    {
      int cx = s->x + s->dx * n;
      int cy = s->y + s->dy * n;
      if (cx <= x && x < cx+CARD_WIDTH
	  && cy <= y && y < cy+CARD_HEIGHT)
      {
	*stack_ret = s;
	*n_ret = n;
	return 1;
      }
    }
  }

  for (s=stacks; s; s=s->next)
  {
    if (s == dragging_s) continue;
    if (s->x <= x && x < s->x+CARD_WIDTH
	&& s->y <= y && y < s->y+CARD_HEIGHT)
    {
      *stack_ret = s;
      *n_ret = -1;
      return 1;
    }
  }

  for (s=stacks; s; s=s->next)
  {
    if (s == dragging_s) continue;
    if (s->dx > 0
	&& s->y <= y && y < s->y+CARD_HEIGHT
	&& s->x < x)
    {
	*stack_ret = s;
	*n_ret = -1;
	return 1;
    }
    if (s->dy > 0
	&& s->y <= y
	&& s->x < x && x < s->x+CARD_WIDTH)
    {
	*stack_ret = s;
	*n_ret = -1;
	return 1;
    }
  }

  return 0;
}

int
stack_drag_find(int x, int y, Stack **stack_ret)
{
  int n_ret;
  return stack_find(x-dragging_dx+CARD_WIDTH/2, y-dragging_dy+CARD_HEIGHT/2,
		    stack_ret, &n_ret);
}

int
stack_card_posn(Stack *s, int n, int *x, int *y)
{
  if (s->num_cards == 0)
  {
    *x = s->x;
    *y = s->y;
    return 1;
  }
  if (n < 0 || n >= s->num_cards)
    return 0;
  *x = s->x + s->dx * n;
  *y = s->y + s->dy * n;
  return 1;
}


int
stack_move_cards(Stack *src, int n, Stack *dest)
{
  int count = src->num_cards - n;
  if (n < 0 || n >= src->num_cards)
    return 0;

  stack_note_undo(src, n, dest);

  stack_expand(dest, dest->num_cards+count);

  memcpy(dest->cards+dest->num_cards, src->cards+n, count*sizeof(int));

  src->num_cards -= count;
  stack_recalculate_size(src);
  stack_show_change(src, src->num_cards, src->num_cards+count);

  dest->num_cards += count;
  stack_recalculate_size(dest);
  stack_show_change(dest, dest->num_cards, dest->num_cards-count);
}

void
stack_flip_card(Stack *src, Stack *dest)
{
  stack_note_undo(src, src->num_cards-1, dest);
  doing_undo = 1;
  if (src != dest)
  {
    src->cards[src->num_cards-1] &= ~FACEDOWN;
    stack_move_cards(src, src->num_cards-1, dest);
  }
  else
    stack_change_card(dest, dest->num_cards-1,
		      dest->cards[dest->num_cards-1] & ~FACEDOWN);
  doing_undo = 0;
}  

void
stack_flip_stack(Stack *src, Stack *dest)
{
  int old_s = src->num_cards;
  int old_d = dest->num_cards;

  stack_note_undo(src, 0, dest);
  stack_expand(dest, dest->num_cards + src->num_cards);
  while (src->num_cards > 0)
    dest->cards[dest->num_cards++] = src->cards[--src->num_cards] | FACEDOWN;
  stack_show_change(dest, old_d-1, dest->num_cards-1);
  stack_show_change(src, old_s-1, 0);
}

static void
stack_note_undo(Stack *src, int n, Stack *dest)
{
  if (doing_undo) return;
  if (num_undo >= max_undo)
  {
    max_undo += 50;
    if (undo)
      undo = (Undo *)realloc(undo, max_undo * sizeof(Undo));
    else
      undo = (Undo *)malloc(max_undo * sizeof(Undo));
  }
  undo[num_undo].src = src;
  undo[num_undo].dest = dest;
  undo[num_undo].count = stack_count_cards(src) - n + 1;
  undo[num_undo].facedown = src->cards[src->num_cards-1] & FACEDOWN;
  num_undo++;
}

void
stack_undo_reset()
{
  num_undo = 0;
}

void
stack_undo()
{
  Stack *src;
  if (num_undo == 0)
    return;
  doing_undo = 1;
  num_undo--;
  if (undo[num_undo].dest != undo[num_undo].src)
  {
    stack_move_cards(undo[num_undo].dest,
		     stack_count_cards(undo[num_undo].dest)-undo[num_undo].count+1,
		     undo[num_undo].src);
  }
  if (undo[num_undo].facedown)
  {
    src = undo[num_undo].src;
    stack_change_card(src, src->num_cards-1, src->cards[src->num_cards-1] | FACEDOWN);
  }
  doing_undo = 0;
}

void
stack_begin_drag(Stack *s, int n, int x, int y)
{
  if (dragging_s == 0)
  {
    /* build a fake "stack" that we can move about, redraw, etc. */
    dragging_s = (Stack *)malloc(sizeof(Stack));
    memset(dragging_s, 0, sizeof(Stack));
    if (stacks)
    {
      Stack *tmp;
      for (tmp=stacks; tmp->next; tmp=tmp->next);
      tmp->next = dragging_s;
      dragging_s->prev = tmp;
    }
    else
      stacks = dragging_s;
  }
  dragging_s->dx = s->dx;
  dragging_s->dy = s->dy;

  dragging_os = s;

  if (n < 0)
    n = 0;

  dragging_s->cards = s->cards+n;
  dragging_s->num_cards = s->num_cards-n;
  dragging_s->x = s->x + s->dx * n;
  dragging_s->y = s->y + s->dy * n;
  dragging_dx = x - dragging_s->x;
  dragging_dy = y - dragging_s->y;
  /* we don't actually move anything, so we don't have to invalidate
     anything */

  dragging_n = s->num_cards; /* so that the first drag exposes the right area */
}

void
stack_continue_drag(int n, int x, int y)
{
  int old_x, old_y, old_w, old_h;
  int new_x, new_y, new_w, new_h;
  int old_n = dragging_n;

  if (!dragging_os) return;

  if (n < 0) n = 0;
  if (n > dragging_os->num_cards) n = dragging_os->num_cards;

  old_x = dragging_s->x;
  old_y = dragging_s->y;
  old_w = dragging_s->w;
  old_h = dragging_s->h;

  /* recalculate new location for invalidations */
  dragging_s->x = x - dragging_dx;
  dragging_s->y = y - dragging_dy;
  dragging_s->cards = dragging_os->cards + n;
  dragging_s->num_cards = dragging_os->num_cards - n;
  dragging_s->w = (dragging_s->num_cards-1)*dragging_s->dx + CARD_WIDTH;
  dragging_s->h = (dragging_s->num_cards-1)*dragging_s->dy + CARD_HEIGHT;

  if (n != old_n)
  {
    dragging_n = n;
    stack_show_change(dragging_os, n, old_n);
  }

  if (dragging_s->num_cards > 0)
  {
    stack_hide_nodrop();
    invalidate_exposure(old_x, old_y, old_w, old_h,
			dragging_s->x, dragging_s->y,
			dragging_s->w, dragging_s->h);
    stack_redraw_stack(dragging_s);
  }
  else
  {
    stack_show_nodrop(x, y);
    if (old_n != dragging_os->num_cards)
      invalidate(old_x, old_y, old_w, old_h);
  }
}

void
stack_drop(Stack *onto, int n)
{
  Stack *os = dragging_os;
  if (!dragging_os) return;

  if (n < 0) n = 0;
  if (n > dragging_os->num_cards) n = dragging_os->num_cards;

  stack_hide_nodrop();
  dragging_os = 0;
  dragging_s->num_cards = 0;
  invalidate(dragging_s->x, dragging_s->y, dragging_s->w, dragging_s->h);

  if (onto == os || n == os->num_cards)
  {
    stack_show_change(os, dragging_n, os->num_cards);
  }
  else
  {
    stack_move_cards(os, n, onto);
    stack_show_change(os, dragging_n, os->num_cards);
  }
}

static inline int
iabs(int a)
{
  return a<0 ? -a : a;
}

static double
utime()
{
  struct timeval tp;
  gettimeofday(&tp, 0);
  return tp.tv_sec + (tp.tv_usec/1000000.0);
}

void
stack_animate(Stack *s, Stack *d)
{
  int x1, y1, x2, y2, x, y, ox, oy;
  double dx, ddx;
  int sn, dn, frames=0;
  double start, end;
  static double fps_factor = 5.0;
  static double fps_factors[5] = { 15, 15, 15, 15, 15 };
  static int factor_slot=0;

  sn = stack_count_cards(s);
  dn = stack_count_cards(d);
  stack_card_posn(s, sn-1, &x1, &y1);
  stack_card_posn(d, dn-1, &x2, &y2);
  stack_begin_drag(s, sn-1, x1, y1);
  flush();
  ddx = fps_factor/(iabs(x1-x2) + iabs(y1-y2));
  if (ddx > 0.25) ddx = 0.25;
  ox = y1;
  oy = x1;
  start = utime();
  for (dx=0; dx<1; dx+=ddx)
  {
    x = (int)(x1+dx*(x2-x1));
    y = (int)(y1+dx*(y2-y1));
    if (x != ox || y != oy)
    {
      stack_continue_drag(sn-1, x, y);
      frames++;
    }
    ox = x;
    oy = y;
    flush();
  }
  stack_drop(d, sn-1);
  flush();

  /* Figure out the new speed; we want the visual rate to be
     independent of the CPU/video performance */
  end = utime();
  if (end > start)
  {
#if 0
    printf("%d frames in %g seconds = %g fps\n",
	   frames, end-start, frames / (end-start));
#endif
    /* calculate new factor */
    fps_factors[factor_slot] = 4000 / (frames / (end-start));
    factor_slot = (factor_slot+1)%5;
    /* average the last five factors */
    fps_factor = (fps_factors[0]+fps_factors[1]+fps_factors[2]
		  +fps_factors[3]+fps_factors[4])/5;
#if 0
    printf("fps factor = %g\n", fps_factor);
#endif
  }
	 
}
