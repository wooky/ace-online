/* The Ace of Penguins - table.c
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
#include <string.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

static char AOP[] = "The Ace of Penguins - ";

#define CD printf("%d: %d %d %d %d\n", __LINE__, ex, ey, ew, eh)
#undef CD
#define CD

#include "cards.h"

#define TRACE_UNCOMPRESS	0
#define TRACE_EVENTS		0
#define TRACE_PICTURES		0
#define TRACE_INVALIDATE	0

#define DBLCLICK_TIME		500
#define DBLCLICK_MOVE		5

Display *display=0;
int screen=0;
Visual *visual=0;
Colormap cmap=0;
Window window=0;
Window rootwin=0;
GC gc=0;
XFontStruct *font;
int font_width, font_height;

static Atom wm_protocols_atom=0;
static Atom delete_atom=0;
static Atom paste_atom=0;
static Atom mwm_atom=0;
static XEvent event;
static XRectangle clip_rect;

static Picture *centered_pic = 0;

/* Motif window hints */
typedef struct
{
  unsigned flags;
  unsigned functions;
  unsigned decorations;
  int inputMode;
} PropMotifWmHints;

typedef PropMotifWmHints        PropMwmHints;

/* Motif window hints */
#define MWM_HINTS_FUNCTIONS           (1L << 0)
#define MWM_HINTS_DECORATIONS         (1L << 1)

/* bit definitions for MwmHints.functions */
#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)       

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL                 (1L << 0)
#define MWM_DECOR_BORDER              (1L << 1)
#define MWM_DECOR_RESIZEH             (1L << 2)
#define MWM_DECOR_TITLE               (1L << 3)
#define MWM_DECOR_MENU                (1L << 4)
#define MWM_DECOR_MINIMIZE            (1L << 5)
#define MWM_DECOR_MAXIMIZE            (1L << 6)

#define PROP_MOTIF_WM_HINTS_ELEMENTS  4
#define PROP_MWM_HINTS_ELEMENTS       PROP_MOTIF_WM_HINTS_ELEMENTS


int table_background;
int help_background;

int table_width, table_height;

static int ex=0, ey=0, ew=0, eh=0;

static int graphics_disabled = 1;

typedef struct ImageTable {
  char *name;
  char **xpm;
  Pixmap pixmap;
  Pixmap mask;
} ImageTable;
ImageTable *image_table=0;

static char **string_table;

extern int imglib_num_images;
extern int imglib_data_size;
extern int imglib_num_strings;
extern unsigned char imglib_compressed[];
extern int imglib_compressed_size;

extern int appimglib_num_images;
extern int appimglib_data_size;
extern int appimglib_num_strings;
extern unsigned char appimglib_compressed[];
extern int appimglib_compressed_size;

static void
uncompress_data(unsigned char *cdata, int clen, char *udata, int ulen)
{
  int bpv=1, mo=3, cutoff=0x100;
  int i, t, l, p;
  char *dp;

  dp = udata;
  for (i=0, dp=udata; i<clen;)
  {
    if (dp-udata>=cutoff)
    {
      bpv ++;
      mo = 1+2*bpv;
      cutoff <<= 8;
    }

    assert(dp < udata+ulen);
    if (cdata[i] == 0x80)
    {
      int mult = 0;
      i++;
#if TRACE_UNCOMPRESS
      printf("match i=%d: p = ", i);
#endif
      p = 0;
      for (t=0; t<bpv; t++)
      {
#if TRACE_UNCOMPRESS
	printf("%02x", cdata[i]);
#endif
	p |= cdata[i++] << mult;
	mult += 8;
      }
      mult = 0;
#if TRACE_UNCOMPRESS
      printf("  l = ");
#endif
      l = 0;
      for (t=0; t<bpv; t++)
      {
#if TRACE_UNCOMPRESS
	printf("%02x", cdata[i]);
#endif
	l |= cdata[i++] << mult;
	mult += 8;
      }
#if TRACE_UNCOMPRESS
      printf("\n");
#endif
      assert(p<(dp-udata) && p>=0);
      assert(l<=ulen-(dp-udata));
      memcpy(dp, udata+p, l);
      dp += l;
    }
    else if (cdata[i] & 0x80)
    {
      l = cdata[i++] & 0x7f;
      t = cdata[i++] & 0x7f;
#if TRACE_UNCOMPRESS
      printf("run %d %02x\n", l, t);
#endif
      assert(dp+l < udata+ulen);
      while (l--)
	*dp++ = t;
    }
    else
    {
#if TRACE_UNCOMPRESS
      printf("char %02x\n", cdata[i]);
#endif
      *dp++ = cdata[i++];
    }
  }
  *dp++ = 0;
#if 0
  printf("%s\n", udata);
  assert(1==2);
#endif
}

