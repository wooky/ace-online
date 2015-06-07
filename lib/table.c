/* The Ace of Penguins - table.c
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
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

#define CD printf("%d: %d %d %d %d\n", __LINE__, ex, ey, ew, eh)
#undef CD
#define CD

#include "imagelib.h"
#include "table.h"
#include "cards.h"
#include "xwin.h"
#include "funcs.h"

#define TRACE_EVENTS		0
#define TRACE_PICTURES		0
#define TRACE_INVALIDATE	0

#define DBLCLICK_TIME		800
#define DBLCLICK_MOVE		5


int table_width=0, table_height=0, table_type;
int display_width, display_height;

static Picture *centered_pic = 0;

static int ex=0, ey=0, ew=0, eh=0;

static int graphics_disabled = 1;

OptionDesc *app_options;
OptionDesc *xwin_options;
static OptionDesc *options[5];

static OptionDesc ace_options[] = {
  { "-width", OPTION_INTEGER, &table_width },
  { "-height", OPTION_INTEGER, &table_height },
  { 0, 0, 0 }
};

static FunctionMapping flist[] = {
  { "click", &click_cb },
  { "drag", &drag_cb },
  { "redraw", &redraw_cb },
  { "init", &init_cb },
  { "drop", &drop_cb },
  { "key", &key_cb },
  { "resize", &resize_cb },
  { "double_click", &double_click_cb },
  { 0, 0 }
};

extern image_list cards_imagelib[];
static image_list card_images[];

void
init_ace(int argc, char **argv, FunctionMapping *funcs)
{
  int i = 0, o, a, errors=0;

  register_imagelib(cards_imagelib);
  register_imagelib(card_images);

  if (app_options)
    options[i++] = app_options;
  if (xwin_options)
    options[i++] = xwin_options;
  options[i++] = ace_options;
  options[i++] = 0;

  for (i=0; funcs[i].name; i++)
    for (a=0; flist[a].name; a++)
      if (strcmp(funcs[i].name, flist[a].name) == 0)
	*(void **)flist[a].function = funcs[i].function;

  for (a=1; a<argc; a++)
    {
      int found = 0;
      if (argv[a][0] != '-')
	break;
      for (i=0; options[i]; i++)
	for (o=0; options[i][o].option; o++)
	  if (strcmp (options[i][o].option, argv[a]) == 0)
	    {
	      found = 1;
	      if (options[i][o].type != OPTION_BOOLEAN && a == argc-1)
		{
		  fprintf(stderr, "Option `%s' takes an argument\n", argv[a]);
		  errors++;
		  continue;
		}
	      switch (options[i][o].type)
		{
		case OPTION_BOOLEAN:
		  *(int *)(options[i][o].ptr) = 1;
		  break;
		case OPTION_STRING:
		  *(char **)(options[i][o].ptr) = argv[a+1];
		  a++;
		  break;
		case OPTION_INTEGER:
		  *(int *)(options[i][o].ptr) = strtol(argv[a+1], 0, 0);
		  a++;
		  break;
		}
	    }
      if (!found)
	{
	  fprintf(stderr, "Unrecognized option `%s'\n", argv[a]);
	  errors++;
	}
    }
  if (errors)
    exit(errors);

  i = 1;
  while (a < argc)
    argv[i++] = argv[a++];
  argv[i] = 0;

  if (xwin_init(argc, argv))
    exit(1);
}

void
init_table(int width, int height)
{

  if (width > display_width)
    width = display_width;
  if (height > display_height)
    height = display_height;

  ew = width;
  eh = height;

  table_width = width;
  table_height = height;

  xwin_create (width, height);
}

typedef struct PicRec {
  Pixmap pixmap;
  Pixmap mask;
  char **xpm_data;
  int image_table_index;
} PicRec;

int get_picture_default_width  = CARD_WIDTH;
int get_picture_default_height = CARD_HEIGHT;

Picture *
get_picture(char *name)
{
  image *img;
  img = get_image(name, get_picture_default_width, get_picture_default_height, 0);
  return (Picture *)img;
}

static int put_picture_flags = 0;

void
put_picture(Picture *picture, int dx, int dy,
	    int x, int y, int w, int h)
{
  if (!picture) return;
  if (graphics_disabled) return;
#if TRACE_PICTURES
  /*printf("copy bef: x=%3d y=%3d w=%3d h=%3d\n", dx+x, dy+y, w, h);*/
