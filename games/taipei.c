/* The Ace of Penguins - taipei.c
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
#include "cards.h"
#include "taipeilib.h"

char *tile_names[] = {
  0, 0, 0, 0,
  "n1", "n1", "n1", "n1",
  "n2", "n2", "n2", "n2",
  "n3", "n3", "n3", "n3",
  "n4", "n4", "n4", "n4",
  "n5", "n5", "n5", "n5",
  "n6", "n6", "n6", "n6",
  "n7", "n7", "n7", "n7",
  "n8", "n8", "n8", "n8",
  "n9", "n9", "n9", "n9",

  "p1", "p1", "p1", "p1",
  "p2", "p2", "p2", "p2",
  "p3", "p3", "p3", "p3",
  "p4", "p4", "p4", "p4",
  "p5", "p5", "p5", "p5",
  "p6", "p6", "p6", "p6",
  "p7", "p7", "p7", "p7",
  "p8", "p8", "p8", "p8",
  "p9", "p9", "p9", "p9",

  "b1", "b1", "b1", "b1",
  "b2", "b2", "b2", "b2",
  "b3", "b3", "b3", "b3",
  "b4", "b4", "b4", "b4",
  "b5", "b5", "b5", "b5",
  "b6", "b6", "b6", "b6",
  "b7", "b7", "b7", "b7",
  "b8", "b8", "b8", "b8",
  "b9", "b9", "b9", "b9",

  "xj", "xj", "xj", "xj",
  "xa", "xa", "xa", "xa",
  "xx", "xx", "xx", "xx",

  "sh", "sh", "sh", "sh",
  "sd", "sd", "sd", "sd",
  "sc", "sc", "sc", "sc",
  "ss", "ss", "ss", "ss",

  "cr", "cg", "cb", "ck",
  "du", "dd", "dl", "dr"
};

signed char tp_standard[] = {
#include "taipei-standard.tp"
};

signed char tp_bridge[] = {
#include "taipei-bridge.tp"
};

signed char tp_castle[] = {
#include "taipei-castle.tp"
};

signed char tp_cube[] = {
#include "taipei-cube.tp"
};

signed char tp_glyph[] = {
#include "taipei-glyph.tp"
};

signed char tp_pyramid[] = {
#include "taipei-pyramid.tp"
};

signed char tp_spiral[] = {
#include "taipei-spiral.tp"
};

signed char tp_simple[] = {
#include "taipei-simple.tp"
};

signed char tp_smiley[] = {
#include "taipei-smiley.tp"
};

signed char tp_jason[] = {
#include "taipei-jason.tp"
};

signed char tp_rebecca[] = {
#include "taipei-rebecca.tp"
};

signed char *layouts[] = {
  tp_standard,
  tp_bridge,
  tp_castle,
  tp_cube,
  tp_glyph,
  tp_pyramid,
  tp_spiral,
  tp_simple,
  tp_smiley,
  tp_jason,
  tp_rebecca,
  0
};

int layout_number = 1;

#define NUM_TILES (sizeof(tile_names)/(sizeof(tile_names[0])))
#define NUM_TILE_SETS (NUM_TILES/4)

Picture *tiles[NUM_TILES], *blank_tile;
Picture *splash, *youwin, *youlose;

unsigned char exposures[GRID_SX][GRID_SY][GRID_SZ];

extern int table_width, table_height;

static int selected_x=-1, selected_y=-1, selected_z=-1;
static int selected_tile = 0;

int
main(int argc, char **argv)
{
  init_table(argc, argv,
	     GRID_SX*GRID_DX+GRID_SZ*GRID_DZ+2*MARGIN,
	     GRID_SY*GRID_DY+GRID_SZ*GRID_DZ+2*MARGIN);
  filename = argv[1];
  if (filename)
    layout_number = 0;
  table_loop();
}

static int
calculate_exposures(int x, int y, int z)
{
  int e = 0;
  if (z == GRID_SZ-1)
    return 0;
  if (grid[x+1][y+1][z+1]) e++;
  if (grid[x-1][y+1][z+1]) e++;
  if (grid[x+1][y-1][z+1]) e++;
  if (grid[x-1][y-1][z+1]) e++;
  if (grid[x-1][y][z+1]) e+=2;
  if (grid[x+1][y][z+1]) e+=2;
  if (grid[x][y-1][z+1]) e+=2;
  if (grid[x][y+1][z+1]) e+=2;
  if (grid[x][y][z+1]) e+=4;
  return e;
}

typedef struct XYZ {
  unsigned char x, y, z, a;
} XYZ;

static XYZ xyz[NUM_TILES];
static int n_xyz, num_tiles;
static int shuffle[NUM_TILES];
static XYZ backup[NUM_TILES];
static int n_backup;

#define ixyz(i) (((int *)xyz)[i])

#define TRACE_PLACEMENT 1
#if TRACE_PLACEMENT
int
show_grid()
{
  int x, y;
  for (y=0; y<GRID_SY; y++)
  {
    for (x=0; x<GRID_SX; x++)
    {
      if (grid[x][y][0] == 1)
	putchar(183);
      else if (grid[x][y][0] == 3)
	putchar('X');
      else if (grid[x][y][0] == 0)
	putchar(' ');
      else
	putchar('O');
    }
    putchar('\n');
  }
  putchar('\n');
}
#endif

static int one_means_filled = 1;

static int
is_filled(int x, int y, int z)
{
  if (x < 0 || x >= GRID_SX || y < 0 || y >= GRID_SY || z < 0 || z >= GRID_SZ)
    return 0;
  if (one_means_filled)
  {
    if (grid[x][y][z] == 1)
      return 1;
  }
  else
  {
    if (grid[x][y][z] > 3)
      return 1;
  }
  return 0;
}

static int
gather_usable_spots(XYZ *xyz)
{
  int x, y, z;
  int n_xyz = 0;
  for (y=0; y<GRID_SY; y++)
    for (x=0; x<GRID_SX; x++)
      for (z=0; z<GRID_SZ; z++)
	if (one_means_filled ? (grid[x][y][z] == 1) : (grid[x][y][z] > 3))
	{
	  int blocked_over=0, blocked_left=0, blocked_right=0;
	  int ux, uy;
	  for (ux=-1; ux<=1; ux++)
	    for (uy=-1; uy<=1; uy++)
	      if (is_filled(x+ux, y+uy, z+1))
		blocked_over=1;
	  for (uy=-1; uy<=1; uy++)
	  {
	    if (is_filled(x-2, y+uy, z))
	      blocked_left = 1;
	    if (is_filled(x+2, y+uy, z))
	      blocked_right = 1;
	  }
	  if (!blocked_over && (!blocked_left || !blocked_right))
	  {
	    xyz[n_xyz].x = x;
	    xyz[n_xyz].y = y;
	    xyz[n_xyz].z = z;
	    xyz[n_xyz].a = 0;
	    n_xyz++;
	  }
	}
  return n_xyz;
}

static int
place_tiles(int n)
{
  XYZ xyz[NUM_TILES], txyz; /* maximum number of spots */
  int n_xyz, i, j;

  if (n >= num_tiles)
    return 1;

  n_xyz = gather_usable_spots(xyz);
  for (i=0; i<n_xyz; i++)
  {
    j = rand() % n_xyz;
    txyz = xyz[i];
    xyz[i] = xyz[j];
    xyz[j] = txyz;
  }

  for (i=0; i<n_xyz-1; i++)
  {
    grid[xyz[i].x][xyz[i].y][xyz[i].z] = shuffle[n];
    for (j=i+1; j<n_xyz; j++)
    {
      grid[xyz[j].x][xyz[j].y][xyz[j].z] = shuffle[n+1];
      if (place_tiles(n+2))
	return 1;
      grid[xyz[j].x][xyz[j].y][xyz[j].z] = 1;
    }
    grid[xyz[i].x][xyz[i].y][xyz[i].z] = 1;
  }
  return 0;
}