static void
uncompress_images()
{
  int i, t;
  char *data = (char *)malloc(imglib_data_size+appimglib_data_size+1), *dp;

  uncompress_data(imglib_compressed, imglib_compressed_size,
		  data, imglib_data_size);
  if (appimglib_data_size)
    uncompress_data(appimglib_compressed, appimglib_compressed_size,
		    data+imglib_data_size, appimglib_data_size);

  string_table = (char **)malloc((imglib_num_strings+appimglib_num_strings)
				 *sizeof(char *));
  string_table[0] = data;
  for (dp=data, i=1; *dp && i<imglib_num_strings+appimglib_num_strings; dp++)
    if (*dp == '\n')
    {
      *dp++ = 0;
      string_table[i++] = dp;
    }

  image_table = (ImageTable *)malloc((imglib_num_images+appimglib_num_images+1)
				     *sizeof(ImageTable));
  for (t=0, i=0; i<imglib_num_strings+appimglib_num_strings;)
  {
    int w, h, nc;
    image_table[t].name = string_table[i++];
    image_table[t].xpm = string_table+i;
    image_table[t].pixmap = 0;
    image_table[t].mask = 0;
    sscanf(string_table[i], "%d %d %d", &w, &h, &nc);
    i += 1 + nc + h;
    t++;
  }
  image_table[t].name = 0;
}

void
init_table(int argc, char **argv, int width, int height)
{
  char *name = argv[0];
  char *sl;
  XSizeHints size_hints;
  XTextProperty xtp;
  XSetWindowAttributes attributes;
  XColor color;
  PropMwmHints mwm_hints;
  
  sl = strrchr(name, '/');
  if (sl) name = sl+1;

  ew = table_width = width;
  eh = table_height = height;
  CD;

  display = XOpenDisplay(0);
  screen = XDefaultScreen(display);
  cmap = XDefaultColormap(display, screen);
  visual = XDefaultVisual(display, screen);
  rootwin = XDefaultRootWindow(display);
  gc = XCreateGC(display, rootwin, 0, 0);

  wm_protocols_atom = XInternAtom(display, "WM_PROTOCOLS", 0);
  delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", 0);
  paste_atom = XInternAtom(display, "PASTE_DATA", 0);
  mwm_atom = XInternAtom(display, "_MOTIF_WM_HINTS", 0);

  size_hints.flags = PSize|PMinSize|PMaxSize;
  size_hints.width = width;
  size_hints.height = height;
  size_hints.x = 0;
  size_hints.y = 0;
  size_hints.min_width = width;
  size_hints.min_height = height;
  size_hints.max_width = width;
  size_hints.max_height = height;

  window = XCreateWindow(display,
			 rootwin,
			 size_hints.x, size_hints.y,
			 size_hints.width, size_hints.height,
			 0,
			 CopyFromParent, /* depth */
			 InputOutput,
			 CopyFromParent, /* visual */
			 0, 0);

  XSetWMNormalHints(display, window, &size_hints);

  sl = (char *)malloc(strlen(name) + strlen(AOP)+1);
  sprintf(sl, "%s%s", AOP, name);
  XStringListToTextProperty(&sl, 1, &xtp);
  XSetWMName(display, window, &xtp);
  XFree(xtp.value);

  XSetWMProtocols(display, window, &delete_atom, 1);

  attributes.event_mask = (  ExposureMask
			   | ButtonPressMask
			   | ButtonReleaseMask
			   | ButtonMotionMask
			   | KeyPressMask
			   | PointerMotionHintMask );
  XChangeWindowAttributes(display, window, CWEventMask, &attributes);

  mwm_hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
  mwm_hints.functions = MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE | MWM_FUNC_CLOSE;
  mwm_hints.decorations = MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU | MWM_DECOR_MINIMIZE;
  XChangeProperty(display, window, mwm_atom, mwm_atom, 32, PropModeReplace,
		  (char *)&mwm_hints, PROP_MWM_HINTS_ELEMENTS);

  XMapWindow(display, window);
  XFlush(display);

  color.red   = 0x0000;
  color.green = 0x6666;
  color.blue  = 0x0000;
  color.flags = DoRed | DoGreen | DoBlue;
  if (!XAllocColor(display, cmap, &color))
  {
    color.red   = 0x0000;
    color.green = 0x8000;
    color.blue  = 0x0000;
    color.flags = DoRed | DoGreen | DoBlue;
    if (!XAllocColor(display, cmap, &color))
    {
      color.pixel = BlackPixel(display, screen);
    }
  }
  table_background = color.pixel;

  font = XLoadQueryFont(display, "6x13bold");
  if (!font) font = XLoadQueryFont(display, "6x10");
  if (!font) font = XLoadQueryFont(display, "fixed");
  font_width = font->max_bounds.width;
  font_height = font->ascent + font->descent;
}

