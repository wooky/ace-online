/* The Ace of Penguins - pegged.c
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

#include <X11/keysym.h>

static Picture *xlogo, *splash, *youwin, *youlose;
static Picture *hole, *peg;
#define RADIUS (peg->w/2)

extern int table_width, table_height;

#define RULES_VH       	1
#define RULES_TRI	2

static char *layout_cross[] = {
  "    . . .    ",
  "    . * .    ",
  ". . * * * . .",
  ". . . * . . .",
  ". . . * . . .",
  "    . . .    ",
  "    . . .    ",
};

static char *layout_plus[] = {
  "    . . .    ",
  "    . * .    ",
  ". . . * . . .",
  ". * * * * * .",
  ". . . * . . .",
  "    . * .    ",
  "    . . .    ",
};

static char *layout_fireplace[] = {
  "    * * *    ",
  "    * * *    ",
  ". . * * * . .",
  ". . * . * . .",
  ". . . . . . .",
  "    . . .    ",
  "    . . .    ",
};

static char *layout_uparrow[] = {
  "    . * .    ",
  "    * * *    ",
  ". * * * * * .",
  ". . . * . . .",
  ". . . * . . .",
  "    * * *    ",
  "    * * *    ",
};

static char *layout_pyramid[] = {
  "    . . .    ",
  "    . * .    ",
  ". . * * * . .",
  ". * * * * * .",
  "* * * * * * *",
  "    . . .    ",
  "    . . .    ",
};

static char *layout_diamond[] = {
  "    . * .    ",
  "    * * *    ",
  ". * * * * * .",
  "* * * . * * *",
  ". * * * * * .",
  "    * * *    ",
  "    . * .    ",
};

static char *layout_solitaire[] = {
  "    * * *    ",
  "    * * *    ",
  "* * * * * * *",
  "* * * . * * *",
  "* * * * * * *",
  "    * * *    ",
  "    * * *    ",
};

static char *layout_triangle5[] = {
  "*        ",
  "* *      ",
  "* * *    ",
  "* * * *  ",
  "* * * * *",
};

static char *layout_triangle7[] = {
  "*            ",
  "* *          ",
  "* * *        ",
  "* * * *      ",
  "* * * * *    ",
  "* * * * * *  ",
  "* * * * * * *",
};

static struct {
  int key, width, height, rules;
  char **layout;
} layouts[] = {
  '1', 7, 7, RULES_VH, layout_cross,
  '2', 7, 7, RULES_VH, layout_plus,
  '3', 7, 7, RULES_VH, layout_fireplace,
  '4', 7, 7, RULES_VH, layout_uparrow,
  '5', 7, 7, RULES_VH, layout_pyramid,
  '6', 7, 7, RULES_VH, layout_diamond,
  '7', 7, 7, RULES_VH, layout_solitaire,
  '8', 5, 5, RULES_TRI, layout_triangle5,
  '9', 7, 7, RULES_TRI, layout_triangle7,
  0, 0
};
#define INITIAL_LAYOUT 6

static int layout_number = 0;

#define MAXX	7
#define MAXY	7

#define GRID_HOLE	1
#define GRID_PEG	2

#define GRID_XX_TRI	40
#define GRID_YX_TRI	20
#define GRID_YY_TRI	34

static char grid[MAXX+1][MAXY+1];
static int gr, gw, gh;

static int gx[MAXX+1][MAXY+1];
static int gy[MAXX+1][MAXY+1];

static struct UndoInfo {
  unsigned short initial_pick:1;
  unsigned short from_x:3, from_y:3;
  unsigned short to_x:3, to_y:3;
} undo_list[49];
static int next_undo = 0;

static void
start_again()
{
  int x, y, cx, cy;
  gr = layouts[layout_number].rules;
  gw = layouts[layout_number].width;
  gh = layouts[layout_number].height;

  switch (gr)
  {
  case RULES_VH:
    cx = table_width/2 - (gw-1)*20;
    cy = table_height/2 - (gh-1)*20;
    break;
  case RULES_TRI:
    cx = table_width/2 - (gw-1)*20;
    cy = table_height/2 - (gh-1)*GRID_YY_TRI/2;
    break;
  default:
    printf("no rules!\n");
    exit(1);
  }
  for (x=0; x<gw; x++)
    for (y=0; y<gh; y++)
    {
      switch (layouts[layout_number].layout[y][x*2])
      {
      case ' ':
	grid[x][y] = 0;
	break;
      case '.':
	grid[x][y] = GRID_HOLE;
	break;
      case '*':
	grid[x][y] = GRID_PEG;
	break;
      }
      switch (gr)
      {
      case RULES_VH:
	gx[x][y] = cx+x*40;
	gy[x][y] = cy+y*40;
	break;
      case RULES_TRI:
	gx[x][y] = cx + x*GRID_XX_TRI + (gh-y-1)*GRID_YX_TRI;
	gy[x][y] = cy + y*GRID_YY_TRI;
      }
    }
  invalidate(0, 0, table_width, table_height);
  next_undo = 0;
}

static void
init()
{
  layout_number = INITIAL_LAYOUT;
  set_centered_pic(splash);
  start_again();
}

static void
xyput(int x, int y, Picture *p)
{
  int px, py;
  px = gx[x][y] - p->w/2;
  py = gy[x][y] - p->h/2;
  put_picture(p, px, py, 0, 0, p->w, p->h);
}

static void
redraw()
{
  int x, y;
  put_picture(xlogo, table_width-xlogo->w-4, 4, 0, 0, xlogo->w, xlogo->h);
  for (x=0; x<gw; x++)
    for (y=0; y<gh; y++)
      switch (grid[x][y])
      {
      case GRID_HOLE:
	xyput(x, y, hole);
	break;
      case GRID_PEG:
	xyput(x, y, peg);
	break;
      }
}

static void
set_grid(int x, int y, int which)
{
  if (grid[x][y] == which)
    return;
  grid[x][y] = which;
  switch (which)
  {
  case GRID_HOLE:
    /* the peg is bigger */
    clear(gx[x][y]-RADIUS, gy[x][y]-RADIUS, RADIUS*2, RADIUS*2);
    xyput(x, y, hole);
    break;
  case GRID_PEG:
    xyput(x, y, peg);
    break;
  }
}

