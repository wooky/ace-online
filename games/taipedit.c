/* The Ace of Penguins - tpe.c
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

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "cards.h"
#include "taipeilib.h"

static Picture *tpe_bs, *tpe_bu, *tpe_bt, *splash;
static Picture *tiles[9];

static int lbx, lby, lbw, lbh;

extern int table_background;
extern Display *display;
extern int screen;
extern Window window;
extern GC gc;
extern int table_width, table_height;

static void
save()
{
  int x, y, z, samey, w=-1;
  FILE *f;
  if (!filename) return;
  f = fopen(filename, "w");
  if (!f)
  {
    perror(filename);
    return;
  }

  for (z=0; z<GRID_SZ; z++)
    for (y=0; y<GRID_SY; y++)
    {
      samey = 0;
      for (x=0; x<GRID_SX; x++)
	if (grid[x][y][z])
	{
	  if (w > 20)
	  {
	    w = 0;
	    fputc('\n', f);
	  }
	  if (samey)
	  {
	    fprintf(f, "%d,", x);
	    w += 1;
	  }
	  else
	  {
	    fprintf(f, "-%d,%d,%d,", x, y, z);
	    samey = 1;
	    w += 3;
	  }
	}
    }
  fprintf(f, "127\n");
  fclose(f);
}

static void
init()
{
  lbx = table_width-MARGIN-tpe_bs->w;
  lby = table_height-MARGIN-tpe_bs->h;
  lbw = tpe_bs->w;
  lbh = tpe_bs->h;
  load(0);
}

static int tile_count = 0;

static void
show_tile_count()
{
  int x, y, z, b4;
  char tmp[10];
  tile_count = 0;
  for (x=0; x<GRID_SX; x++)
    for (y=0; y<GRID_SY; y++)
      for (z=0; z<GRID_SZ; z++)
	if (grid[x][y][z])
	  tile_count++;
  sprintf(tmp, "%5d%c", tile_count, tile_count%4 ? '!' : ' ');
  text(tmp, lbx, lby-lbh*GRID_SZ+2);
}

static void
redraw()
{
  int x, y, z;
  int dy = GRID_DZ-1;
  XSetForeground(display, gc, BlackPixel(display, screen));
  for (x=0; x<=GRID_SX; x++)
    XDrawLine(display, window, gc,
	      gx2x(x,0), gy2y(0,0)+dy, gx2x(x,0), gy2y(GRID_SY,0)+dy);
  for (y=0; y<=GRID_SY; y++)
    XDrawLine(display, window, gc,
	      gx2x(0,0), gy2y(y,0)+dy, gx2x(GRID_SX,0), gy2y(y,0)+dy);
  XDrawLine(display, window, gc, gx2x(0,0), gy2y(0,0)+dy,
	    gx2x(GRID_SX, 0), gy2y(GRID_SY, 0)+dy);
  XDrawLine(display, window, gc, gx2x(0,0), gy2y(GRID_SY, 0)+dy,
	    gx2x(GRID_SX, 0), gy2y(0,0)+dy);
  for (z=0; z<GRID_SZ; z++)
  {
    Picture *p;
    if (z < layer)
      p = tpe_bs;
    else if (z == layer)
      p = tpe_bt;
    else
      p = tpe_bu;
    put_picture(p, lbx, lby-z*lbh, 0, 0, p->w, p->h);
  }

  for (z=0; z<=layer; z++)
  {
    for (x=GRID_SX-1; x>=0; x--)
      for (y=0; x+y<GRID_SX && y<GRID_SY; y++)
	if (grid[x+y][y][z])
	  put_picture(tiles[z], gx2x(x+y, z), gy2y(y, z), 0, 0, TILE_SX, TILE_SY);
    for (y=0; y<GRID_SY; y++)
      for (x=0; x+y<GRID_SY && x<GRID_SX; x++)
	if (grid[x][x+y][z])
	  put_picture(tiles[z], gx2x(x, z), gy2y(x+y, z), 0, 0, TILE_SX, TILE_SY);
  }

  show_tile_count();
}

static int
check_layer_buttons(int x, int y)
{
  int z;
  if (x >= lbx)
  {
    for (z=0; z<GRID_SZ; z++)
    {
      if (y >= lby-z*lbh && y < lby-z*lbh+lbh)
      {
	if (layer != z)
	{
	  layer = z;
	  invalidate(0, 0, table_width, table_height);
	}
	return 1;
      }
    }
  }
  return 0;
}

static void
add_tile(int x, int y)
{
  int nx, ny, nz;
  int cx, cy;
  if (tile_count >= 144)
    return;
  nx = x2gx(x-GRID_DX/2, layer);
  ny = y2gy(y-GRID_DY/2, layer);
  nz = layer;
  if (nx < 0 || nx >= GRID_SX-1
      || ny < 0 || ny >= GRID_SY-1
      || nz < 0 || nz >= GRID_SZ)
    return;
  for (cx=nx-1; cx<=nx+1; cx++)
    if (cx >= 0 && cx < GRID_SX)
      for (cy=ny-1; cy<=ny+1; cy++)
	if (cy >= 0 && cy < GRID_SY)
	  if (grid[cx][cy][nz])
	    return;
  grid[nx][ny][nz] = 1;
  invalidate_tile(nx, ny, nz);
  show_tile_count();
}

static void
remove_tile(int x, int y)
{
  int nx, ny, nz;
  for (nx=0; nx<GRID_SX; nx++)
    if (x >= gx2x(nx, layer) && x < gx2x(nx, layer)+TILE_SX)
      for (ny=0; ny<GRID_SY; ny++)
	if (y >= gy2y(ny, layer) && y < gy2y(ny, layer)+TILE_SY)
	  if (grid[nx][ny][layer])
	  {
	    grid[nx][ny][layer] = 0;
	    invalidate_tile(nx, ny, layer);
	    show_tile_count();
	  }
}

static void
click(int x, int y, int b)
{
  if (get_centered_pic())
  {
    set_centered_pic(0);
    return;
  }

  if (check_layer_buttons(x, y))
    return;
  if (b == 1)
    add_tile(x, y);
  else
    remove_tile(x, y);
}

static void
drag(int x, int y, int b)
{
  click(x, y, b);
}

static int
check(int x1, int x2, int y1, int y2, int z1, int z2)
{
  int x, y, z;
  for (x = x1; x <= x2; x++)
    for (y = y1; y <= y2; y++)
      for (z = z1; z <= z2; z++)
	if (grid[x][y][z])
	  return 0;
  return 1;
}

static int
shift(int dx, int dy, int dz)
{
  int x, y, z, x1=0, x2=GRID_SX-1, y1=0, y2=GRID_SY-1, z1=0, z2=GRID_SZ-1;
  unsigned char temp[GRID_SX][GRID_SY][GRID_SZ];
  memset(temp, 0, sizeof(temp));
  if (dx > 0) x2 -= dx;
  if (dx < 0) x1 -= dx;
  if (dy > 0) y2 -= dy;
  if (dy < 0) y1 -= dy;
  if (dz > 0) z2 -= dz;
  if (dz < 0) z1 -= dz;
  for (x=x1; x<=x2; x++)
    for (y=y1; y<=y2; y++)
      for (z=z1; z<=z2; z++)
	temp[x][y][z] = grid[x+dx][y+dy][z+dz];
  memcpy(grid, temp, sizeof(temp));
  invalidate(0, 0, table_width, table_height);
}

static void
auto_center()
{
  int x, y, z, minx=GRID_SX, maxx=0, miny=GRID_SY, maxy=0, minz=GRID_SZ, maxz=0;
  for (x=0; x<GRID_SX; x++)
    for (y=0; y<GRID_SY; y++)
      for (z=0; z<GRID_SZ; z++)
	if (grid[x][y][z])
	{
	  if (minx > x) minx = x;
	  if (maxx < x) maxx = x;
	  if (miny > y) miny = y;
	  if (maxy < y) maxy = y;
	  if (minz > z) minz = z;
	  if (maxz < z) maxz = z;
	}
  shift((minx+maxx-GRID_SX+2)/2, (miny+maxy-GRID_SY+2)/2, minz);
}

extern char taipedit_help[];

static void
key(int k, int x, int y)
{
  if (get_centered_pic())
  {
    set_centered_pic(0);
    return;
  }

  if (k == 3 || k == 27 || k == 'q')
    exit(0);

  if (k == KEY_F(1) || k == 'h')
  {
    help("taipedit.html", taipedit_help);
  }

  if (k == 'l')
    load(1);

  if (k == 's')
    save();

  if (k >= '1' && k < '1'+GRID_SZ)
  {
    if (layer != k - '1')
    {
      layer = k - '1';
      invalidate(0, 0, table_width, table_height);
    }
  }

  switch (k)
    {
    case KEY_UP:
      if (check(0, GRID_SX-1, 0, 0, 0, GRID_SZ-1))
	shift(0, 1, 0);
      break;
    case KEY_DOWN:
      if (check(0, GRID_SX-1, GRID_SY-2, GRID_SY-1, 0, GRID_SZ-1))
	shift(0, -1, 0);
      break;
    case KEY_LEFT:
      if (check(0, 0, 0, GRID_SY-1, 0, GRID_SZ-1))
	shift(1, 0, 0);
      break;
    case KEY_RIGHT:
      if (check(GRID_SX-2, GRID_SX-1, 0, GRID_SY-1, 0, GRID_SZ-1))
	shift(-1, 0, 0);
      break;
    case KEY_PGUP:
      if (check(0, GRID_SX-1, 0, GRID_SY-1, GRID_SZ-1, GRID_SZ-1))
	shift(0, 0, -1);
      break;
    case KEY_PGDN:
      if (check(0, GRID_SX-1, 0, GRID_SY-1, 0, 0))
	shift(0, 0, 1);
      break;
    case KEY_HOME:
      auto_center();
      break;
    }
}

static FunctionMapping fmap[] = {
  { "click", (void *)click },
  { "drag", (void *)drag },
  { "init", (void *)init },
  { "key", (void *)key },
  { "redraw", (void *)redraw },
  { 0, 0 }
};

int
main(int argc, char **argv)
{
  int x, y, z;
  char tmp[20];
  memset(grid, 0, sizeof(grid));

  register_imagelib(appimglib_imagelib);
  register_imagelib(tile_images);
  init_ace(argc, argv, fmap);

  layer = 0;

  for (x=1; x<9; x++)
  {
    sprintf(tmp, "n%d", x);
    tiles[x-1] = get_picture(tmp);
  }

  tpe_bs = get_picture("taipedit-bs");
  tpe_bu = get_picture("taipedit-bu");
  tpe_bt = get_picture("taipedit-bt");
  splash = get_picture("taipedit");
  set_centered_pic(splash);

  init_table(GRID_SX*GRID_DX+GRID_SZ*GRID_DZ+3*MARGIN + tpe_bs->w,
	     GRID_SY*GRID_DY+GRID_SZ*GRID_DZ+2*MARGIN);

  filename = argv[1];
  table_loop();
}