void
flush()
{
  XFlush(display);
}

void
beep()
{
  XBell(display, 0);
}

typedef struct PicRec {
  Pixmap pixmap;
  Pixmap mask;
  char **xpm_data;
  int image_table_index;
} PicRec;

Picture *
get_picture(char *name)
{
  int i;
  if (image_table == 0)
    uncompress_images();
  for (i=0; image_table[i].name; i++)
    if (strcmp(image_table[i].name, name) == 0)
    {
      Picture *p;
      p = (Picture *)malloc(sizeof(Picture));
      p->pixmap = (void *)malloc(sizeof(PicRec));
      ((PicRec *)(p->pixmap))->pixmap = 0;
      ((PicRec *)(p->pixmap))->mask = 0;
      ((PicRec *)(p->pixmap))->xpm_data = image_table[i].xpm;
      sscanf(image_table[i].xpm[0], "%d %d", &(p->w), &(p->h));
      if (image_table[i].pixmap)
      {
	((PicRec *)(p->pixmap))->pixmap = image_table[i].pixmap;
	((PicRec *)(p->pixmap))->mask = image_table[i].mask;
      }
      ((PicRec *)(p->pixmap))->image_table_index = i;
      return p;
    }
  printf("can't find image `%s'\n", name);
  return 0;
}

static void
get_pixmap(PicRec *pr)
{
  if (image_table[pr->image_table_index].pixmap)
  {
    pr->pixmap = image_table[pr->image_table_index].pixmap;
    pr->mask = image_table[pr->image_table_index].mask;
  }
  else
  {
    XpmAttributes attr;
    attr.valuemask = XpmSize | XpmExactColors | XpmCloseness;
    attr.exactColors = 0;
    attr.closeness = 32769;
    XpmCreatePixmapFromData(display, window, pr->xpm_data,
			    &(pr->pixmap), &(pr->mask), &attr);
    image_table[pr->image_table_index].pixmap = pr->pixmap;
    image_table[pr->image_table_index].mask = pr->mask;
  }
}

void
put_picture(Picture *picture, int dx, int dy,
	    int x, int y, int w, int h)
{
  PicRec *pr = (PicRec *)(picture->pixmap);
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
    if (pr->pixmap == 0)
      get_pixmap(pr);
    if (pr->mask)
    {
      XSetClipMask(display, gc, pr->mask);
      XSetClipOrigin(display, gc, dx, dy);
    }
    if (pr->pixmap) /* safety! */
      XCopyArea(display, pr->pixmap, window, gc, x, y, w, h, dx+x, dy+y);
    if (pr->mask)
      XSetClipRectangles(display, gc, 0, 0, &clip_rect, 1, YXBanded);
  }
}

