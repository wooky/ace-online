/* The Ace of Penguins - mastermind.c
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
#include <time.h>
#include <math.h>
#include "cards.h"

static Picture *splash, *youwin, *youlose;

static Picture *colors[6], *bighole, *cover;
static Picture *black, *white, *smallhole;

/* The "palette" */
static int px, py, pdy;
/* The guesses */
static int gx, gy, gdx, gdy;
/* The ranks */
static int rx, ry, rdx, rdy;
/* The solution */
static int sx, sy, sdx;

/* Sizes of the images */
#define BSZ 16
#define SSZ 10
#define WGAP 30

#define MOVES 10

static Picture *guesses[MOVES][4];
static Picture *ranks[MOVES][4];
static Picture *solution[4];

extern int table_width, table_height;

static int active_row, solution_shown;

static int
redraw_row(int row)
{
  int x;
  for (x=0; x<4; x++)
  {
    if (guesses[row][x])
      put_picture(guesses[row][x], gx+x*gdx, gy+row*gdy, 0, 0, BSZ, BSZ);
    if (ranks[row][x])
      put_picture(ranks[row][x], rx+x*rdx, ry+row*rdy, 0, 0, SSZ, SSZ);
  }
}

static void
set_active_row(int row)
{
  int x;
  active_row = row;
  for (x=0; x<4; x++)
    guesses[active_row][x] = bighole;
  redraw_row(active_row);
  py = (gy+active_row*gdy)-(BSZ+1)*5/2;
  if (py < CARD_MARGIN)
    py = CARD_MARGIN;
  if (py > table_height-CARD_MARGIN-6*BSZ-5)
    py = table_height-CARD_MARGIN-6*BSZ-5;
  invalidate(0, 0, px+BSZ, table_height);
}

static void
start_again()
{
  int x, y;
  for (x=0; x<4; x++)
  {
    for (y=0; y<MOVES; y++)
    {
      guesses[y][x] = 0;
      ranks[y][x] = 0;
    }
    solution[x] = colors[rand()%6];
  }
  set_active_row(0);
  solution_shown = 0;
}

static void
init()
{
  start_again();
  set_centered_pic(splash);
}

static void
show_solution()
{
  solution_shown = 1;
  invalidate(0, 0, table_width, table_height);
}

static void
redraw()
{
  int x, y;
  for (y=0; y<MOVES; y++)
    redraw_row(y);
  for (x=0; x<4; x++)
    if (solution_shown)
      put_picture(solution[x], sx+x*sdx, sy, 0, 0, BSZ, BSZ);
    else
      put_picture(cover, sx+x*sdx, sy, 0, 0, BSZ, BSZ);
  if (! solution_shown)
    for (y=0; y<6; y++)
      put_picture(colors[y], px, py+y*pdy, 0, 0, BSZ, BSZ);
}

static void
show_rank(int row)
{
  int x, y;
  int b=0, w=0, ri=0;
  int gs[4], ss[4];
  for (x=0; x<4; x++)
    gs[x] = ss[x] = 0;
  for (x=0; x<4; x++)
    if (guesses[active_row][x] == solution[x])
    {
      ranks[active_row][ri++] = black;
      b++;
      gs[x] = ss[x] = 1;
    }
  for (x=0; x<4; x++)
    if (! gs[x])
      for (y=0; y<4; y++)
	if (! ss[y])
	  if (guesses[active_row][x] == solution[y])
	  {
	    ranks[active_row][ri++] = white;
	    w++;
	    gs[x] = 1;
	    ss[y] = 1;
	    break;
	  }
  while (ri<4)
    ranks[active_row][ri++] = smallhole;
  if (b == 4)
    show_solution();
  redraw_row(active_row);
}

static void
check_row()
{
  int x;
  for (x=0; x<4; x++)
    if (!guesses[active_row][x] || guesses[active_row][x] == bighole)
      return;
  show_rank(active_row);
  if (!solution_shown)
  {
    if (active_row == (MOVES-1))
    {
      show_solution();
      set_centered_pic(youlose);
    }
    else
      set_active_row(active_row+1);
  }
}

static void
key_color(int c)
{
  int x;
  for (x=0; x<4; x++)
    if (guesses[active_row][x] == bighole)
    {
      guesses[active_row][x] = colors[c];
      redraw_row(active_row);
      return;
    }
}

extern char mastermind_help[];

