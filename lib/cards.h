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

/* Change these if you replace the card images */
#ifndef PDA
#define CARD_WIDTH	73
#define CARD_HEIGHT	97
#define CARD_MARGIN	6
#define CARD_FAN_DOWN	19
#define CARD_FAN_RIGHT	16
#define CARD_FAN_TBDOWN	6
#define CARD_FAN_TBRIGHT 6
#endif
#if 0
#define CARD_WIDTH	65
#define CARD_HEIGHT	86
#define CARD_MARGIN	4
#define CARD_FAN_DOWN	13
#define CARD_FAN_RIGHT	12
#define CARD_FAN_TBDOWN	4
#define CARD_FAN_TBRIGHT 4
#endif
#if 0
#define CARD_WIDTH	110
#define CARD_HEIGHT	145
#define CARD_MARGIN	9
#define CARD_FAN_DOWN	30
#define CARD_FAN_RIGHT	24
#define CARD_FAN_TBDOWN	9
#define CARD_FAN_TBRIGHT 9
#endif
#ifdef PDA
#define CARD_WIDTH	17
#define CARD_HEIGHT	31
#define CARD_MARGIN	1
#define CARD_FAN_DOWN	11
#define CARD_FAN_RIGHT	5
#define CARD_FAN_TBDOWN	3
#define CARD_FAN_TBRIGHT 3
#endif

#define TABLE_MONO	0
#define TABLE_GRAY	1
#define TABLE_COLOR	2
extern int table_type;

void init_table(int argc, char **argv, int table_width, int table_height);
void table_loop();

/* Really, see image in imagelib.h */
typedef struct Picture {
  int w, h;
} Picture;

Picture *get_picture(char *name);

/* The x,y,w,h are relative to (0,0) on the picture.  The dx,dy
   indicate where (0,0) on the picture would go */
void put_picture(Picture *picture, int dx, int dy,
		 int x, int y, int w, int h);
void put_picture_inverted(Picture *picture, int dx, int dy,
			  int x, int y, int w, int h);

/* This is drawn over everything else, centered.  Pass zero to remove */
void set_centered_pic(Picture *picture);
Picture *get_centered_pic();

extern void clip(int x, int y, int w, int h);
extern void clip_more(int x, int y, int w, int h);
extern void unclip();
extern void clear(int x, int y, int w, int h);
extern void invalidate(int x, int y, int w, int h);
extern void invalidate_nc(int x, int y, int w, int h);
extern void invalidate_exposure(int ox, int oy, int ow, int oh,
				int nx, int ny, int nw, int nh);
extern void flush();
extern void flushsync();
extern void beep();
extern void text(char *s, int x, int y); /* lower left corner */
extern int font_width, font_height;

extern void help(char *filename, char *text);

/* user program may define these as needed.  It is not neccessary to
   clip the pictures during redraw, as put_picture knows when you're
   inside an expose and will clip and optimize accordingly. */
extern void init();
extern void redraw();
extern void key(int k, int x, int y);
extern void click(int x, int y, int b);
extern void double_click(int x, int y, int b);
extern void drag(int x, int y, int b);
extern void drop(int x, int y, int b);

#define KEY_F(x)	(0x100 + (x))
#define KEY_DELETE	0x200
#define KEY_UP		0x201
#define KEY_DOWN	0x202
#define KEY_LEFT	0x203
#define KEY_RIGHT	0x204
#define KEY_PGUP	0x205
#define KEY_PGDN	0x206
#define KEY_HOME	0x207

void snap_to_grid(int *x, int *y,
		  int step_x, int step_y,
		  int origin_x, int origin_y,
		  int max_distance);

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

Stack *	stack_create(int x, int y);
void	stack_destroy(Stack *s);
void	stack_set_pictures(Picture **fronts, Picture *back);
void	stack_load_standard_deck();
void	stack_move(Stack *s, int x, int y);
void	stack_set_offset(Stack *s, int which_offset);

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

int	stack_move_cards(Stack *src, int n, Stack *dest); /* n is card # */
void	stack_flip_card(Stack *src, Stack *dest);
void	stack_flip_stack(Stack *src, Stack *dest);
void	stack_animate(Stack *src, Stack *dest);
void	stack_undo();
void	stack_undo_reset();

void	stack_begin_drag(Stack *s, int n, int x, int y);
void	stack_continue_drag(int n, int x, int y);
void	stack_drop(Stack *onto, int n);

#endif