#endif

  if (dx+x < ex)
  {
    w -= ex-(dx+x);
    x += ex-(dx+x);
  }
  if (dx+x+w > ex+ew)
    w = ex+ew - (dx+x);
  if (dy+y < ey)
  {
    h -= ey-(dy+y);
    y += ey-(dy+y);
  }
  if (dy+y+h > ey+eh)
    h = ey+eh - (dy+y);
#if TRACE_PICTURES
  printf("copy clip: x=%3d y=%3d w=%3d h=%3d (ex=%d ey=%d ew=%d eh=%d)\n",
	 dx+x, dy+y, w, h, ex, ey, ew, eh);
#endif
  if (w>0 && h>0)
  {
#if TRACE_PICTURES
    printf("copy aft: x=%3d y=%3d w=%3d h=%3d\n", dx+x, dy+y, w, h);
#endif
    put_image ((image *)picture, x, y, w, h, display_image, dx, dy, put_picture_flags);
  }
}

void
put_picture_inverted(Picture *picture, int dx, int dy,
		     int x, int y, int w, int h)
{
  put_picture_flags = PUT_INVERTED;
  put_picture (picture, dx, dy, x, y, w, h);
  put_picture_flags = 0;
}

void
set_centered_pic(Picture *picture)
{
  int x, y, w=0, h=0;
  if (centered_pic)
  {
    x = table_width/2-centered_pic->w/2;
    y = table_height/2-centered_pic->h/2;
    w = centered_pic->w;
    h = centered_pic->h;
  }
  centered_pic = picture;
  if (centered_pic)
  {
    if (centered_pic->w > w)
    {
      x = table_width/2-centered_pic->w/2;
      w = centered_pic->w;
    }
    if (centered_pic->h > h)
    {
      y = table_height/2-centered_pic->h/2;
      h = centered_pic->h;
    }
  }
  if (! graphics_disabled)
    invalidate(x, y, w, h);
}

Picture *
get_centered_pic()
{
  return centered_pic;
}

static void
redraw_centered_pic()
{
  if (centered_pic)
    put_picture(centered_pic, table_width/2-centered_pic->w/2,
                table_height/2-centered_pic->h/2,
                0, 0, centered_pic->w, centered_pic->h);
}

#define CLEAR_CLIP	ex = 0; ey = 0; ew = table_width; eh = table_height; xwin_noclip();

static int dcx, dcy, dct=0; /* double click memory */
static int drag_enabled=0;

static void
check_dclick(int x, int y, int t)
{
  if (t>dct+DBLCLICK_TIME)
    dct = t-2*DBLCLICK_TIME;
  if (x < dcx-DBLCLICK_MOVE || x>dcx+DBLCLICK_MOVE
      || y < dcy-DBLCLICK_MOVE || y > dcy+DBLCLICK_MOVE)
  {
    dct = t-2*DBLCLICK_TIME;
    drag_enabled = 1;
  }
}

int help_is_showing = 0;
static void help_nothing() { help_is_showing = 0; }
void (*help_redraw)(void) = help_nothing;
void (*help_click)(int x, int y, int b) = help_nothing;
void (*help_key)(int c, int x, int y) = help_nothing;

static int no_resize = 0;
void
table_no_resize()
{
  no_resize = 1;
}

static int initted = 0;

static void
maybe_init()
{
  if (! initted)
    {
      initted = 1;
      flush();
      graphics_disabled = 1;
      init_cb();
      graphics_disabled = 0;
#if TRACE_EVENTS
      printf(" - done init\n");
#endif
    }
}