static void
key(int k, int x, int y)
{
  int i;
  Picture *p = get_centered_pic();
  set_centered_pic(0);
  if (p == splash)
    return;
  if (p == youwin || p == youlose || solution_shown)
  {
    start_again();
    invalidate(0, 0, table_width, table_height);
    return;
  }
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  if (k == KEY_F(2))
  {
    start_again();
    invalidate(0, 0, table_width, table_height);
    return;
  }
  if (k == KEY_F(1) || k == 'h')
  {
    set_centered_pic(0);
    help("mastermind.html", mastermind_help);
    return;
  }
  if (k == 'r') key_color(0);
  if (k == 'o') key_color(1);
  if (k == 'y') key_color(2);
  if (k == 'g') key_color(3);
  if (k == 'b') key_color(4);
  if (k == 'p') key_color(5);
  if (k == '\r' || k == '\n')
    check_row();
}

static Picture *color_dragged=0;
static int drag_dx, drag_dy, drag_ox, drag_oy;

static void
click(int x, int y, int b)
{
  Picture *p = get_centered_pic();
  set_centered_pic(0);
  if (p == splash)
    return;
  if (p == youwin || p == youlose || solution_shown)
  {
    start_again();
    invalidate(0, 0, table_width, table_height);
    return;
  }
  drag_ox = x;
  drag_oy = y;
  if (x >= px && x < px+BSZ && y >= py && y < py+6*pdy)
  {
    color_dragged = colors[(y-py)/pdy];
    drag_dx = x - px;
    drag_dy = y - (((int)((y-py)/pdy))*pdy+py);
    return;
  }
  if (x >= gx && x < gx+4*gdx && y < gy+BSZ && y > gy+(MOVES-1)*gdy)
  {
    int ty;
    ty = ((gy+BSZ)-y)/(-gdy);
    color_dragged = guesses[ty][(x-gx)/gdx];
    if (color_dragged == bighole)
      color_dragged = 0;
    drag_dx = x - (((int)((x-gx)/gdx))*gdx+gx);
    ty = gy+ty*gdy;
    drag_dy = y - ty;
    return;
  }
  color_dragged = 0;
  if (x > rx-WGAP/2)
    check_row();
}

static void
drag(int x, int y, int b)
{
  if (color_dragged == 0) return;
  invalidate_exposure(drag_ox-drag_dx, drag_oy-drag_dy, BSZ, BSZ,
		      x-drag_dx, y-drag_oy, BSZ, BSZ);
  put_picture(color_dragged, x-drag_dx, y-drag_dy, 0, 0, BSZ, BSZ);
  drag_ox = x;
  drag_oy = y;
}

static void
drop(int x, int y, int b)
{
  if (color_dragged == 0) return;
  invalidate(drag_ox-drag_dx, drag_oy-drag_dy, BSZ, BSZ);
  x = x-drag_dx + BSZ/2;
  y = y-drag_dy + BSZ/2;
	 
  if (x >= gx && x < gx+4*gdx
      && y >= gy+active_row*gdy && y < gy+active_row*gdy+BSZ)
  {
    int i = (x-gx)/gdx;
    guesses[active_row][i] = color_dragged;
    redraw_row(active_row);
  }
  color_dragged = 0;
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
  { "drag", (void *)drag },
  { "drop", (void *)drop },
  { "init", (void *)init },
  { "key", (void *)key },
  { "redraw", (void *)redraw },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  register_imagelib(appimglib_imagelib);
  init_ace(argc, argv, fmap);

  splash = get_picture("mastermind");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");

  colors[0] = get_picture("mastermind-r");
  colors[1] = get_picture("mastermind-o");
  colors[2] = get_picture("mastermind-y");
  colors[3] = get_picture("mastermind-g");
  colors[4] = get_picture("mastermind-b");
  colors[5] = get_picture("mastermind-p");
  bighole = get_picture("mastermind-eb");
  cover = get_picture("mastermind-c");

  black = get_picture("mastermind-k");
  white = get_picture("mastermind-w");
  smallhole = get_picture("mastermind-e");

  px = WGAP;
  py = CARD_MARGIN;
  pdy = BSZ + 1;

  sx = 2*WGAP+BSZ;
  sy = CARD_MARGIN;
  sdx = BSZ + 3;

  gdx = BSZ + 3;
  gdy = -(BSZ + 3);
  gx = sx;
  gy = 2*CARD_MARGIN+BSZ - (MOVES-1)*gdy;

  rx = gx + 3*gdx + BSZ + WGAP;
  ry = gy + (BSZ-SSZ)/2;
  rdx = SSZ+3;
  rdy = gdy;

  table_width = rx + 3*rdx + SSZ + WGAP;
  table_height = gy + BSZ + CARD_MARGIN;

  init_table(table_width, table_height);
  table_loop();
}
