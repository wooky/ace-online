/* The Ace of Penguins - tplib.c
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
#include "taipeilib.h"
#include "imagelib.h"

unsigned char grid[GRID_SX][GRID_SY][GRID_SZ];

char *filename;
int layer;

extern int table_width, table_height;

void
load(int complain)
{
  int x, y, z;
  FILE *f;
  if (!filename) return;
  f = fopen(filename, "r");
  if (!f)
  {
    if (complain)
      perror(filename);
    return;
  }

  memset(grid, 0, sizeof(grid));
  layer = 0;
  while (fscanf(f, "%d%*[^0-9-]", &x) > 0)
  {
    if (x == 127)
      break;
    if (x <= 0)
    {
      x = -x;
      fscanf(f, "%d%*[^0-9-]%d%*[^0-9-]", &y, &z);
      if (layer < z)
	layer = z;
    }
    grid[x][y][z] = 1;
  }
    
  invalidate(0, 0, table_width, table_height);
}

void
invalidate_tile(int x, int y, int z)
{
  invalidate(gx2x(x, z), gy2y(y, z), TILE_SX, TILE_SY);
}

static void
tile_synth2(image *rv)
{
  image *img;
  int x, y, which;
  int bx, by, bw, bh;
  int xo, yo, dx, dy;
  int tw, th, txo, tyo;
  image_list *list = rv->list;
  int width = rv->width;
  int height = rv->height;

  img = get_image("taipei-tiles", width*6, height*9, 0);

  switch (img->width)
    {
    case 243:
      bx = 5; by = 5; bw = 39; bh = 39;
      xo = 50; yo = 6; dx = 36; dy = 36;
      tw = 34; th = 34; txo = 4; tyo = 1;
      break;
    }

  which = list->synth_flags;

  x = (which/10 - 2) * dx + xo;
  y = (which%10 - 1) * dy + yo;

  put_image (img, bx, by, bw, bh, rv, -bx, -by, 0);
  put_mask  (img, bx, by, bw, bh, rv, -bx, -by, 0);
  if (which)
    put_image (img, x, y, tw, th, rv, txo-x, tyo-y, 0);
}

static image *
tile_synth(image_list *list, int type, int width, int height)
{
  image *rv, *img;
  int bw, bh;

  for (rv = list->subimage[type]; rv; rv=rv->next)
    if (rv->width == width && rv->height == height)
      return rv;

  img = get_image("taipei-tiles", width*6, height*9, 0);

  switch (img->width)
    {
    case 243:
      bw = 39; bh = 39;
      break;
    }

  rv = alloc_synth_image(list, bw, bh, type);
  rv->synth_func = tile_synth2;
  return rv;
}

image_list tile_images[] = {
  { "bl", 1, 1, {0,0,0}, 0, tile_synth, 0 },

  { "n1", 1, 1, {0,0,0}, 0, tile_synth, 21 },
  { "n2", 1, 1, {0,0,0}, 0, tile_synth, 22 },
  { "n3", 1, 1, {0,0,0}, 0, tile_synth, 23 },
  { "n4", 1, 1, {0,0,0}, 0, tile_synth, 24 },
  { "n5", 1, 1, {0,0,0}, 0, tile_synth, 25 },
  { "n6", 1, 1, {0,0,0}, 0, tile_synth, 26 },
  { "n7", 1, 1, {0,0,0}, 0, tile_synth, 27 },
  { "n8", 1, 1, {0,0,0}, 0, tile_synth, 28 },
  { "n9", 1, 1, {0,0,0}, 0, tile_synth, 29 },

  { "p1", 1, 1, {0,0,0}, 0, tile_synth, 31 },
  { "p2", 1, 1, {0,0,0}, 0, tile_synth, 32 },
  { "p3", 1, 1, {0,0,0}, 0, tile_synth, 33 },
  { "p4", 1, 1, {0,0,0}, 0, tile_synth, 34 },
  { "p5", 1, 1, {0,0,0}, 0, tile_synth, 35 },
  { "p6", 1, 1, {0,0,0}, 0, tile_synth, 36 },
  { "p7", 1, 1, {0,0,0}, 0, tile_synth, 37 },
  { "p8", 1, 1, {0,0,0}, 0, tile_synth, 38 },
  { "p9", 1, 1, {0,0,0}, 0, tile_synth, 39 },

  { "b1", 1, 1, {0,0,0}, 0, tile_synth, 41 },
  { "b2", 1, 1, {0,0,0}, 0, tile_synth, 42 },
  { "b3", 1, 1, {0,0,0}, 0, tile_synth, 43 },
  { "b4", 1, 1, {0,0,0}, 0, tile_synth, 44 },
  { "b5", 1, 1, {0,0,0}, 0, tile_synth, 45 },
  { "b6", 1, 1, {0,0,0}, 0, tile_synth, 46 },
  { "b7", 1, 1, {0,0,0}, 0, tile_synth, 47 },
  { "b8", 1, 1, {0,0,0}, 0, tile_synth, 48 },
  { "b9", 1, 1, {0,0,0}, 0, tile_synth, 49 },

  { "xj", 1, 1, {0,0,0}, 0, tile_synth, 51 },
  { "xa", 1, 1, {0,0,0}, 0, tile_synth, 52 },
  { "xx", 1, 1, {0,0,0}, 0, tile_synth, 53 },

  { "cr", 1, 1, {0,0,0}, 0, tile_synth, 56 },
  { "cg", 1, 1, {0,0,0}, 0, tile_synth, 57 },
  { "cb", 1, 1, {0,0,0}, 0, tile_synth, 58 },
  { "ck", 1, 1, {0,0,0}, 0, tile_synth, 59 },

  { "sh", 1, 1, {0,0,0}, 0, tile_synth, 61 },
  { "sd", 1, 1, {0,0,0}, 0, tile_synth, 62 },
  { "sc", 1, 1, {0,0,0}, 0, tile_synth, 63 },
  { "ss", 1, 1, {0,0,0}, 0, tile_synth, 64 },

  { "du", 1, 1, {0,0,0}, 0, tile_synth, 66 },
  { "dd", 1, 1, {0,0,0}, 0, tile_synth, 67 },
  { "dl", 1, 1, {0,0,0}, 0, tile_synth, 68 },
  { "dr", 1, 1, {0,0,0}, 0, tile_synth, 69 },
  { 0 }
};

REGISTER_IMAGE_LIBRARY(tile_images)