void
table_loop()
{
  int first_expose = 0;
  int click_button;

  while (1)
  {
    XWin_Event event;
    xwin_nextevent(&event);

    if (!initted && (event.type != ev_expose && event.type != ev_resize))
      continue;

    switch (event.type)
      {
      case ev_resize:
#if TRACE_EVENTS
	printf("resize: x=%3d y=%3d w=%3d h=%3d, no=%d\n",
	       event.x, event.y, event.w, event.h, no_resize);
#endif

	maybe_init();

	if (no_resize)
	  xwin_fixed_size(table_width, table_height);
	else
	  {
	    graphics_disabled = 1;
	    resize_cb(event.w, event.h);
	    graphics_disabled = 0;
	    if (no_resize)
	      xwin_fixed_size(table_width, table_height);
	    else
	      {
		table_width = event.w;
		table_height = event.h;
		if (first_expose)
		  {
		    clear (0, 0, table_width, table_height);
		    redraw_cb();
		  }
	      }
	  }
	break;

      case ev_expose:
	first_expose = 1;
	ex = event.x;
	ey = event.y;
	ew = event.w;
	eh = event.h;
#if TRACE_EVENTS
	printf("expose: x=%3d y=%3d w=%3d h=%3d\n",
	       ex, ey, ew, eh);
#endif
	CD;
	xwin_clip (ex, ey, ew, eh);
	clear(ex, ey, ew, eh);

	maybe_init();

	ex = event.x;
	ey = event.y;
	ew = event.w;
	eh = event.h;
	CD;
	if (help_is_showing)
	  help_redraw();
	else
	  redraw_cb();
	redraw_centered_pic();
	xwin_noclip();
#if TRACE_EVENTS
	printf(" - done expose\n");
#endif
	break;

      case ev_buttondown:
	CLEAR_CLIP;
	CD;
#if TRACE_EVENTS
	printf("click: %d,%d %d\n",
	       event.x, event.y, event.button);
#endif
	click_button = event.button;
	check_dclick(event.x, event.y, event.time);
	if (help_is_showing)
	  {
	    help_click(event.x, event.y, click_button);
	  }
	else if (event.time - dct < DBLCLICK_TIME)
	  {
	    double_click_cb(event.x, event.y, event.button);
	    dct -= DBLCLICK_TIME;
	  }
	else
	  {
	    click_cb(event.x, event.y, click_button);
	    dcx = event.x;
	    dcy = event.y;
	    dct = event.time;
	  }
	drag_enabled = 0;
	break;

      case ev_motion:
	CLEAR_CLIP;
	CD;
	check_dclick(event.x, event.y, event.time);
	if (drag_enabled && !help_is_showing)
	  {
#if TRACE_EVENTS
	    printf("drag: %d,%d %d\n", event.x, event.y, click_button);
#endif
	    drag_cb(event.x, event.y, click_button);
	  }
	break;

      case ev_buttonup:
	CLEAR_CLIP;
	check_dclick(event.x, event.y, event.time);
	CD;
#if TRACE_EVENTS
	printf("drop: %d,%d\n", event.x, event.y);
#endif
	if (!help_is_showing)
	  drop_cb(event.x, event.y, click_button);
	break;

      case ev_keypress:
	CLEAR_CLIP;
	CD;
	if (help_is_showing)
	  help_key(event.key, event.x, event.y);
	else
	  key_cb(event.key, event.x, event.y);
	break;

      case ev_quit:
	exit(0);
      }
  }
}

static void
reset_clip()
{
  xwin_clip (ex, ey, ew, eh);
}

void
clip(int x, int y, int w, int h)
{
  if (graphics_disabled) return;
  ex = x;
  ey = y;
  ew = w;
  eh = h;

  if (ew < 0) ew = 0;
  if (eh < 0) eh = 0;
  if (ex < 0)
  {
    ew += ex;
    ex = 0;
  }
  if (ey < 0)
  {
    eh += ey;
    ey = 0;
  }
  if (ex+ew > table_width)
    ew = table_width - ex;
  if (ey+eh > table_height)
    eh = table_height - ey;
  reset_clip();
}

