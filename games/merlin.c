/* The Ace of Penguins - merlin.c
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
#include <math.h>
#include "cards.h"

#include <X11/keysym.h>

Picture *xlogo, *splash, *youwin;
Picture *cell, *blank;

extern int table_width, table_height;

char grid[9];

char affects[9][9] = {
  "XX-XX----",
  "XXX------",
  "-XX-XX---",
  "X--X--X--",
  "-X-XXX-X-",
  "--X--X--X",
  "---XX-XX-",
  "------XXX",
  "----XX-XX"
};

int
main(int argc, char **argv)
{
  cell = get_picture("merlin-c.gif");
  blank = get_picture("merlin-b.gif");
  splash = get_picture("merlin.gif");
  youwin = get_picture("youwin.gif");
  xlogo = get_picture("xemboss.gif");

  init_table(argc, argv, 300, 300);
  table_loop();
}

void
start_again()
{
  int i;
  for (i=0; i<9; i++)
    grid[i] = rand() & 1;
}

void
init()
{
  set_centered_pic(splash);
  start_again();
}

static void
show(int i)
{
  int x = (i/3)*100;
  int y = (i%3)*100;
  if (grid[i])
    put_picture(cell, x, y, 0, 0, 100, 100);
  else
    put_picture(blank, x, y, 0, 0, 100, 100);
}

void
redraw()
{
  int i;
  for (i=0; i<9; i++)
    show(i);
}

void
click(int x, int y, int b)
{
  int i;
  if (get_centered_pic() == youwin)
  {
    set_centered_pic(0);
    start_again();
    redraw();
    return;
  }
  if (get_centered_pic())
  {
    set_centered_pic(0);
    return;
  }

  x /= 100;
  y /= 100;
  i = x*3+y;
  for (x=0; x<9; x++)
    if (affects[i][x] == 'X')
    {
      grid[x] ^= 1;
      show(x);
    }
  for (x=0; x<9; x++)
    if (grid[x] == 0)
      return;
  set_centered_pic(youwin);
}

extern char merlin_help[];

void
key(int k, int x, int y)
{
  int i;
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  set_centered_pic(0);
  if (k == XK_F1 || k == 'h')
  {
    set_centered_pic(0);
    help("merlin.html", merlin_help);
    return;
  }
  if (k == XK_F2)
  {
    start_again();
    redraw();
  }
}
