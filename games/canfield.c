/* The Ace of Penguins - canfield.c
   Copyright (C) 1999, 2001 Martin Thornquist

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
#include "cards.h"

#define W CARD_WIDTH
#define H CARD_HEIGHT
#define M CARD_MARGIN
#define F CARD_FAN_DOWN

#define WIN_H H + M + 23*F
#define WIN_W 6*W + 6*M

/* number of cards in stock -- higher number gives more difficult game */
#define CARDS_IN_STOCK 10

Picture *xlogo, *splash, *youwin, *youlose;
Stack *hand, *talon, *stock, *tableau[4], *foundation[4];

int base_rank;
char base_mesg[14];

extern int table_width, table_height;

static int auto_move();

void debug(char *s)
{
#ifdef DEBUG
	printf("%s", s);
#endif
}

int main(int argc, char **argv)
{
	xlogo = get_picture("xemboss");
	splash = get_picture("canfield");
	youwin = get_picture("youwin");
	youlose = get_picture("youlose");

	init_table(argc, argv, WIN_W, WIN_H);
	table_loop();

	return 0;
}


static void start_again()
{
	int i, j;

	for (i = 0; i < 4; i++)
		stack_flip_stack(tableau[i], hand);

	for (i = 0; i < 4; i++)
		stack_flip_stack(foundation[i], hand);

	stack_flip_stack(stock, hand);

	stack_shuffle(hand);
	stack_shuffle(hand);
	stack_shuffle(hand);

	for (i = 0; i < CARDS_IN_STOCK; i++)
		stack_flip_card(hand, stock);

	for (i = 0; i < 4; i++)
		stack_flip_card(hand, tableau[i]);

	stack_flip_card(hand, foundation[0]);

	text("              ", 90, font_height+5);

	base_rank = VALUE(stack_get_card(foundation[0], 0));
	sprintf(base_mesg, "Base rank: %d", base_rank);
	printf("%s\n", base_mesg);
	redraw();

	stack_undo_reset();
}


void init()
{
	int s, v;
	Picture *empty;

	stack_load_standard_deck();
	empty = get_picture("empty");

	set_centered_pic(splash);

	for (s = 0; s < 4; s++) {
		foundation[s] = stack_create(2*W + 5*M + s*W, M);
		stack_set_empty_picture(foundation[s], empty);
	}

	for (s = 0; s < 4; s++) {
		tableau[s] = stack_create(2*W+2*M+s*(M+W), H+2*M);
		stack_set_offset(tableau[s], STACK_OFFSET_DOWN);
	}

	stock = stack_create(M, M);
	hand = stack_create(M, 2*H+3*M);
	talon = stack_create(M, H+2*M);
	stack_set_empty_picture(talon, empty);

	for (s = 0; s < 4; s++)
		for (v = ACE; v <= KING; v++)
			stack_add_card(hand, MAKE_CARD(s, v, FACEDOWN));

	start_again();
}


void redraw()
{
	/* draw base_rank on background */

	put_picture(xlogo, M+W/2-xlogo->w/2, M+H/2-xlogo->h/2,
		    0, 0, xlogo->h, xlogo->w);
	put_picture(xlogo, M+W/2-xlogo->w/2, 3*M+2*H+H/2-xlogo->h/2,
		    0, 0, xlogo->h, xlogo->w);
	text(base_mesg, 90, font_height+5);

	stack_redraw();
}


extern char unnamed_help[];

