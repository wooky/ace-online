/* The Ace of Penguins - table-rn.c
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

/* Not all platforms do these right, so supply our own */

#include <time.h>

static unsigned long long next = 0;
static int initialized = 0;

int
rand(void)
{
  if (! initialized)
  {
    time((time_t *)&next);
    initialized = 1;
  }
  next = next * 0x5deece66dLL + 11;
  return (int)((next >> 16) & 0x7fffffff);
}

void
srand(unsigned seed)
{
  next = seed;
}