static int *clip_saves = 0;

void
clip_more(int x, int y, int w, int h)
{
  int *save = (int *)malloc (5*sizeof(int));
  save[0] = (int)clip_saves;
  save[1] = ex;
  save[2] = ey;
  save[3] = ew;
  save[4] = eh;
  clip_saves = save;

  if (x+w > ex+ew)
    w = ex+ew - x;
  if (y+h > ey+eh)
    h = ey+eh - y;
  if (x < ex)
    {
      w -= ex-x;
      x = ex;
    }
  if (y < ey)
    {
      h -= ey-y;
      y = ey;
    }
  clip (x, y, w, h);
}

void
unclip()
{
  int *ptr = clip_saves;
  if (!ptr) return;
  ex = clip_saves[1];
  ey = clip_saves[1];
  ew = clip_saves[1];
  eh = clip_saves[1];
  clip_saves = (int *)clip_saves[0];
  free (clip_saves);
  xwin_noclip();
  reset_clip();
}

static void
invalidate_sub(int x, int y, int w, int h)
{
#if TRACE_INVALIDATE
  printf("inv_sub(%d, %d, %d, %d)\n", x, y, w, h);
#endif
  ex = x;
  ey = y;
  ew = w;
  eh = h;
  CD;
  reset_clip();
  clear(ex, ey, ew, eh);
  if (help_is_showing)
    help_redraw();
  else
    redraw_cb();
  redraw_centered_pic();
  xwin_noclip();
}

void
invalidate(int x, int y, int w, int h)
{
  int ox = ex;
  int oy = ey;
  int ow = ew;
  int oh = eh;
  if (graphics_disabled) return;
#if TRACE_INVALIDATE
  printf("invalidate(%d, %d, %d, %d)\n", x, y, w, h);
#endif
  invalidate_sub(x, y, w, h);
#if TRACE_INVALIDATE
  printf(" - done invalidate\n");
#endif
  ex = ox;
  ey = oy;
  ew = ow;
  eh = oh;
  reset_clip();
  CD;
}

void
invalidate_nc(int x, int y, int w, int h)
{
  int ox = ex;
  int oy = ey;
  int ow = ew;
  int oh = eh;
  if (graphics_disabled) return;
#if TRACE_INVALIDATE
  printf("invalidate_nc(%d, %d, %d, %d)\n", x, y, w, h);
#endif
  ex = x;
  ey = y;
  ew = w;
  eh = h;
  CD;
  reset_clip();
  if (help_is_showing)
    help_redraw();
  else
    redraw_cb();
  redraw_centered_pic();
#if TRACE_INVALIDATE
  printf(" - done invalidate\n");
#endif
  ex = ox;
  ey = oy;
  ew = ow;
  eh = oh;
  reset_clip();
  CD;
}

/* Invalidate the results of moving a picture and "exposing" what
   it used to cover. */
