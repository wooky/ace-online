/* The Ace of Penguins - tplib.c
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
#include "taipeilib.h"

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
