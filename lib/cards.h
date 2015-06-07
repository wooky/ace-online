/* The Ace of Penguins - cards.h
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

#ifndef _cards_h_
#define _cards_h_

#define CARD_WIDTH	73
#define CARD_HEIGHT	97
#define CARD_MARGIN	6
#define CARD_FAN_DOWN	19
#define CARD_FAN_RIGHT	16
#define CARD_FAN_TBDOWN	6
#define CARD_FAN_TBRIGHT 6

#ifndef _IMAGELIB_H_
typedef struct image_list {
  int filler;
} image_list;
extern struct image_list appimglib_imagelib[];
#endif
int register_imagelib(struct image_list *);

extern void clear(int x, int y, int w, int h);
extern void flush();
extern void flushsync();
extern void beep();
extern void text(char *s, int x, int y); /* lower left corner */
extern int font_width, font_height;

extern void help(char *filename, char *text);

/* user program may define these as needed.  It is not neccessary to
   clip the pictures during redraw, as put_picture knows when you're
   inside an expose and will clip and optimize accordingly. */
static void init();
static void redraw();
static void resize(int width, int height);
static void key(int k, int x, int y);
static void click(int x, int y, int b);
static void double_click(int x, int y, int b);
static void drag(int x, int y, int b);
static void drop(int x, int y, int b);

#define KEY_F(x)	(0x100 + (x))
#define KEY_DELETE	0x200
#define KEY_UP		0x201
#define KEY_DOWN	0x202
#define KEY_LEFT	0x203
#define KEY_RIGHT	0x204
#define KEY_PGUP	0x205
#define KEY_PGDN	0x206
#define KEY_HOME	0x207

#ifndef STACK_DEF
typedef struct Stack {void *stack__p;} Stack;
#endif

#define MAKE_CARD(s, v, f) ((f) + (v)*4 + (s))
#define SUIT(c)		((c) & 3)
#define COLOR(c)	((c) & 2)
#define VALUE(c)	(((c)>>2) & 15)
#define FACEDOWNP(c)	((c) & FACEDOWN)
#define FACEUP		0x00
#define FACEDOWN	0x40

#define SUIT_HEARTS	0
#define SUIT_DIAMONDS	1
#define SUIT_CLUBS	2
#define SUIT_SPADES	3

#define ACE		1
#define JACK		11
#define QUEEN		12
#define KING		13

#define STACK_OFFSET_NONE	0
#define STACK_OFFSET_RIGHT	1
#define STACK_OFFSET_DOWN	2
#define STACK_OFFSET_TBRIGHT	3
#define STACK_OFFSET_TBDOWN	4

typedef struct {
  int card_width, card_height;
  int fan_down, fan_right, fan_tbdown, fan_tbright;
} StackSizes;

#ifndef _TABLE_H_
// for struct Picture
#error include table.h first
#endif

Stack *	stack_create(int x, int y);
void	stack_destroy(Stack *s);
void	stack_set_pictures(Picture **fronts, Picture *back);
void	stack_load_standard_deck();
void	stack_move(Stack *s, int x, int y);
void	stack_set_offset(Stack *s, int which_offset);
void	stack_set_card_size(int width, int height);
void	stack_get_card_size(int *width, int *height);
void	stack_get_fans(int *down, int *right, int *tbdown, int *tbright);

void	stack_set_empty_picture(Stack *s, Picture *p);
void	stack_redraw();
void	stack_peek_card(Stack *s, int n, int show);

int	stack_count_cards(Stack *s);
int	stack_get_card(Stack *s, int n);
void	stack_add_card(Stack *s, int c);
int	stack_take_card(Stack *s);
void	stack_change_card(Stack *s, int n, int c);
void	stack_shuffle(Stack *s);

int	stack_find(int x, int y, Stack **stack_ret, int *n_ret);
int	stack_drag_find(int x, int y, Stack **stack_ret);
int	stack_card_posn(Stack *s, int n, int *x, int *y);

void	stack_move_cards(Stack *src, Stack *dest, int num, int flag);
void	stack_move_card(Stack *src, Stack *dest, int flag);
void	stack_flip_cards(Stack *src, Stack *dest, int num, int flag);
void	stack_flip_card(Stack *src, Stack *dest, int flag);
void	stack_flip_stack(Stack *src, Stack *dest, int flag);
void	stack_flip_move_stack(Stack *src, Stack *dest, int flag);
void	stack_animate(Stack *src, Stack *dest, int flag);
void	stack_undo();
void	stack_undo_reset();

void	stack_begin_drag(Stack *s, int n, int x, int y);
void	stack_continue_drag(int n, int x, int y);
void	stack_drop(Stack *onto, int n, int flag);

#endif