void
invalidate_exposure(int ox, int oy, int ow, int oh,
		    int nx, int ny, int nw, int nh)
{
  int oex = ex;
  int oey = ey;
  int oew = ew;
  int oeh = eh;
  if (graphics_disabled) return;

#if TRACE_INVALIDATE
  printf("invalidate_exposure(%d, %d, %d, %d - %d, %d, %d, %d)\n",
	 ox, oy, ow, oh, nx, ny, nw, nh);
#endif

  /* check for the non-overlapping case */
  if (ox+ow <= nx || ox >= nx+nw || oy+oh <= ny || oy >= ny+nh)
  {
#if TRACE_INVALIDATE
    printf("- not overlapping\n");
#endif
    invalidate_sub(ox, oy, ow, oh);
    ex = oex;
    ey = oey;
    ew = oew;
    eh = oeh;
    CD;
    return;
  }

  /* now we know that they *do* overlap */
      
  if (nx+nw < ox+ow) /* exposure on the right */
  {
#if TRACE_INVALIDATE
    printf("- exposed on right\n");
#endif
    invalidate_sub(nx+nw, oy, (ox+ow)-(nx+nw), oh);
    ow = (nx+nw) - ox;
  }

  if (ox < nx) /* exposure on the left */
  {
#if TRACE_INVALIDATE
    printf("- exposed on left\n");
#endif
    invalidate_sub(ox, oy, (nx-ox), oh);
    ow = (ox+ow) - nx;
    ox = nx;
  }
      
  if (ny+nh < oy+oh) /* exposure on the bottom */
  {
#if TRACE_INVALIDATE
    printf("- exposed on bottom\n");
#endif
    invalidate_sub(ox, ny+nh, ow, (oy+oh)-(ny+nh));
    oh = (ny+nh) - oy;
  }

  if (oy < ny) /* exposure on the top */
  {
#if TRACE_INVALIDATE
    printf("- exposed on top\n");
#endif
    invalidate_sub(ox, oy, ow, (ny-oy));
    /* Not needed */
    /* oh = (oy+oh) - ny; */
    /* oy = ny; */
  }
  ex = oex;
  ey = oey;
  ew = oew;
  eh = oeh;
  CD;

#if TRACE_INVALIDATE
  printf("- done invalidate_exposure\n");
#endif
}


static int
snap_one(int x, int step, int origin, int m, int *did_it)
{
  int offset, dx;

  x -= origin;
  dx = ((x+step/2) % step) - step/2;
  if (-m <= dx && dx <= m)
  {
    x -= dx;
    *did_it = 1;
  }

  x += origin;
  return x;
}

void
snap_to_grid(int *x, int *y,
	     int step_x, int step_y,
	     int origin_x, int origin_y,
	     int max_distance)
{
  int sx, sy, snapx=0, snapy=0;
  sx = snap_one(*x, step_x, origin_x, max_distance, &snapx);
  sy = snap_one(*y, step_y, origin_y, max_distance, &snapy);
  if (snapx && snapy)
  {
    *x = sx;
    *y = sy;
  }
}

static char *suit_spots[] = {
  "15",
  "101:",
  "10151:",
  "00200:2:",
  "0020150:2:",
  "002005250:2:",
  "00201205250:2:",
  "0020032307270:2:",
  "002003231507270:2:",
  "00201104240626190:2:"
  };

static int spot_xx[] = { 0, 50, 100 };
static int spot_yy[] = { 0, 16, 25, 33, 36, 50, 63, 66, 75, 83, 100 };