void key(int k, int x, int y)
{
	switch (k) {
	case 3: case 27: case 'q':
		exit(0);

	case KEY_F(1): case 'h':
		set_centered_pic(0);
/*		help("canfield.html", canfield_help);*/
		return;

	case KEY_F(2): case 'r':
		set_centered_pic(0);
		start_again();

		while (auto_move())
			;

		break;

	case 8: case 127: case KEY_DELETE:
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
static int src_n = 0;

static Stack *dest_stack;
static int last_n = -1;

static int n_droppable_s(Stack *dest_stack)
{
	int src_top, dest_top, src_count, dest_count, i;

	src_count = stack_count_cards(src_stack);
	dest_count = stack_count_cards(dest_stack);
	dest_top = stack_get_card(dest_stack, dest_count-1);
	src_top = stack_get_card(src_stack, src_count-1);

	if (dest_stack == src_stack)
		return src_n;

	for (i = 0; i < 4; i++)
		if (dest_stack == foundation[i]) {
			if (dest_count == 0 && VALUE(src_top) == base_rank)
				return src_count - 1;
			if (dest_count > 0 && SUIT(src_top) == SUIT(dest_top)
			    && ((VALUE(src_top) == VALUE(dest_top) + 1) ||
				(VALUE(src_top) == KING && VALUE(dest_top) == ACE)))
				return src_count - 1;
			return src_count;
		}

	for (i = 0; i < 4; i++)
		if (dest_stack == tableau[i]) {
			if (0 == src_n && 0 == dest_count)
				return 0;

			src_top = stack_get_card(src_stack, src_n);

			if (FACEDOWN == FACEDOWNP(src_top))
				return src_count;

			if (0 == dest_count) {
				if (stock == src_stack)
					return src_n;

				return src_count;
			}

			if (COLOR(src_top) != COLOR(dest_top)
			    && (VALUE(src_top) == VALUE(dest_top)-1 ||
				(VALUE(src_top) == KING && VALUE(dest_top) == ACE)))
				return src_n;

			break;
		}

	return src_count;
}

static int n_droppable(int x, int y)
{
	if (!src_stack)
		return 0;

	if (!stack_drag_find(x, y, &dest_stack)) {
		dest_stack = src_stack;

		return last_n != -1 ? last_n : src_n;
	}

	return n_droppable_s(dest_stack);
}

void hand_to_talon() {
	stack_flip_card(hand, talon);
	stack_flip_card(hand, talon);
	stack_flip_card(hand, talon);
}

static int auto_move()
{
	int i, j, c, f, m = 0;

	for (i = 0; i < 4; ++i)
		if (!stack_count_cards(tableau[i]))
			if (stack_count_cards(stock))
				stack_animate(stock, tableau[i]);
			else if (stack_count_cards(talon))
				stack_animate(talon, tableau[i]);				

	if (!stack_count_cards(talon)) {
		if (stack_count_cards(hand)) {
			hand_to_talon();
			m = 1;
		}
	} else {
		c = stack_get_card(talon, stack_count_cards(talon) - 1);

		if (base_rank == (VALUE(c))) {
			for (j = 0; stack_count_cards(foundation[j]); ++j)
				;

			stack_animate(talon, foundation[j]);
			m = 1;
		}

		for (i = 0; i < 4; ++i) {
			f = stack_get_card(foundation[i],
					   stack_count_cards(foundation[i])-1);

			if (SUIT(f) == SUIT(c) &&
			    (VALUE(f) == VALUE(c) - 1 ||
			     (VALUE(f) == KING && VALUE(c) == ACE))) {
				stack_animate(talon, foundation[i]);
				m = 1;
				break;
			}
		}
	}
		

	if (stack_count_cards(stock)) {
		c = stack_get_card(stock, stack_count_cards(stock)-1);

		if (base_rank == VALUE(c)) {
			for (j = 0; stack_count_cards(foundation[j]); ++j)
				;

			stack_animate(stock, foundation[j]);
			m = 1;
		}

		for (i = 0; i < 4; ++i) {
			f = stack_get_card(foundation[i], stack_count_cards(foundation[i])-1);

			if (SUIT(f) == SUIT(c) &&
			    (VALUE(f) == VALUE(c) - 1 ||
			     (VALUE(f) == KING && VALUE(c) == ACE))) {
				stack_animate(stock, foundation[i]);
				m = 1;
				break;
			}
		}
	}

	for (i = 0; i < 4; ++i) {
		c = stack_get_card(tableau[i], stack_count_cards(tableau[i])-1);

		if (base_rank == VALUE(c)) {
			for (j = 0; stack_count_cards(foundation[j]); ++j)
				;

			stack_animate(tableau[i], foundation[j]);
			m = 1;
			continue;
		}

		for (j = 0; j < 4; ++j) {
			f = stack_get_card(foundation[j], stack_count_cards(foundation[j])-1);

			if (SUIT(f) == SUIT(c) &&
			    (VALUE(f) == VALUE(c) - 1 ||
			     (VALUE(f) == KING && VALUE(c) == ACE))) {
				stack_animate(tableau[i], foundation[j]);
				m = 1;
				break;
			}
		}
	}

	return m;
}


static void check_for_end_of_game()
{
	int i, j, k, c, f;

	/* presumes that auto_move() is called */

	if (stack_count_cards(foundation[0]) == 13 &&
	    stack_count_cards(foundation[1]) == 13 &&
	    stack_count_cards(foundation[2]) == 13 &&
	    stack_count_cards(foundation[3]) == 13) {
		set_centered_pic(youwin);
		return;
	}

/*	for (i = 0; i < 8; ++i) {
		j = stack_count_cards(tableau[i]);

		while (j-- > 0) {
			f = stack_get_card(tableau[i], j);

			printf("VALUE(f) = %d\n", VALUE(f));

			if (FACEDOWN == FACEDOWNP(f))
				break;
			printf("hei\n");

			for (k = 0; k < 8; ++k) {
				if (k == i)
					continue;

				c = stack_get_card(tableau[k],
						   stack_count_cards(tableau[k]-1))-1;
				printf("VALUE(c) = %d\n", VALUE(c));
				if (SUIT(c) == SUIT(j) && VALUE(c) == VALUE(j) - 1)
					return;
			}
		}
	}

	set_centered_pic(youlose);*/
}


void click(int x, int y, int b)
{
	int c, f;

	Picture *cp = get_centered_pic();

	if ((cp == youlose || cp == youwin)
	    && (x > table_width/2-cp->w/2
		&& x < table_width/2+cp->w/2
		&& y > table_height/2-cp->h/2
		&& y < table_height/2+cp->h/2)) {
		set_centered_pic(0);
		start_again();

		while (auto_move())
			;

		return;
	}

	if (cp == splash) {
		set_centered_pic(0);

/*		while (auto_move())
			;*/

		return;
	}

	src_stack = 0;
	if (!(f = stack_find(x, y, &src_stack, &src_n)))
		return;

	if (b > 1) {
		stack_peek_card(src_stack, src_n, 1);

		return;
	}

	last_n = -1;

	for (c = 0; c < 4; c++)
		if (src_stack == foundation[c])
			return;

	if (src_stack == hand) {
		if (0 == stack_count_cards(hand)) {
			stack_flip_stack(talon, hand);

			return;
		}

		hand_to_talon();
/*		stack_flip_card(hand, talon);
		stack_flip_card(hand, talon);
		stack_flip_card(hand, talon);*/

		return;
	}

	if (f && stack_count_cards(src_stack) > 0)
		stack_begin_drag(src_stack, src_n, x, y);
	else
		src_stack = 0;
}

void double_click(int x, int y, int b)
{
	int c, f, i;

	src_stack = 0;

	if (!(stack_find(x, y, &src_stack, &src_n)))
		return;

	if (src_stack == hand) {
		click(x, y, b);
		return;
	}

	if (b > 1)
		return;

	c = stack_get_card(src_stack, stack_count_cards(src_stack)-1);

	if (VALUE(c) == base_rank) {
		for (i = 0; i < 4; ++i)
			if (stack_count_cards(foundation[i]) == 0)
				break;

		stack_animate(src_stack, foundation[i]);
		return;
	}	

	for (i = 0; i < 4; ++i) {
		f = stack_get_card(foundation[i], stack_count_cards(foundation[i])-1);

		if (SUIT(f) == SUIT(c) && VALUE(f) == VALUE(c) - 1) {
			stack_animate(src_stack, foundation[i]);
			return;
		}
	}

	while (auto_move())
		;

}

void drag(int x, int y, int b)
{
	if (b > 1)
		return;

	last_n = n_droppable(x, y);

	stack_continue_drag(last_n, x, y);
}

void drop(int x, int y, int b)
{
	last_n = n_droppable(x, y); /* also sets dest_stack */

	if (b > 1) {
		if (src_stack)
			stack_peek_card(src_stack, src_n, 0);

		return;
	}

	stack_drop(dest_stack, last_n);

	while (auto_move())
		;

	check_for_end_of_game();
}
