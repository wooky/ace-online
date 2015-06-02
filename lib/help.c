/* The Ace of Penguins - help.c
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "cards.h"
#include "xwin.h"
#include "table.h"

extern Display *display;
extern int screen;
extern Window window;
extern GC gc;
extern int help_background, table_background;
extern int table_width, table_height;

static int help_foreground, help_beyondcolor;

extern int help_is_showing;
extern void (*help_redraw)(void);
extern void (*help_click)(int x, int y, int b);
extern void (*help_key)(int c, int x, int y);

#define STYLE_NONE	0x00
#define STYLE_B		0x01
#define STYLE_I		0x02
#define STYLE_TT	0x04
#define STYLE_BIG	0x08
#define STYLE_BITS	0x0f
#define FLOAT_LEFT	0x01
#define FLOAT_RIGHT	0x02
#define ALIGN_CENTER	0x03
#define PTR_IMG		0x10
#define BLK_BR		0x20
#define BLK_P		0x40
#define HEADER		0x80

#define MENU_FONT	0x10

static XFontStruct *fonts[17];
static int thin_space[17];

static struct {
  char *tag;
  int taglen;
  int bits_set;
  int bits_reset;
} tags[] = {
  { "<b>", 0, STYLE_B, 0 },
  { "</b>", 0, 0, STYLE_B },
  { "<i>", 0, STYLE_I, 0 },
  { "</i>", 0, 0, STYLE_I },
  { "<tt>", 0, STYLE_TT, 0 },
  { "</tt>", 0, 0, STYLE_TT },
  { "<big>", 0, STYLE_BIG, 0 },
  { "</big>", 0, 0, STYLE_BIG },
  { "<h1>", 0, HEADER, STYLE_BITS },
  { "</h1>", 0, BLK_P, STYLE_BITS },
  { "<h2>", 0, STYLE_BIG|STYLE_B|BLK_P, 0 },
  { "</h2>", 0, BLK_P, STYLE_BITS },
  { "<p>", 0, BLK_P, STYLE_BITS },
  { "</p>", 0, BLK_P, STYLE_BITS },
  { "<br>", 0, BLK_BR, 0 },
  { "<img src=", 0, PTR_IMG, 0 },
  { "<img align=center src=", 0, PTR_IMG|ALIGN_CENTER, 0 },
  { "<img align=left src=", 0, PTR_IMG|FLOAT_LEFT, 0 },
  { "<img align=right src=", 0, PTR_IMG|FLOAT_RIGHT, 0 },
};
#define NTAGS (sizeof(tags)/sizeof(tags[0]))

typedef struct Word {
  short x, y;
  char a, d, lb;
  unsigned char flags;
  short rb, width;
  char *ptr;
} Word;

static Word *words=0;
static int nwords=0;
static char *file=0;

static int *menus=0;
static int num_menus;
static int cur_page = 0;
static int menu_height;

static int vscroll = 0;
static int max_vscroll = 0;
static int cur_menu = 0;

static void
show_page (int n, int m)
{
  int i, y;
  cur_page = n;
  cur_menu = m;
  vscroll = 0;
  for (i=cur_page; i<nwords; i++)
  {
    Word *w = words+i;
    if (w->flags & HEADER)
      break;
    y = w->y+w->d;
    if (max_vscroll < y)
      max_vscroll = y;
  }
  max_vscroll -= (table_height - menu_height);
  if (max_vscroll < 0)
    max_vscroll = 0;
  invalidate(0, 0, table_width, table_height);
}

static void
help_init()
{
  int i;
  int bigpoints, smallpoints;
  int use_helvetica;

  if (table_type != TABLE_COLOR)
    {
      help_background = pixel_for(255, 255, 255);
      help_foreground = pixel_for(0, 0, 0);
      help_beyondcolor = pixel_for(255, 255, 255);
    }
  else
    {
      help_background = table_background;
      help_foreground = pixel_for(255, 255, 255);
      help_beyondcolor = pixel_for(0, 0, 0);
    }
  if (table_width < 300)
    {
      bigpoints = 120;
      smallpoints = 80;
      use_helvetica = 1;
    }
  else
    {
      bigpoints = 180;
      smallpoints = 100;
      use_helvetica = 0;
    }
  for (i=0; i<16; i++)
  {
    char name[100];
    sprintf(name, "*-%s-%s-%s-*--*-%d-*-*-*-*-*-*",
	    i & STYLE_TT ? "courier" : use_helvetica ? "helvetica" : "times",
	    i & STYLE_B ? "bold" : "medium",
	    i & STYLE_I ? ((i & STYLE_TT || use_helvetica) ? "o" : "i") : "r",
	    i & STYLE_BIG ? bigpoints : smallpoints);
    fonts[i] = XLoadQueryFont(display, name);
  }
  fonts[MENU_FONT] =
    XLoadQueryFont(display, "*-helvetica-medium-r-normal--*-100-*-*-*-*-*-*");
  for (i=0; i<17; i++)
  {
    if (!fonts[i])
      fonts[i] = XLoadQueryFont(display, i & STYLE_TT ? "fixed" : "variable");
    thin_space[i] = XTextWidth(fonts[i], " ", 1);
  }
  for (i=0; i<NTAGS; i++)
    tags[i].taglen = strlen(tags[i].tag);

  menu_height = fonts[MENU_FONT]->ascent + fonts[MENU_FONT]->descent
    + 2*thin_space[MENU_FONT];
}

static int mx = 0;

static void
my_help_redraw()
{
  int i, l, r;
  int ts = thin_space[MENU_FONT];
  int y = menu_height;
  int saw_cur_menu = 0;
  Word *w;

  mx = 0;
  w = words + menus[cur_menu];
  i = w->x + w->width + ts;
  if (i > table_width - 20)
    mx = i - table_width + ts/2 + 20;

  XSetForeground(display, gc, help_foreground);
  XSetFont(display, gc, fonts[MENU_FONT]->fid);
  for (i=0; i<num_menus; i++)
  {
    w = words+menus[i];
    l = w->x-ts-mx;
    r = w->x+w->width+ts-mx;
    XDrawString(display, window, gc, w->x-mx, w->y, w->ptr, strlen(w->ptr));
    XDrawLine(display, window, gc, l, y-2, l, 5);
    XDrawLine(display, window, gc, r, y-2, r, 5);
    XDrawLine(display, window, gc, l+4, 1, r-4, 1);
    XDrawLine(display, window, gc, l+1, 4, l+1, 3);
    XDrawLine(display, window, gc, r-1, 4, r-1, 3);
    XDrawLine(display, window, gc, l+2, 2, l+3, 2);
    XDrawLine(display, window, gc, r-2, 2, r-3, 2);
    XDrawPoint(display, window, gc, l-1, y-1);
    if (menus[i] == cur_page-1)
    {
      XDrawLine(display, window, gc, -2, y, l-2, y);
      XDrawLine(display, window, gc, r+2, y, table_width+2, y);
      saw_cur_menu = 1;
    }
  }
  XDrawPoint(display, window, gc, r+1, y-1);
  if (!saw_cur_menu)
    XDrawLine(display, window, gc, 0, y, table_width, y);

  XSetForeground(display, gc, help_beyondcolor);
  XFillRectangle(display, window, gc, r+2, 0, table_width-r-2, y);
  XDrawLine(display, window, gc, 0, 0, table_width, 0);
  XDrawLine(display, window, gc, 0, 0, 0, y-1);
  XDrawLine(display, window, gc, 1, 0, 1, y-2);
  XDrawLine(display, window, gc, 2, 0, 2, 4);
  XDrawLine(display, window, gc, 3, 0, 3, 2);
  XDrawLine(display, window, gc, 4, 1, 5, 1);
  XDrawPoint(display, window, gc, 3, 0);
  for (i=0; i<num_menus; i++)
  {
    w = words+menus[i];
    r = w->x+w->width+ts+1-mx;
    XDrawLine(display, window, gc, r, 5, r, y-2);
    XDrawLine(display, window, gc, r-1, 4, r+1, 4);
    XDrawLine(display, window, gc, r-1, 3, r+1, 3);
    XDrawLine(display, window, gc, r-2, 2, r+2, 2);
    XDrawLine(display, window, gc, r-4, 1, r+4, 1);
  }

  clip_more (0, menu_height+3, table_width, table_height-menu_height-3);
  for (i=cur_page; i<nwords; i++)
  {
    w = words+i;
    if (w->flags & HEADER)
      break;
    if (w->flags & PTR_IMG)
      put_picture((Picture *)w->ptr, w->x, w->y-w->a-vscroll, 0, 0, w->width, w->a+w->d);
    else
    {
      XSetForeground(display, gc, help_foreground);
      XSetFont(display, gc, fonts[w->flags & STYLE_BITS]->fid);
      XDrawString(display, window, gc, w->x, w->y-vscroll, w->ptr, strlen(w->ptr));
    }
  }
  unclip();
}

static void
my_help_key(int c, int x, int y)
{
  int m, vs, old_vscroll = vscroll;
  if (c == 'q')
    exit(1);
  if (c == 27 || c == ' ' || c == KEY_F(1) || c == 8 || c == 127 || c == KEY_DELETE)
  {
    free(file);
    file = 0;
    help_is_showing = 0;
    invalidate(0, 0, table_width, table_height);
    return;
  }
  if (c >= '0' && c <= '9')
  {
    m = c-'1';
    if (m == -1 && !(words[0].flags & HEADER))
	show_page(0, 0);
    else if (m >= 0 && m < num_menus)
      show_page(menus[m]+1, m);
    return;
  }
  for (m=0; m<num_menus; m++)
    if (tolower(words[menus[m]].ptr[0]) == tolower(c))
    {
      show_page(menus[m]+1, m);
      return;
    }

  if (c == KEY_LEFT && cur_menu > 0)
    show_page (menus[cur_menu-1]+1, cur_menu-1);
  if (c == KEY_RIGHT && cur_menu < num_menus-1)
    show_page (menus[cur_menu+1]+1, cur_menu+1);

  vs = 0;
  switch (c)
    {
    case KEY_DOWN: vs = 10; break;
    case KEY_PGDN: vs = (table_height-menu_height)*9/10; break;
    case KEY_UP: vs = -10; break;
    case KEY_PGUP: vs = -(table_height-menu_height)*9/10; break;
    }
  if (vs)
    {
      vscroll += vs;
      if (vscroll > max_vscroll)
	vscroll = max_vscroll;
      if (vscroll < 0)
	vscroll = 0;
      if (vscroll != old_vscroll)
	invalidate(0, menu_height+3, table_width, table_height);
    }
}

static void
my_help_click(int x, int y, int b)
{
  int i, old_vscroll = vscroll;
  if (y < menu_height)
  {
    Word *w = 0;
    for (i=0; i<num_menus; i++)
    {
      w = words+menus[i];
      if (x+mx > w->x && x+mx < w->x+w->width)
      {
	show_page(menus[i]+1, i);
	return;
      }
    }
    if (!w || ! (words[0].flags & HEADER) && x+mx > w->x+w->width+2*thin_space[MENU_FONT])
    {
      show_page(0, 0);
      return;
    }
  }

  /* rescale y to whole screen */
  y = (y-menu_height) * table_height / (table_height - menu_height);

  if (y > table_height/3 && y < table_height*2/3)
    {
      if (x < table_width/6)
	{
	  my_help_key (KEY_LEFT, x, y);
	  return;
	}
      else if (x > table_width*5/6)
	{
	  my_help_key (KEY_RIGHT, x, y);
	  return;
	}
    }
  vscroll += (y - table_height/2);
  if (vscroll > max_vscroll)
    vscroll = max_vscroll;
  if (vscroll < 0)
    vscroll = 0;
  if (vscroll != old_vscroll)
    invalidate(0, menu_height+3, table_width, table_height);
}