static void
card_synth2(image *rv)
{
  image *face_img, *img;
  int face, suit, color;
  static char face_chars[] = "a234567890jqk";
  static char suit_chars[] = "cdsh";
  int subw, subh, face_w, w;
  int width = rv->width, height = rv->height;
  image_list *list = rv->list;

  fill_image (rv, 0, 0, width, height, 255, 255, 255);

  face = strchr(face_chars, list->name[0]) - face_chars;
  suit = strchr(suit_chars, list->name[1]) - suit_chars;
  color = suit & 1;

  face_w = width*2/11;
  face_img = get_image("a-k", face_w*2, face_w*13, 0);
  face_w = face_img->width / face_img->list->across;

  if (face < 10 && width > 3*face_w)
    {
      char *spots = suit_spots[face];
      int sw, sh;
      if (face == 0)
	{
	  sw = width;
	  sh = height;
	}
      else
	{
	  sw = (width - 2*face_w) / 3;
	  sh = (height - 2*face_w) / 4;
	}
      if (face == 0 && suit == 2)
	img = get_image("penguin", sw, sh, GI_NOT_BIGGER);
      else
	img = get_image("suits", sw, sh*4, GI_NOT_BIGGER);
      if (img)
	{
	  int spw, sph, spoh, spow;
	  sw = img->width / img->list->across;
	  sh = img->height / img->list->down;
	  spow = face_w + 2;
	  spoh = face_w*3/4 + 2;
	  spw = width - 2*spow - sw;
	  sph = height - 2*spoh - sh;
	  while (*spots)
	    {
	      int sx = spot_xx[spots[0]-'0'] * spw / 100 + spow;
	      int sy = spot_yy[spots[1]-'0'] * sph / 100 + spoh;
	      put_subimage (img, 0, suit, rv, sx, sy,
			    spot_yy[spots[1]-'0'] > 51 ? PUT_ROTATED : 0);
	      spots += 2;
	    }
	}
    }

  if (face >= 10 &&  width > 3*face_w)
    {
      int wm = face_w+2;
      int hm = face_w*3/4+2;
      int w2 = width-2*wm;
      int h2 = height-2*hm;
      image *kqj, *simg;
      static char *portrait[] = {"jack", "queen", "king"};

      fill_image (rv, wm, hm, w2, 1, 0, 0, 0);
      fill_image (rv, wm, hm, 1, h2, 0, 0, 0);
      fill_image (rv, wm, height-hm, w2, 1, 0, 0, 0);
      fill_image (rv, width-wm, hm, 1, h2, 0, 0, 0);

      simg = get_image("suits", w2/3, w2*4/3, 0);

      kqj = get_image(portrait[face-10], w2, h2/2, GI_NOT_BIGGER);
      if (!kqj)
	kqj = get_image(portrait[face-10], w2, h2, GI_NOT_BIGGER);

      if (simg)
	{
	  put_subimage (simg, 0, suit, rv, wm+2, hm+2, 0);
	  put_subimage (simg, 0, suit, rv, width-wm-1-simg->width, height-hm-1-simg->height/4, PUT_ROTATED);
	}

      if (kqj && kqj->height <= h2/2)
	{
	  put_subimage (kqj, 0, 0, rv, width-wm-kqj->width, height/2-kqj->height, 0);
	  put_subimage (kqj, 0, 0, rv, wm+1, (height+1)/2, PUT_ROTATED);
	}
      else if (kqj && kqj->height <= h2/2+3)
	{
	  put_subimage (kqj, 0, 0, rv, width-wm-kqj->width, hm+1, 0);
	  put_subimage (kqj, 0, 0, rv, wm+1, height-hm-kqj->height, PUT_ROTATED);
	}
      else if (kqj)
	{
	  put_subimage (kqj, 0, 0, rv, (width+1-kqj->width)/2, (height+1-kqj->height)/2, 0);
	}
    }

  fill_image (rv, 0, 0, width, 1, 0, 0, 0);
  fill_image (rv, 0, 0, 1, height, 0, 0, 0);
  fill_image (rv, 0, height-1, width, 1, 0, 0, 0);
  fill_image (rv, width-1, 0, 1, height, 0, 0, 0);

  put_subimage (face_img, color, face, rv, 1, 2, 0);
  subw = face_img->width / face_img->list->across;
  subh = face_img->height / face_img->list->down;
  if (width > subw*2+4)
    put_subimage (face_img, color, face, rv, width-1-subw, height-2-subh, PUT_ROTATED);

  img = get_image("suits", subw-2, (subw-2)*4, GI_NOT_BIGGER);
  put_subimage (img, 0, suit, rv, 1+subw/2-img->width/2, 4+subh, 0);
  if (width > subw*2+4)
    put_subimage (img, 0, suit, rv,
		  width-1-subw/2-img->width/2, height-4-subh-img->height/img->list->down,
		  PUT_ROTATED);
}

static image *
card_synth(image_list *list, int type, int width, int height)
{
  image *rv;
  static int minw=0, minh=0;

  for (rv = list->subimage[type]; rv; rv=rv->next)
    if (rv->width == width && rv->height == height)
      return rv;

  if (minw == 0)
    {
      image *val, *suit;
      int face_w = width*2/11;
      val = get_image("a-k", face_w*2, face_w*13, 0);
      suit = get_image("suits", 9, 9*4, 0);
      minw = val->width / val->list->across + 2;
      minh = val->height / val->list->down + suit->height / suit->list->down + 6;
    }

  if (width < minw)
    width = minw;
  if (height < minh)
    height = minh;

  rv = alloc_synth_image(list, width, height, type);
  rv->synth_func = card_synth2;
  return rv;
}

