/* The Ace of Penguins - funcs.h
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

extern void (*click_cb)(int x, int y, int b);

extern void (*drag_cb)(int x, int y, int b);

extern void (*redraw_cb)();

extern void (*init_cb)();

extern void (*drop_cb)(int x, int y, int b);

extern void (*key_cb)(int x, int y, int b);

extern void (*resize_cb)(int w, int h);

extern void (*double_click_cb)(int x, int y, int b);