static int defer_redraw = 1;

static void
bi_load(signed char *data)
{
  int x, y, z, i;
  memset(grid, 0, sizeof(grid));
  while (*data != 127)
  {
    x = *data++;
    if (x <= 0)
    {
      x = -x;
      y = *data++;
      z = *data++;
    }
    grid[x][y][z] = 1;
  }
}

static void
remove_pair(int x1, int y1, int z1, int x2, int y2, int z2)
{
  selected_tile = 0;
  selected_x = -1;

  backup[n_backup].x = x1;
  backup[n_backup].y = y1;
  backup[n_backup].z = z1;
  backup[n_backup].a = grid[x1][y1][z1];
  n_backup ++;
  grid[x1][y1][z1] = 0;
  invalidate_tile(x1, y1, z1);

  backup[n_backup].x = x2;
  backup[n_backup].y = y2;
  backup[n_backup].z = z2;
  backup[n_backup].a = grid[x2][y2][z2];
  n_backup ++;
  grid[x2][y2][z2] = 0;
  invalidate_tile(x2, y2, z2);

  num_tiles -= 2;
}

static void
redraw_free_tile_count()
{
  XYZ xyz[NUM_TILES], txyz; /* maximum number of spots */
  int n = gather_usable_spots(xyz), i, j, c;
  char tmp[10];
  c = 0;
  for (i=0; i<n; i++)
    for (j=0; j<n; j++)
      if (i != j
	  && ((grid[xyz[i].x][xyz[i].y][xyz[i].z] & ~3)
	      == (grid[xyz[j].x][xyz[j].y][xyz[j].z] & ~3)))
      {
	c ++;
	break;
      }
  sprintf(tmp, " %3d %3d", num_tiles, c);
  text(tmp, 2, table_height-2);
}

