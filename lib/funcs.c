/* The Ace of Penguins - funcs.c
   Copyright (C) 2001 DJ Delorie

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
#include "funcs.h"

static void
default_click_cb(int x, int y, int b)
{
}

void (*click_cb)(int,int,int) = default_click_cb;

static void
default_drag_cb(int x, int y, int b)
{
}

void (*drag_cb)(int,int,int) = default_drag_cb;

static void
default_redraw_cb()
{
  stack_redraw();
}

void (*redraw_cb)() = default_redraw_cb;

static void
default_init_cb()
{
}

void (*init_cb)() = default_init_cb;

static void
default_drop_cb(int x, int y, int b)
{
}

void (*drop_cb)(int,int,int) = default_drop_cb;

static void
default_key_cb(int x, int y, int k)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
}

void (*key_cb)(int,int,int) = default_key_cb;

static void
default_resize_cb(int w, int h)
{
  table_no_resize();
}

void (*resize_cb)(int,int) = default_resize_cb;

static void
default_double_click_cb(int x, int y, int b)
{
  click_cb(x, y, b);
}

void (*double_click_cb)(int,int,int) = default_double_click_cb;