#define MARGIN 10

void
help(char *filename, char *text)
{
  int i, j, flags=0;
  char *cp;
  struct stat st;
  FILE *f;
  static int initted = 0;
  int row_top, row_width;
  int max_width = table_width - 2 * MARGIN;
  int first_in_row;
  int menu_x = 2;
  Word *floating_left = 0, *floating_right = 0;
  int float_margin = 0, float_left_margin = 0;

  if (!initted) help_init();

  max_vscroll = 0;

  f = fopen(filename, "r");
  if (!f)
  {
    int l = strlen(text)+1;
    file = (char *)malloc(l);
    memcpy(file, text, l);
  }
  else
  {
    stat(filename, &st);
    file = (char *)malloc(st.st_size+1);
    fread(file, 1, st.st_size, f);
    fclose(f);
    file[st.st_size] = 0;
  }

  nwords = 2; /* initial word plus filler */
  for (cp=file; *cp; cp++)
    if (*cp == '<'
	|| *cp == '>' && !isspace(cp[1])
	|| isspace(*cp) && !isspace(cp[1]))
      nwords++;

  words = (Word *)malloc(nwords * sizeof(Word));
  nwords = 0;
  num_menus = 2;
  cp = file;
  while (*cp)
  {
    while (*cp && isspace(*cp)) *cp++ = 0;
    if (*cp == '<')
    {
      for (i=0; i<NTAGS; i++)
	if (strncmp(cp, tags[i].tag, tags[i].taglen) == 0)
	{
	  if (tags[i].bits_set & PTR_IMG)
	  {
	    Picture *p;
	    char *rbr;
	    cp += tags[i].taglen;
	    for (rbr=cp; *rbr && *rbr != '>'; rbr++);
	    *rbr = 0;
	    words[nwords].flags = ((flags & (HEADER|BLK_BR|BLK_P))
				   | PTR_IMG
				   | tags[i].bits_set);
	    flags &= ~(BLK_BR|BLK_P|PTR_IMG|HEADER);
	    p = get_picture(cp);
	    words[nwords].ptr = (char *)p;
	    words[nwords].a = p->h;
	    words[nwords].width = p->w;
	    words[nwords].d = words[nwords].rb = words[nwords].lb = 0;
	    if ((tags[i].bits_set & ALIGN_CENTER) == ALIGN_CENTER)
	    {
	      XFontStruct *f = fonts[0];
	      words[nwords].a = p->h/2 + (f->ascent-f->descent)/2;
	      words[nwords].d = p->h - words[nwords].a;
	      words[nwords].flags &= ~ALIGN_CENTER;
	    }
	    cp = rbr+1;
	    nwords ++;
	  }
	  else
	  {
	    flags &= ~tags[i].bits_reset;
	    flags |= tags[i].bits_set;
	    *cp = 0;
	    cp += tags[i].taglen;
	  }
	  break;
	}
      if (i < NTAGS)
	continue;
    }
    if (*cp && !isspace(*cp))
    {
      words[nwords].flags = flags;
      words[nwords].ptr = cp;
      if (flags & HEADER)
	num_menus++;
      nwords ++;
      cp++;
      if (flags & HEADER)
	while (*cp && *cp != '<') cp++;
      else
	while (*cp && !isspace(*cp) && *cp != '<') cp++;
      flags &= ~(BLK_BR|BLK_P|PTR_IMG|HEADER);
    }
  }

  menus = (int *)malloc(num_menus * sizeof(*menus));
  num_menus = 0;

  for (i=0; i<nwords; i++)
  {
    Word *w = words+i;
    int junk;
    XCharStruct cs;
    XFontStruct *fs = fonts[w->flags & STYLE_BITS];
    if (!(w->flags & PTR_IMG))
    {
      if (w->flags & HEADER) fs = fonts[MENU_FONT];
      w->a = fs->ascent;
      w->d = fs->descent;
      XTextExtents(fs, w->ptr, strlen(w->ptr),
		   &junk, &junk, &junk, &cs);
      w->lb = cs.lbearing;
      w->rb = cs.rbearing;
      w->width = cs.width;
      if (w->flags & HEADER)
	menus[num_menus++] = i;
    }
  }
  menus[num_menus] = i;
  if (menus[0] == 0)
    cur_page = 1;
  else
    cur_page = 0;

  row_top = menu_height + MARGIN;
  row_width = 0;
  first_in_row = 0;
  for (i=0; i<=nwords; i++)
  {
    int end_para = 0;
    Word *w = words+i;
    int ts = (thin_space[words[i-1].flags & STYLE_BITS]
	      + thin_space[words[i].flags & STYLE_BITS])/2;

    /* decide when we're done with a row */
#if 0
    printf("i==%d rw=%d ts=%d rw+ts+ww=%d mw=%d f=%d\n",
	   i==nwords, row_width, ts, row_width + w->width + ts, max_width,
	   w->flags & (BLK_P|BLK_BR|HEADER));
#endif

    if (i == nwords) end_para = 1;
    if (row_width + w->width + ts > max_width - float_margin) end_para = 1;
    if (w->flags & (BLK_P|BLK_BR|HEADER)) end_para = 1;
    if (row_width == 0) end_para = 0;
    if (row_width > 0 && w->flags & PTR_IMG && w->flags & FLOAT_LEFT)
      end_para = 1;
    if (w->flags & PTR_IMG && w->flags & FLOAT_LEFT && floating_left)
      end_para = 1;
    if (w->flags & PTR_IMG && w->flags & FLOAT_RIGHT && floating_right)
      end_para = 1;
    if (end_para)
    {
      int maxa=0, maxd=0;
      for (j=first_in_row; j<i; j++)
      {
	if (words[j].flags & PTR_IMG && words[j].flags & (FLOAT_RIGHT|FLOAT_LEFT))
	  continue;
	if (maxa < words[j].a) maxa = words[j].a;
	if (maxd < words[j].d) maxd = words[j].d;
      }
      for (j=first_in_row; j<i; j++)
      {
	if (words[j].flags & PTR_IMG && words[j].flags & (FLOAT_RIGHT|FLOAT_LEFT))
	  continue;
	words[j].y = row_top + maxa;
      }
      row_top += maxa + maxd;
      if (w->flags & BLK_P)
	row_top += MARGIN;
      row_width = 0;
      first_in_row = i;
    }
    if (i == nwords) break;

    if (w->flags & PTR_IMG && w->flags & FLOAT_LEFT)
    {
      if (floating_left)
	row_top = floating_left->y;
      floating_left = w;
      w->x = MARGIN;
      w->y = row_top + w->a;
      continue;
    }

    if (w->flags & PTR_IMG && w->flags & FLOAT_RIGHT)
    {
      if (floating_right)
	row_top = floating_right->y;
      floating_right = w;
      w->x = table_width - MARGIN - w->width;
      w->y = row_top + w->a;
      continue;
    }

    float_margin = float_left_margin = 0;
      if (floating_left)
      {
	if (floating_left->y <= row_top)
	  floating_left = 0;
	else
	{
	  float_margin += floating_left->width + MARGIN;
	  float_left_margin = floating_left->width + MARGIN;
	}
      }
      if (floating_right)
      {
	if (floating_right->y <= row_top)
	  floating_right = 0;
	else
	  float_margin += floating_right->width + MARGIN;
      }

    if (w->flags & HEADER)
    {
      w->x = menu_x + thin_space[MENU_FONT];
      w->y = fonts[MENU_FONT]->ascent + thin_space[MENU_FONT]+1;
      menu_x += w->width + 2*thin_space[MENU_FONT] + 2;
      row_top = menu_height + MARGIN;
      first_in_row++;
    }
    else
    {
      if (row_width > 0)
	row_width += ts;
      w->x = row_width + MARGIN + float_left_margin;
      row_width += w->width;
    }
#if 0
    printf("[%02d] f=%02x x=%3d y=%3d a=%2d d=%2d lb=%2d rb=%2d w=%2d s=`%.*s'\n",
	   i, w->flags, w->x, w->y, w->a, w->d, w->lb, w->rb,
	   w->width, strlen(w->ptr), w->ptr);
#endif
  }
#if 0
  for (i=0; i<nwords; i++)
  {
    Word *w = words+i;
    printf("[%02d] f=%02x x=%3d y=%3d a=%2d d=%2d lb=%2d rb=%2d w=%2d s=`%.*s'\n",
	   i, w->flags, w->x, w->y, w->a, w->d, w->lb, w->rb,
	   w->width, strlen(w->ptr), w->ptr);
  }
#endif
  
  help_is_showing = 1;
  help_redraw = my_help_redraw;
  help_click = my_help_click;
  help_key = my_help_key;
  show_page (cur_page, 0);
}