static void
shuffle_set(int *s)
{
  int i, j, t;
  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      if (rand() & 16)
      {
	t = s[i];
	s[i] = s[j];
	s[j] = t;
      }
}

void
start_again()
{
  int i, x, y, z, n;

  if (selected_tile)
  {
    int nx = selected_x;
    selected_tile = 0;
    selected_x = -1;
    invalidate_tile(nx, selected_y, selected_z);
  }

  /* load the board into grid (each cell is zero or one) */
  defer_redraw = 1;
  if (layout_number)
    bi_load(layouts[layout_number-1]);
  else
    load(0);
  defer_redraw = 0;

  /* count the number of tiles we need */
  num_tiles = 0;
  for (y=0; y<GRID_SY; y++)
    for (x=0; x<GRID_SX; x++)
      for (z=0; z<GRID_SZ; z++)
	if (grid[x][y][z])
	  num_tiles++;

  if (num_tiles > NUM_TILES-4 || num_tiles == 0 || num_tiles%2)
  {
    fprintf(stderr, "Invalid number of tiles: %d\n", num_tiles);
    exit(0);
  }

  /* build a list of all tiles */
  for (x=4; x<NUM_TILES; x++)
    shuffle[x-4] = x;
  /* randomize the colors and directions - the last two sets */
  shuffle_set(shuffle+NUM_TILES-8);
  shuffle_set(shuffle+NUM_TILES-12);

  if (layout_number > 9)
  {
    /* special case for "learning" sets */
    for (x=0; x<NUM_TILES-4; x+=2)
    {
      y = (rand() % (NUM_TILES-4)) & ~1;
      z = shuffle[x];
      shuffle[x] = shuffle[y];
      shuffle[y] = z;
      z = shuffle[x+1];
      shuffle[x+1] = shuffle[y+1];
      shuffle[y+1] = z;
    }
  }
  else
  {
    /* Randomize in groups of four */
    for (x=0; x<NUM_TILES-4; x+=4)
    {
      y = (rand() % (NUM_TILES-4)) & ~3;
      for (n=0; n<4; n++)
      {
	z = shuffle[x+n];
	shuffle[x+n] = shuffle[y+n];
	shuffle[y+n] = z;
      }
    }

    /* Now randomize the first N tiles in groups of two */
    for (x=0; x<num_tiles; x+=2)
    {
      y = (rand() % num_tiles) & ~1;
      z = shuffle[x];
      shuffle[x] = shuffle[y];
      shuffle[y] = z;
      z = shuffle[x+1];
      shuffle[x+1] = shuffle[y+1];
      shuffle[y+1] = z;
    }
  }

  /* Now place the tiles */

  one_means_filled = 1;
  if (! place_tiles(0))
  {
    fprintf(stderr, "Error: unable to find a way to lay out tiles.\n");
    exit(0);
  }
  n_backup = 0;
  one_means_filled = 0;

  redraw_free_tile_count();
  invalidate(0, 0, table_width, table_height);
}

void
init()
{
  int i;
  memset(grid, 0, sizeof(grid));
  for (i=4; i<NUM_TILES; i++)
  {
    char n[20];
    sprintf(n, "%s", tile_names[i]);
    tiles[i] = get_picture(n);
  }
  blank_tile = get_picture("bl");
  youwin = get_picture("youwin");
  youlose = get_picture("youlose");
  splash = get_picture("taipei");
  set_centered_pic(splash);
  start_again();
}

static void
redraw_tile(int x, int y, int z)
{
  Picture *p = tiles[grid[x][y][z]];
  if (calculate_exposures(x, y, z) >= 4)
    put_picture(blank_tile, gx2x(x, z), gy2y(y, z), 0, 0, TILE_SX, TILE_SY);
  else if (x == selected_x && y == selected_y && z == selected_z)
    put_picture_inverted(p, gx2x(x, z), gy2y(y, z), 0, 0, TILE_SX, TILE_SY);
  else
    put_picture(p, gx2x(x, z), gy2y(y, z), 0, 0, TILE_SX, TILE_SY);
}