void
put_picture_inverted(Picture *picture, int dx, int dy,
		     int x, int y, int w, int h)
{
  static Picture *inverted_picture = 0;
  static Pixmap inverted_pixmap=0;
  static ipw=0, iph=0;
  Pixmap save_pixmap;
  PicRec *pr = (PicRec *)(picture->pixmap);
  if (graphics_disabled) return;

  if (!pr->pixmap)
    get_pixmap(pr);

  if (picture != inverted_picture)
  {
    int x, y;
    XImage *img;
    if (inverted_pixmap == 0 || ipw < picture->w || iph < picture->h)
    {
      if (inverted_pixmap)
	XFreePixmap(display, inverted_pixmap);
      if (ipw < picture->w) ipw = picture->w;
      if (iph < picture->h) iph = picture->h;
      inverted_pixmap = XCreatePixmap(display, pr->pixmap, ipw, iph,
				      DefaultDepth(display, screen));
    }
    if (pr->mask)
      XSetClipMask(display, gc, None);
    img = XGetImage(display, pr->pixmap,
		    0, 0, picture->w, picture->h, -1, ZPixmap);
    for (x=0; x<picture->w; x++)
      for (y=0; y<picture->h; y++)
      {
	int p = XGetPixel(img, x, y);
	if (img->depth >= 24)
	{
	  p = ~p & 0xffffff;
	}
	else
	{
	  if (p == WhitePixel(display, screen))
	    p = BlackPixel(display, screen);
	  else if (p == BlackPixel(display, screen))
	    p = WhitePixel(display, screen);
	}
	XPutPixel(img, x, y, p);
      }
    XPutImage(display, inverted_pixmap, gc, img,
	      0, 0, 0, 0, picture->w, picture->h);
    XDestroyImage(img);
    inverted_picture = picture;
  }
  save_pixmap = pr->pixmap;
  pr->pixmap = inverted_pixmap;
  put_picture(picture, dx, dy, x, y, w, h);
  pr->pixmap = save_pixmap;
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

#define CLEAR_CLIP	clip_rect.x = ex = 0; clip_rect.y = ey = 0; clip_rect.width = ew = table_width; clip_rect.height = eh = table_height;

#define ButtonMask (Button1Mask | Button2Mask | Button3Mask)

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

void
table_loop()
{
  int root_x, root_y, pos_x, pos_y;
  unsigned int keys_buttons;
  int i;
  Window root, child;
  KeySym keysym;
  char c;
  int click_button;
  int initted = 0;

  while (1)
  {
    XNextEvent(display, &event);
    if (event.xany.window == window)
    {
      if (!initted && event.type != Expose)
	continue;

      switch (event.type)
      {
      case Expose:
#if TRACE_EVENTS
	printf("expose: x=%3d y=%3d w=%3d h=%3d c=%d\n",
	       ex, ey, ew, eh, event.xexpose.count);
#endif
	clip_rect.x = ex = event.xexpose.x;
	clip_rect.y = ey = event.xexpose.y;
	clip_rect.width = ew = event.xexpose.width;
	clip_rect.height = eh = event.xexpose.height;
	CD;
	XSetClipRectangles(display, gc, 0, 0, &clip_rect, 1, YXBanded);
	clear(ex, ey, ew, eh);

	if (! initted)
	{
	  initted = 1;
	  XFlush(display);
	  graphics_disabled = 1;
	  init();
	  graphics_disabled = 0;
#if TRACE_EVENTS
	  printf(" - done init\n");
#endif
	}
	ex = event.xexpose.x;
	ey = event.xexpose.y;
	ew = event.xexpose.width;
	eh = event.xexpose.height;
	CD;
	if (help_is_showing)
	  help_redraw();
	else
	  redraw();
	redraw_centered_pic();
	XSetClipMask(display, gc, None);
	clip_rect.x = 0;
	clip_rect.y = 0;
	clip_rect.width = table_width;
	clip_rect.height = table_height;
#if TRACE_EVENTS
	printf(" - done expose\n");
#endif
	break;

      case ButtonPress:
	CLEAR_CLIP;
	CD;
#if TRACE_EVENTS
	printf("click: %d,%d %d\n",
	       event.xbutton.x, event.xbutton.y, event.xbutton.button);
#endif
	click_button = event.xbutton.button;
	check_dclick(event.xbutton.x, event.xbutton.y, event.xbutton.time);
	if (help_is_showing)
	{
	  help_click(event.xbutton.x, event.xbutton.y, click_button);
	}
	else if (event.xbutton.time - dct < DBLCLICK_TIME)
	{
	  double_click(event.xbutton.x, event.xbutton.y, event.xbutton.button);
	  dct -= DBLCLICK_TIME;
	}
	else
	{
	  click(event.xbutton.x, event.xbutton.y, click_button);
	  dcx = event.xbutton.x;
	  dcy = event.xbutton.y;
	  dct = event.xbutton.time;
	}
	drag_enabled = 0;
	break;

      case MotionNotify:
	CLEAR_CLIP;
	CD;
	check_dclick(event.xmotion.x, event.xmotion.y, event.xmotion.time);
	while (XCheckMaskEvent(display, ButtonMotionMask, &event));
	if (!XQueryPointer(display, event.xmotion.window,
			   &root, &child, &root_x, &root_y,
			   &pos_x, &pos_y, &keys_buttons))
	  break;
#if TRACE_EVENTS
	printf("drag: %d,%d\n", pos_x, pos_y);
#endif
	if (drag_enabled && !help_is_showing)
	  drag(pos_x, pos_y, click_button);
	break;

      case ButtonRelease:
	CLEAR_CLIP;
	check_dclick(event.xbutton.x, event.xbutton.y, event.xbutton.time);
	CD;
	i = event.xbutton.state & ButtonMask;
	if ((i & (i>>1)) == 0)
	{
#if TRACE_EVENTS
	  printf("drop: %d,%d\n", event.xbutton.x, event.xbutton.y);
#endif
	  if (!help_is_showing)
	    drop(event.xbutton.x, event.xbutton.y, click_button);
	}
	break;

      case KeyPress:
	CLEAR_CLIP;
	CD;
	if (XLookupString(&event.xkey, &c, 1, &keysym, 0) == 1)
	  i = c;
	else
	  i = keysym;
	if (help_is_showing)
	  help_key(i, event.xkey.x, event.xkey.y);
	else
	  key(i, event.xkey.x, event.xkey.y);
	break;

      case ClientMessage:
	if (event.xclient.message_type == wm_protocols_atom
	    && event.xclient.data.l[0] == delete_atom)
	    exit(0);
	break;
      }
    }
  }
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
  CD;
}

void
clear(int x, int y, int w, int h)
{
  XSetForeground(display, gc, help_is_showing ?
		 table_background : table_background);
  XFillRectangle(display, window, gc, x, y, w, h);
}

static void
reset_clip()
{
  clip_rect.x = ex;
  clip_rect.y = ey;
  clip_rect.width = ew;
  clip_rect.height = eh;
  XSetClipRectangles(display, gc, 0, 0, &clip_rect, 1, YXBanded);
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
    redraw();
  redraw_centered_pic();
  XSetClipMask(display, gc, None);
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
    redraw();
  redraw_centered_pic();
  XSetClipMask(display, gc, None);
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

void
text(char *t, int x, int y)
{
  XSetBackground(display, gc, table_background);
  XSetForeground(display, gc, WhitePixel(display, screen));
  if (font) XSetFont(display, gc, font->fid);
  XDrawImageString(display, window, gc, x, y-font->descent, t, strlen(t));
}