static void
empty_synth2(image *img)
{
  fill_image (img, 0, 0, img->width, img->height, 0, 102, 0);
  fill_image (img, 0, img->height-1, img->width, 1, 0, 0, 0);
  fill_image (img, img->width-1, 0, 1, img->height, 0, 0, 0);
  fill_image (img, 0, 0, img->width, 1, 0, 204, 0);
  fill_image (img, 0, 0, 1, img->height, 0, 204, 0);
}

static image *
empty_synth(image_list *list, int type, int width, int height)
{
  image *rv, *img;

  for (rv = list->subimage[type]; rv; rv=rv->next)
    if (rv->width == width && rv->height == height)
      return rv;

  rv = alloc_synth_image(list, width, height, type);
  rv->synth_func = empty_synth2;
  return rv;
}

static void
back_synth2(image *img)
{
  image *tile = get_image ("back-tile", 1, 1, 0);
  int xo, yo, size;
  int x, y, l, r;
  int xs[6];
  int x01, x23, x24, x25;

  for (x=0; x<img->width; x += tile->width)
    for (y=0; y<img->height; y += tile->height)
      put_image (tile, 0, 0, tile->width, tile->height, img, x, y, 0);
  fill_image (img, 0, img->height-1, img->width, 1, 0, 0, 0);
  fill_image (img, img->width-1, 0, 1, img->height, 0, 0, 0);
  fill_image (img, 0, 0, img->width, 1, 0, 0, 0);
  fill_image (img, 0, 0, 1, img->height, 0, 0, 0);

  if (img->width < img->height)
    size = img->width * 2/3;
  else
    size = img->height * 2/3;
  xo = (img->width - size) / 2;
  yo = (img->height - size) / 2;

  x = (size-1) * 203 / 256;
  x25 = size - 1 - x;
  x23 = x25 * 105 / 256;
  x24 = x25 - x23;

  x = (size-1) * 200 / 256;
  x01 = size - 1 - x;

  for (y=0; y<size; y++)
    {
      xs[0] = y * 192/256;
      xs[1] = xs[0] + x01;
      xs[2] = (size-1-y) * 203/256;
      xs[3] = xs[2] + x23;
      xs[4] = xs[2] + x24;
      xs[5] = xs[2] + x25;

      l = (xs[0] < xs[2]) ? xs[0] : xs[2];
      r = (xs[1] < xs[3]) ? xs[1] : xs[3];
      fill_image (img, l+xo, y+yo, r-l+1, 1, 0, 0, 0);

      l = (xs[0] > xs[4]) ? xs[0] : xs[4];
      r = (xs[1] > xs[5]) ? xs[1] : xs[5];
      fill_image (img, l+xo, y+yo, r-l+1, 1, 0, 0, 0);
    }

}

static image *
back_synth(image_list *list, int type, int width, int height)
{
  image *rv, *img;

  for (rv = list->subimage[type]; rv; rv=rv->next)
    if (rv->width == width && rv->height == height)
      return rv;

  rv = alloc_synth_image(list, width, height, type);
  rv->synth_func = back_synth2;
  return rv;
}

static image_list card_images[] = {
  { "ac", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "ad", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "as", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "ah", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "2c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "2d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "2s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "2h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "3c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "3d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "3s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "3h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "4c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "4d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "4s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "4h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "5c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "5d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "5s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "5h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "6c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "6d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "6s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "6h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "7c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "7d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "7s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "7h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "8c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "8d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "8s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "8h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "9c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "9d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "9s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "9h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "0c", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "0d", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "0s", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "0h", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "jc", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "jd", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "js", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "jh", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "qc", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "qd", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "qs", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "qh", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "kc", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "kd", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "ks", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "kh", 1, 1, {0,0,0}, 0, card_synth, 0 },
  { "empty", 1, 1, {0,0,0}, 0, empty_synth, 0 },
  { "back", 1, 1, {0,0,0}, 0, back_synth, 0 },
  { 0 }
};