void
redraw()
{
  int x, y, z;
  if (defer_redraw)
    return;
  for (z=0; z<GRID_SZ; z++)
  {
    for (x=GRID_SX-1; x>=0; x--)
      for (y=0; x+y<GRID_SX && y<GRID_SY; y++)
	if (grid[x+y][y][z])
	  redraw_tile(x+y, y, z);
    for (y=0; y<GRID_SY; y++)
      for (x=0; x+y<GRID_SY && x<GRID_SX; x++)
	if (grid[x][x+y][z])
	  redraw_tile(x, x+y, z);
  }
  redraw_free_tile_count();
}

extern char taipei_help[];

void
key(int k, int x, int y)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
  if (k == KEY_F(1) || k == 'h')
  {
    set_centered_pic(0);
    help("taipei.html", taipei_help);
  }
  if (k == KEY_F(2))
  {
    set_centered_pic(0);
    start_again();
  }
  if (k == 8 || k == 127 || k == KEY_DELETE)
  {
    int nx = selected_x;
    selected_tile = 0;
    set_centered_pic(0);
    selected_x = -1;
    invalidate_tile(nx, selected_y, selected_z);
    if (n_backup > 1)
    {
      int i;
      for (i=0; i<2; i++)
      {
	n_backup--;
	grid[backup[n_backup].x][backup[n_backup].y][backup[n_backup].z]
	  = backup[n_backup].a;
	invalidate_tile(backup[n_backup].x, backup[n_backup].y, backup[n_backup].z);
	num_tiles++;
      }
      redraw_free_tile_count();
    }
  }

  if (k >= '1' && k <= '9')
  {
    set_centered_pic(0);
    layout_number = k - '0';
    start_again();
  }
  if (k == 'j' || k == 'J')
  {
    set_centered_pic(0);
    layout_number = 10;
    start_again();
  }
  if (k == 'r' || k == 'R')
  {
    set_centered_pic(0);
    layout_number = 11;
    start_again();
  }
  if (k == '0' && filename)
  {
    set_centered_pic(0);
    layout_number = 0;
    start_again();
  }
}

static int
tile_is_free(int x, int y, int z)
{
  int dx, dy, bl=0, br=0;
  if (z < GRID_SZ-1)
  {
    for (dx=-1; dx<=1; dx++)
      for (dy=-1; dy<=1; dy++)
	if (grid[x+dx][y+dy][z+1])
	  return 0;
  }
  for (dy=-1; dy<=1; dy++)
  {
    if (y+dy < 0 || y+dy >= GRID_DY)
      continue;
    if (x-2 >= 0 && grid[x-2][y+dy][z])
      bl = 1;
    if (x+2 < GRID_SX && grid[x+2][y+dy][z])
      br = 1;
  }
  if (bl && br)
    return 0;
  return 1;
}

static void
remove_tile(int x, int y)
{
  int nx, ny, nz;
  for (nz=GRID_SZ-1; nz>=0; nz--)
    for (nx=0; nx<GRID_SX; nx++)
      if (x >= gx2x(nx, nz) && x < gx2x(nx, nz)+TILE_SX)
	for (ny=0; ny<GRID_SY; ny++)
	  if (y >= gy2y(ny, nz) && y < gy2y(ny, nz)+TILE_SY)
	    if (grid[nx][ny][nz])
	    {
	      if (!tile_is_free(nx, ny, nz))
	      {
		beep();
		return;
	      }
	      if (selected_tile && selected_x == nx
		  && selected_y == ny && selected_z == nz)
	      {
		selected_tile = 0;
		selected_x = -1;
		invalidate_tile(nx, ny, nz);
	      }
	      else if (selected_tile)
	      {
		if ((grid[nx][ny][nz] & ~3)
		    != (selected_tile & ~3))
		{
		  beep();
		  return;
		}
		remove_pair(selected_x, selected_y, selected_z, nx, ny, nz);
	      }
	      else
	      {
		selected_x = nx;
		selected_y = ny;
		selected_z = nz;
		selected_tile = grid[nx][ny][nz];
		invalidate_tile(nx, ny, nz);
	      }
	      return;
	    }
}


void
click(int x, int y, int b)
{
  Picture *cp = get_centered_pic();

  if ((cp == youlose || cp == youwin)
      && (x > table_width/2-cp->w/2
	  && x < table_width/2+cp->w/2
	  && y > table_height/2-cp->h/2
	  && y < table_height/2+cp->h/2))
  {
    set_centered_pic(0);
    start_again();
    return;
  }

  if (cp == splash)
  {
    set_centered_pic(0);
    return;
  }

  remove_tile(x, y);
  if (num_tiles == 0)
    set_centered_pic(youwin);
  redraw_free_tile_count();
}