static int
find_hole(int x, int y, int *rx, int *ry)
{
  int ix, iy;
  for (ix=0; ix<gw; ix++)
    for (iy=0; iy<gh; iy++)
      if (x > gx[ix][iy]-RADIUS-5 && x < gx[ix][iy]+RADIUS+5
	  && y > gy[ix][iy]-RADIUS-5 && y < gy[ix][iy]+RADIUS+5)
      {
	*rx = ix;
	*ry = iy;
	return 1;
      }
  return 0;
}

static int holes_left, pegs_left;

static void
count_stuff()
{
  int x, y;
  holes_left=0;
  pegs_left=0;
  for (x=0; x<gw; x++)
    for (y=0; y<gh; y++)
      switch (grid[x][y])
      {
      case GRID_HOLE:
	holes_left++;
	break;
      case GRID_PEG:
	pegs_left++;
	break;
      }
}

static int
see_if_lose()
{
  int x, y;
  for (x=0; x<gw; x++)
    for (y=0; y<gh; y++)
      if (grid[x][y] == GRID_PEG)
      {
	if (x > 1 && grid[x-1][y] == GRID_PEG && grid[x-2][y] == GRID_HOLE)
	  return 0;
	if (x < gw-2 && grid[x+1][y] == GRID_PEG && grid[x+2][y] == GRID_HOLE)
	  return 0;
	if (y > 1 && grid[x][y-1] == GRID_PEG && grid[x][y-2] == GRID_HOLE)
	  return 0;
	if (y < gh-2 && grid[x][y+1] == GRID_PEG && grid[x][y+2] == GRID_HOLE)
	  return 0;
	if (gw == RULES_TRI)
	{
	  if (x > 1 && y > 1 && grid[x-1][y-1] == GRID_PEG
	      && grid[x-2][y-2] == GRID_HOLE)
	    return 0;
	  if (x < gw-2 && y < gh-2 && grid[x+1][y+1] == GRID_PEG
	      && grid[x+2][y+2] == GRID_HOLE)
	    return 0;
	}
      }

  set_centered_pic(youlose);
  return 1;
}

static int click_x, click_y, ofs_x, ofs_y;
static int drag_x, drag_y, dragging;

static void
click(int mx, int my, int b)
{
  int x, y;

  Picture *cp = get_centered_pic();
  set_centered_pic(0);
  if (cp == youwin || cp == youlose)
    start_again();

  dragging = 0;
  if (!find_hole(mx, my, &x, &y))
    return;

  count_stuff();

  if (holes_left == 0 && grid[x][y] == GRID_PEG)
  {
    set_grid(x, y, GRID_HOLE);
    undo_list[next_undo].initial_pick = 1;
    undo_list[next_undo].from_x = x;
    undo_list[next_undo].from_y = y;
    next_undo++;
    return;
  }

  if (grid[x][y] != GRID_PEG)
    return;

  grid[x][y] = GRID_HOLE;
  click_x = x;
  click_y = y;
  ofs_x = mx - gx[x][y];
  ofs_y = my - gy[x][y];
  drag_x = gx[x][y];
  drag_y = gy[x][y];
  dragging = 1;
}

static void
drag(int x, int y, int b)
{
  if (!dragging)
    return;
  invalidate(drag_x-RADIUS, drag_y-RADIUS, RADIUS*2, RADIUS*2);
  drag_x = x - ofs_x;
  drag_y = y - ofs_y;
  put_picture(peg, drag_x-peg->w/2, drag_y-peg->h/2, 0, 0, peg->w, peg->h);
}

static void
drop(int x, int y, int b)
{
  if (!dragging)
    return;
  invalidate(drag_x-RADIUS, drag_y-RADIUS, RADIUS*2, RADIUS*2);
  dragging = 0;
  if (find_hole(x-ofs_x, y-ofs_y, &x, &y))
  {
    if (grid[x][y] == GRID_HOLE)
    {
      int jump_ok = 0;
      if ((x == click_x+2 && y == click_y)
	  || (x == click_x-2 && y == click_y)
	  || (x == click_x && y == click_y+2)
	  || (x == click_x && y == click_y-2))
	jump_ok = 1;
      switch (gr)
      {
      case RULES_TRI:
      if ((x == click_x+2 && y == click_y+2)
	  || (x == click_x-2 && y == click_y-2))
	jump_ok = 1;
	break;
      }
      if (jump_ok)
      {
	int jx, jy;
	jx = (x+click_x)/2;
	jy = (y+click_y)/2;
	if (grid[jx][jy] == GRID_PEG)
	{
	  set_grid(jx, jy, GRID_HOLE);
	  set_grid(x, y, GRID_PEG);
	  undo_list[next_undo].initial_pick = 0;
	  undo_list[next_undo].from_x = click_x;
	  undo_list[next_undo].from_y = click_y;
	  undo_list[next_undo].to_x = x;
	  undo_list[next_undo].to_y = y;
	  next_undo++;

	  count_stuff();

	  if (pegs_left == 1)
	    set_centered_pic(youwin);
	  else
	    see_if_lose();

	  return;
	}
      }
    }
  }
  set_grid(click_x, click_y, GRID_PEG);
}

extern char pegged_help[];

static void
key(int k, int x, int y)
{
  int i;
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  set_centered_pic(0);
  if (k == KEY_F(2) || k == 'r')
  {
    start_again();
    return;
  }
  if (k == KEY_F(1) || k == 'h')
  {
    set_centered_pic(0);
    help("pegged.html", pegged_help);
    return;
  }
  if (k == 8 || k == 127 || k == KEY_DELETE)
  {
    if (next_undo > 0)
    {
      struct UndoInfo *u;
      next_undo--;
      u = undo_list+next_undo;
      if (! u->initial_pick)
      {
	set_grid(u->to_x, u->to_y, GRID_HOLE);
	set_grid((u->from_x+u->to_x)/2, (u->from_y+u->to_y)/2, GRID_PEG);
      }
      set_centered_pic(0);
      set_grid(u->from_x, u->from_y, GRID_PEG);
    }
    return;
  }
  for (i=0; layouts[i].key; i++)
    if (layouts[i].key == k)
    {
      layout_number = i;
      start_again();
      return;
    }
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
  hole = get_picture("pegged-h");
  peg = get_picture("pegged-p");
  splash = get_picture("pegged");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");
  xlogo = get_picture("xemboss");

  init_table(320, 320);
  table_loop();
}
