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
#include <string.h>
#include <assert.h>
#include <math.h>

#include <png.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

typedef struct {
  Pixmap image_pixmap;
  Pixmap image_mask;
  Pixmap rotated_pixmap;
  Pixmap rotated_mask;
  Pixmap inverted_pixmap;
} pixels_type;
#define PIXELS_TYPE pixels_type *

#define ROT(a,b) if (xrotate) { int t = a; a = b; b = t; }

int pixel_for (int r, int g, int b);

#include "cards.h"
#include "imagelib.h"
#include "xwin.h"

static char AOP[] = "The Ace of Penguins - ";
static char *type_names[] = {"mono", "grey", "color"};

int xrotate = 0;
int visual_id = 0;
OptionDesc xwin_options_list[] = {
  { "-rotate", OPTION_BOOLEAN, &xrotate },
  { "-visual", OPTION_INTEGER, &visual_id },
  { 0, 0, 0 }
};
OptionDesc *xwin_options = xwin_options_list;

Display *display=0;
int screen=0;
Visual *visual=0;
Colormap cmap=0;
Window window=0;
Window rootwin=0;
GC gc=0, imggc=0, maskgc=0;
XFontStruct *font;
int font_width, font_height;
XVisualInfo vi, *vip;

static int broken_xserver = 0;

static image_list static_display_image_list;
static image static_display_image;
image *display_image;

static Atom wm_protocols_atom=0;
static Atom delete_atom=0;
static Atom paste_atom=0;
static Atom mwm_atom=0;
static XEvent event;
static XRectangle clip_rect;

int table_background;
int help_background;

#define ButtonMask (Button1Mask | Button2Mask | Button3Mask)

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


void break_here(){}

static char *name;

void
xwin_init(int argc, char **argv)
{
  char *sl;
  XColor color;
  int i;

  name = argv[0];

  atexit(break_here);
  
  sl = strrchr(name, '/');
  if (sl) name = sl+1;

  display = XOpenDisplay(0);
  screen = XDefaultScreen(display);
  rootwin = XDefaultRootWindow(display);

  /* The Agenda's X server can't use a bitmap for a mask.  If/when it
     gets fixed, find out the version and update the test below. */
  if (strcmp(XServerVendor(display), "Keith Packard") == 0)
    broken_xserver = 1;

  visual = XDefaultVisual(display, screen);
  vi.visualid = XVisualIDFromVisual(visual);
  if (visual_id)
    vi.visualid = visual_id;
  vip = XGetVisualInfo (display, VisualIDMask, &vi, &i);
  if (i != 1)
    abort();
  visual = vip->visual;

  if (visual_id)
    {
      cmap = XCreateColormap(display, rootwin, visual, AllocNone);
    }
  else
    cmap = XDefaultColormap(display, screen);

  gc = XCreateGC(display, rootwin, 0, 0);
  imggc = XCreateGC(display, rootwin, 0, 0);

  _Xdebug=999;

  display_width = DisplayWidth(display, screen);
  display_height = DisplayHeight(display, screen);
  ROT(display_width, display_height);
  
  switch (vip->class)
    {
    case StaticGray:
    case GrayScale:
      if (vip->depth == 1)
	table_type = TABLE_MONO;
      else
	table_type = TABLE_GRAY;
      break;
    case StaticColor:
    case PseudoColor:
    case TrueColor:
    case DirectColor:
      table_type = TABLE_COLOR;
      break;
    }

  if (vip->class == DirectColor)
    {
      int i, scale = 0xffff / ((1<<vip->depth)-1);
      int step = 1 << (vip->depth - vip->bits_per_rgb);
      XColor c;
      for (i=0; i < (1<<vip->depth); i += step)
	{
	  c.red = c.blue = c.green = i * scale;
	  c.pixel = i;
	  XStoreColor(display, cmap, &c);
	}
    }

  wm_protocols_atom = XInternAtom(display, "WM_PROTOCOLS", 0);
  delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", 0);
  paste_atom = XInternAtom(display, "PASTE_DATA", 0);
  mwm_atom = XInternAtom(display, "_MOTIF_WM_HINTS", 0);

  table_background = pixel_for(0x00, 0x66, 0x00);

  font = XLoadQueryFont(display, "6x13bold");
  if (!font) font = XLoadQueryFont(display, "6x10");
  if (!font) font = XLoadQueryFont(display, "fixed");
  font_width = font->max_bounds.width;
  font_height = font->ascent + font->descent;
}

void
xwin_create(int width, int height)
{
  char *sl;
  XSizeHints size_hints;
  XTextProperty xtp;
  XSetWindowAttributes attributes;

  ROT(width, height);

  size_hints.flags = PSize;
  size_hints.width = width;
  size_hints.height = height;
  size_hints.x = 0;
  size_hints.y = 0;
#if 0
  size_hints.min_width = width;
  size_hints.min_height = height;
  size_hints.max_width = width;
  size_hints.max_height = height;
#endif

  attributes.colormap = cmap;
  window = XCreateWindow(display,
			 rootwin,
			 size_hints.x, size_hints.y,
			 size_hints.width, size_hints.height,
			 0,
			 vip->depth,
			 InputOutput,
			 visual,
			 CWColormap, &attributes);

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
			   | StructureNotifyMask
			   | PointerMotionHintMask );
  XChangeWindowAttributes(display, window, CWEventMask, &attributes);

  display_image = &static_display_image;
  if (xrotate)
    {
      static_display_image.width = height;
      static_display_image.height = width;
    }
  else
    {
      static_display_image.width = width;
      static_display_image.height = height;
    }
  static_display_image.list = &static_display_image_list;
  static_display_image.pixels = (pixels_type *) malloc (sizeof(pixels_type));
  static_display_image.pixels->image_pixmap = (Pixmap)window;
  static_display_image.pixels->image_mask = 0;

  static_display_image_list.name = "X Window";
  static_display_image_list.across = 1;
  static_display_image_list.down = 1;

  XMapWindow(display, window);
  XFlush(display);
}

void
xwin_fixed_size (int width, int height)
{
  XSizeHints size_hints;
  PropMwmHints mwm_hints;

  ROT(width, height);

  XResizeWindow(display, window, width, height);

  size_hints.flags = PMinSize|PMaxSize;
  size_hints.min_width = width;
  size_hints.min_height = height;
  size_hints.max_width = width;
  size_hints.max_height = height;
  XSetWMNormalHints(display, window, &size_hints);

  mwm_hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
  mwm_hints.functions = MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE | MWM_FUNC_CLOSE;
  mwm_hints.decorations = MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU | MWM_DECOR_MINIMIZE;
  XChangeProperty(display, window, mwm_atom, mwm_atom, 32, PropModeReplace,
		  (char *)&mwm_hints, PROP_MWM_HINTS_ELEMENTS);
}

static struct {
  KeySym sym;
  int key;
} key_mappings[] = {
  { XK_F1,		KEY_F(1) },
  { XK_F2,		KEY_F(2) },
  { XK_F3,		KEY_F(3) },
  { XK_F4,		KEY_F(4) },
  { XK_F5,		KEY_F(5) },
  { XK_F6,		KEY_F(6) },
  { XK_F7,		KEY_F(7) },
  { XK_F8,		KEY_F(8) },
  { XK_F9,		KEY_F(9) },
  { XK_F10,		KEY_F(10) },
  { XK_F11,		KEY_F(11) },
  { XK_F12,		KEY_F(12) },
  { XK_Delete,		KEY_DELETE },
  { XK_KP_Delete,	KEY_DELETE },
  { XK_BackSpace,	KEY_DELETE },
  { XK_Up,		KEY_UP },
  { XK_KP_Up,		KEY_UP },
  { XK_Down,		KEY_DOWN },
  { XK_KP_Down,		KEY_DOWN },
  { XK_Left,		KEY_LEFT },
  { XK_KP_Left,		KEY_LEFT },
  { XK_Right,		KEY_RIGHT },
  { XK_KP_Right,	KEY_RIGHT },
  { XK_Page_Up,		KEY_PGUP },
  { XK_KP_Page_Up,	KEY_PGUP },
  { XK_Page_Down,	KEY_PGDN },
  { XK_KP_Page_Down,	KEY_PGDN },
  { XK_Home,		KEY_HOME },
  { XK_KP_Home,		KEY_HOME }
};
#define NUM_KEYMAPS (sizeof(key_mappings)/sizeof(key_mappings[0]))

#define WROT(x,y,w,h) if (xrotate) { int t = x; x = table_width-y-h; y = t; \
				     t = w; w = h; h = t; }
#define PROT(x,y) if (xrotate) { int t = x; x = table_width-y; y = t; }
#define WROT2(x,y,w,h) if (xrotate) { int t = y; y = table_width-x-w; x = t; \
				     t = w; w = h; h = t; }
#define PROT2(x,y) if (xrotate) { int t = y; y = table_width-x; x = t; }

int
xwin_nextevent (XWin_Event *ev)
{
  int root_x, root_y, pos_x, pos_y;
  unsigned int keys_buttons;
  int i;
  Window root, child;
  KeySym keysym;
  char c;
  static int click_button;
  static int last_resize_w=-1, last_resize_h=-1;

  while (1)
  {
    XNextEvent(display, &event);
    if (event.xany.window == window)
    {
      switch (event.type)
      {
      case ConfigureNotify:
	ev->type = ev_resize;
	WROT (event.xconfigure.x, event.xconfigure.y,
	      event.xconfigure.width, event.xconfigure.height);
	ev->x = event.xconfigure.x;
	ev->y = event.xconfigure.y;
	ev->w = event.xconfigure.width;
	ev->h = event.xconfigure.height;
	if (ev->w == last_resize_w && ev->h == last_resize_h)
	  break;
	last_resize_w = ev->w;
	last_resize_h = ev->h;
	return ev_resize;
	
      case Expose:
	ev->type = ev_expose;
	WROT (event.xexpose.x, event.xexpose.y,
	      event.xexpose.width, event.xexpose.height);
	ev->x = event.xexpose.x;
	ev->y = event.xexpose.y;
	ev->w = event.xexpose.width;
	ev->h = event.xexpose.height;
	return ev_expose;

      case ButtonPress:
	ev->type = ev_buttondown;
	PROT(event.xbutton.x, event.xbutton.y);
	ev->x = event.xbutton.x;
	ev->y = event.xbutton.y;
	click_button = event.xbutton.button;
	if (event.xbutton.state & ShiftMask)
	  click_button ++;
	ev->button = click_button;
	ev->time = event.xbutton.time;
	ev->shifts = 0;
	return ev_buttondown;

      case MotionNotify:

	while (XCheckMaskEvent(display, ButtonMotionMask, &event));
	if (!XQueryPointer(display, event.xmotion.window,
			   &root, &child, &root_x, &root_y,
			   &pos_x, &pos_y, &keys_buttons))
	  break;
	ev->type = ev_motion;
	PROT(pos_x, pos_y);
	ev->x = pos_x;
	ev->y = pos_y;
	ev->button = click_button;
	ev->time = event.xmotion.time;
	return ev_motion;

      case ButtonRelease:
	i = event.xbutton.state & ButtonMask;
	if ((i & (i>>1)) == 0)
	  {
	    ev->type = ev_buttonup;
	    PROT(event.xbutton.x, event.xbutton.y);
	    ev->x = event.xbutton.x;
	    ev->y = event.xbutton.y;
	    ev->button = click_button;
	    ev->time = event.xbutton.time;
	    ev->shifts = 0;
	    return ev_buttonup;
	  }
	break;

      case KeyPress:

	ev->key = 0;
	if (XLookupString(&event.xkey, &c, 1, &keysym, 0) == 1)
	  ev->key = c;
	else
	  for (i=0; i<NUM_KEYMAPS; i++)
	    if (key_mappings[i].sym == keysym)
	      ev->key = key_mappings[i].key;
	if (!ev->key)
	  break;

	ev->type = ev_keypress;
	PROT(event.xkey.x, event.xkey.y);
	ev->x = event.xkey.x;
	ev->y = event.xkey.y;
	ev->time = event.xkey.time;
	ev->shifts = 0;
	return ev_keypress;

      case ClientMessage:
	if (event.xclient.message_type == wm_protocols_atom
	    && event.xclient.data.l[0] == delete_atom)
	  {
	    ev->type = ev_quit;
	    return ev_quit;
	  }
	break;
      }
    }
  }
}

void
flush()
{
  XFlush(display);
}

void
flushsync()
{
  XSync(display, False);
}

void
beep()
{
  XBell(display, 0);
}

static int have_clip = 0;

void
xwin_clip (int x, int y, int w, int h)
{
  WROT2(x, y, w, h);
  clip_rect.x = x;
  clip_rect.y = y;
  clip_rect.width = w;
  clip_rect.height = h;
  XSetClipRectangles(display, gc, 0, 0, &clip_rect, 1, YXBanded);
  have_clip = 1;
}

void
xwin_noclip ()
{
  if (!have_clip)
    return;
  have_clip = 0;
  XSetClipMask(display, gc, None);
}

static void
xwin_restore_clip ()
{
  if (have_clip)
    XSetClipRectangles(display, gc, 0, 0, &clip_rect, 1, YXBanded);
  else
    XSetClipMask(display, gc, None);
}

extern int help_is_showing;

void
clear(int x, int y, int w, int h)
{
  WROT2(x, y, w, h);
  XSetForeground(display, gc, help_is_showing ?
		 help_background : table_background);
  XFillRectangle(display, window, gc, x, y, w, h);
}

void
text(char *t, int x, int y)
{
  PROT2(x, y);
  XSetBackground(display, gc, table_background);
  XSetForeground(display, gc, pixel_for(255, 255, 255));
  if (font) XSetFont(display, gc, font->fid);
  XDrawImageString(display, window, gc, x, y-font->descent, t, strlen(t));
}

/************************************************************************/
/*	Image conversion routines					*/
/************************************************************************/

static XImage *exemplar_image = 0;

static void
png_reader (png_structp png_ptr, png_bytep data, png_uint_32 length)
{
  char **bytes = (char **)png_get_io_ptr (png_ptr);
  memcpy (data, *bytes, length);
  *bytes += length;
}

static png_structp png_ptr;
static png_infop info_ptr;
static png_uint_32 width, height;
static int bit_depth, obit_depth, color_type, interlace_type;
static unsigned char *pixel_data;
static XImage *ximage, *xmask;

static int
shift_for (int depth, int mask)
{
  int v = 1 << (depth-1);
  int s = 0;
  while (v < mask)
    {
      v <<= 1;
      s ++;
    }
  while (v > mask)
    {
      v >>= 1;
      s --;
    }
  return s;
}

static int
do_shift_mask (int v, int s, int m)
{
  if (s<0) v >>= -s;
  if (s>0) v <<= s;
  return v & m;
}

static int
do_gamma(int p)
{
  static unsigned char *gamma_table = 0;
  if (gamma_table == 0)
    {
      int i;
      gamma_table = (unsigned char *)malloc(256);
      for (i=0; i<256; i++)
	gamma_table[i] = (unsigned char)(pow((double)i / 255.0, 0.45) * 255.0 + 0.5);
    }
  return gamma_table[p];
}

static int ppixels[6][6][6];
#define NO_PIXEL -2
static int ppixels_initted = 0;

int
pixel_for (int r, int g, int b)
{
  static int rs=0, gs=0, bs=0;
  int p, i;

  if (table_type != TABLE_COLOR)
    {
      p = (r*77+g*150+b*29) >> 8;
      if (vip->class != StaticGray && vip->class != GrayScale)
	p = do_gamma(p);
      r = g = b = p;
    }
  switch (vip->class)
    {
    case TrueColor:
    case DirectColor:
      if (rs == 0)
	{
	  rs = shift_for (8, vip->red_mask);
	  gs = shift_for (8, vip->green_mask);
	  bs = shift_for (8, vip->blue_mask);
	}
      p = 0;
      p |= do_shift_mask (r, rs, vip->red_mask);
      p |= do_shift_mask (g, gs, vip->green_mask);
      p |= do_shift_mask (b, bs, vip->blue_mask);
      return p;

    case StaticGray:
      return (r*77+g*150+b*29) >> (16 - vip->depth);

    case GrayScale:
    case StaticColor:
    case PseudoColor:
      if (!ppixels_initted)
	{
	  for (i=0; i<6*6*6; i++)
	    ((int *)ppixels)[i] = NO_PIXEL;
	  ppixels_initted = 1;
	}
      rs = (r+25) / 51;
      gs = (g+25) / 51;
      bs = (b+25) / 51;
      if (ppixels[rs][gs][bs] == NO_PIXEL)
	{
	  XColor c;
	  c.red = rs * 13107;
	  c.green = gs * 13107;
	  c.blue = bs * 13107;
	  if (XAllocColor (display, cmap, &c))
	    {
	      ppixels[rs][gs][bs] = c.pixel;
	      return c.pixel;
	    }
	}
      return ppixels[rs][gs][bs];
    }
  fprintf(stderr, "Don't know how to make a pixel!\n");
  abort();
}

void
cvt_rgbt (image *img)
{
  int r, g, b, a=255, x, y;
  unsigned char *pp;
  int has_alpha = color_type & PNG_COLOR_MASK_ALPHA;

  pp = pixel_data;
  for (y=0; y<height; y++)
    for (x=0; x<width; x++)
      {
	r = *pp++;
	g = *pp++;
	b = *pp++;
	if (has_alpha)
	  {
	    a = *pp++;
	    if (xrotate)
	      XPutPixel(xmask, y, width-x-1, a > 128 ? 1 : 0);
	    else
	      XPutPixel(xmask, x, y, a > 128 ? 1 : 0);
	  }
	if (xrotate)
	  XPutPixel(ximage, y, width-x-1, pixel_for(r, g, b));
	else
	  XPutPixel(ximage, x, y, pixel_for(r, g, b));
      }
}

void
cvt_cpt (image *img)
{
  int rs, gs, bs, i, x, y;
  unsigned char *pp;
  png_colorp palette;
  png_bytep trans=0;
  int num_palette, num_trans=0;
  unsigned *palette_xcolors;
  png_color_16p trans_color;
  char *istrans;
  /* We have a color palette image, and we need a truecolor image. */

  png_get_PLTE (png_ptr, info_ptr, &palette, &num_palette);
  if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
    png_get_tRNS (png_ptr, info_ptr, &trans, &num_trans, &trans_color);
  istrans = (char *)malloc (num_palette);
  memset(istrans, ~0, num_palette);
  for (i=0; i<num_trans; i++)
    istrans[trans[i]] = 0;
  palette_xcolors = (unsigned *)malloc(num_palette * sizeof(int));
  for (i=0; i<num_palette; i++)
    palette_xcolors[i] = pixel_for(palette[i].red, palette[i].green, palette[i].blue);
  pp = pixel_data;
  for (y=0; y<height; y++)
    for (x=0; x<width; x++)
      {
	int idx = *pp++;
	if (bit_depth > 8) idx = idx*256 + *pp++;
	if (xrotate)
	  {
	    XPutPixel(ximage, y, width-x-1, palette_xcolors[idx]);
	  }
	else
	  XPutPixel(ximage, x, y, palette_xcolors[idx]);
	if (xmask)
	  {
	    if (xrotate)
	      XPutPixel(xmask, y, width-x-1, istrans[idx]);
	    else
	      XPutPixel(xmask, x, y, istrans[idx]);
	  }
      }
}

void
cvt_gt (image *img)
{
  int rs, gs, bs, i, x, y;
  unsigned char *pp;
  /* We have a greyscale image, and we need a truecolor image. */

  pp = pixel_data;
  for (y=0; y<height; y++)
    for (x=0; x<width; x++)
      {
	int idx = *pp++;
	if (bit_depth > 8) idx = *pp++;
	if (obit_depth < bit_depth)
	  idx <<= (bit_depth-obit_depth);
	i = pixel_for (idx, idx, idx);
	if (xrotate)
	  XPutPixel(ximage, y, width-x-1, i);
	else
	  XPutPixel(ximage, x, y, i);
      }
}

struct {
  int png_image_type;
  void (*converter)(image *);
} image_converters[] = {
  { PNG_COLOR_TYPE_RGB, cvt_rgbt },
  { PNG_COLOR_TYPE_RGB_ALPHA, cvt_rgbt },
  { PNG_COLOR_TYPE_PALETTE, cvt_cpt },
  { PNG_COLOR_TYPE_GRAY, cvt_gt },
};
#define NUM_CONVERTERS (sizeof(image_converters)/sizeof(image_converters[0]))

static void
build_image (image *src)
{
  const char *file_bytes;
  unsigned char **row_pointers;
  int i, number_of_passes, num_trans;

  if (!exemplar_image)
    exemplar_image = XGetImage(display, window, 0, 0, 8, 8, ~0, ZPixmap);

  src->pixels = (pixels_type *)malloc(sizeof(pixels_type));
  memset (src->pixels, 0, sizeof(pixels_type));

  if (xrotate)
    src->pixels->image_pixmap = XCreatePixmap (display, window,
					       src->height, src->width,
					       DefaultDepth(display, screen));
  else
    src->pixels->image_pixmap = XCreatePixmap (display, window,
					       src->width, src->height,
					       DefaultDepth(display, screen));

  if (src->synth_func)
    {
      src->synth_func (src);
      return;
    }
  if (src->file_data == 0)
    return;

  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);
  info_ptr = png_create_info_struct (png_ptr);

  if (setjmp (png_ptr->jmpbuf)) {
    fprintf(stderr, "Invalid PNG image!\n");
    return;
  }

  file_bytes = src->file_data;
  png_set_read_fn (png_ptr, (voidp)&file_bytes, (png_rw_ptr)png_reader);

  png_read_info (png_ptr, info_ptr);

  if (interlace_type == PNG_INTERLACE_NONE)
    number_of_passes = 1;
  else
    number_of_passes = png_set_interlace_handling(png_ptr);

  png_get_IHDR (png_ptr, info_ptr, &width, &height, &obit_depth, &color_type, &interlace_type, 0, 0);
#if 0
  fprintf(stderr, "width %d height %d bit_depth %d color_type %s %s %s %s(%s)\n", width, height, obit_depth,
	  color_type & PNG_COLOR_MASK_PALETTE ? "palette" : "true",
	  color_type & PNG_COLOR_MASK_COLOR ? "color" : "grey",
	  color_type & PNG_COLOR_MASK_ALPHA ? "alpha" : "solid",
	  interlace_type == PNG_INTERLACE_NONE ? "" : "interlaced ",
	  src->list->name);
#endif

  if (obit_depth < 8)
    png_set_packing(png_ptr);

  png_read_update_info (png_ptr, info_ptr);
  png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, 0, 0);
#if 0
  fprintf(stderr, "width %d height %d bit_depth %d color_type %s %s %s %s(%s)\n",
	  width, height, bit_depth,
	  color_type & PNG_COLOR_MASK_PALETTE ? "palette" : "true",
	  color_type & PNG_COLOR_MASK_COLOR ? "color" : "grey",
	  color_type & PNG_COLOR_MASK_ALPHA ? "alpha" : "solid",
	  interlace_type == PNG_INTERLACE_NONE ? "" : "interlaced ",
	  src->list->name);
#endif

  pixel_data = (unsigned char *)malloc(height * png_get_rowbytes (png_ptr, info_ptr));
  row_pointers = (unsigned char **)malloc(height*sizeof(unsigned char *));
  
  for (i=0; i<height; i++)
    row_pointers[i] = pixel_data + i * png_get_rowbytes (png_ptr, info_ptr);

  while (number_of_passes--)
    png_read_rows(png_ptr, row_pointers, NULL, height);

  png_read_end (png_ptr, 0);

  if (xrotate)
    ximage = XCreateImage (display, visual, exemplar_image->depth,
			   exemplar_image->format, 0, 0, height, width,
			   exemplar_image->bitmap_pad, 0);
  else
    ximage = XCreateImage (display, visual, exemplar_image->depth,
			   exemplar_image->format, 0, 0, width, height,
			   exemplar_image->bitmap_pad, 0);
  ximage->data = (unsigned char *)malloc(ximage->bytes_per_line * (xrotate ? width : height));

  if (color_type & PNG_COLOR_MASK_ALPHA
      || png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
    {
      if (xrotate)
	xmask = XCreateImage (display, visual, 1,
			      XYPixmap, 0, 0, height, width,
			      8, 0);
      else
	xmask = XCreateImage (display, visual, 1,
			      XYPixmap, 0, 0, width, height,
			      8, 0);
      xmask->data = (unsigned char *)malloc(xmask->bytes_per_line * (xrotate ? width : height));

      if (xrotate)
	src->pixels->image_mask = XCreatePixmap (display, window,
						 src->height, src->width, 1);
      else
	src->pixels->image_mask = XCreatePixmap (display, window,
						 src->width, src->height, 1);
      if (maskgc == 0)
	{
	  maskgc = XCreateGC(display, src->pixels->image_mask, 0, 0);
	  XSetClipMask(display, maskgc, None);
	}
    }
  else
    xmask = 0;

  for (i=0; i<NUM_CONVERTERS; i++)
    {
      if (color_type == image_converters[i].png_image_type)
	{
	  image_converters[i].converter(src);
	  break;
	}
    }
  if (i == NUM_CONVERTERS)
    fprintf(stderr, "No converter for %s\n", src->list->name);

  XSetClipMask(display, gc, None);
  if (xrotate)
    XPutImage (display, src->pixels->image_pixmap, gc, ximage, 0, 0, 0, 0, height, width);
  else
    XPutImage (display, src->pixels->image_pixmap, gc, ximage, 0, 0, 0, 0, width, height);
  if (xmask)
    {
      if (xrotate)
	XPutImage (display, src->pixels->image_mask, maskgc, xmask, 0, 0, 0, 0, height, width);
      else
	XPutImage (display, src->pixels->image_mask, maskgc, xmask, 0, 0, 0, 0, width, height);
    }
  xwin_restore_clip ();

  png_destroy_read_struct(&png_ptr, &info_ptr, 0);

  XDestroyImage (ximage); ximage = 0;
  if (xmask) XDestroyImage (xmask); xmask = 0;
  free(pixel_data); pixel_data = 0;
  free(row_pointers); row_pointers = 0;
}

void
put_image (image *src, int x, int y, int w, int h,
	   image *dest, int dx, int dy, int flags)
{
  Pixmap which, mask;
  GC pgc;
  int sw, sh, dw, dh;
  if (dest == &static_display_image)
    pgc = gc;
  else
    pgc = imggc;

#if 0
  printf("put_image %s.%s.%dx%d (%dx%d+%d+%d) to %s at %d,%d%s%s\n",
	 src->list->name, type_names[src->type], src->width, src->height,
	 w, h, x, y, dest->list->name, dx, dy,
	 flags & PUT_INVERTED ? " inverted" : "",
	 flags & PUT_ROTATED ? " rotated" : "");
#endif

  if (!src->pixels)
    build_image (src);
  if (!dest->pixels)
    build_image (dest);

  if (!src->pixels->image_pixmap)
    return;
  which = src->pixels->image_pixmap;
  mask = src->pixels->image_mask;

  if (xrotate)
    {
      int t = x;
      x = y;
      y = src->width - t - w;
      t = w;
      w = h;
      h = t;
      t = dy;
      dy = dest->width - dx - src->width;
      dx = t;
      sw = src->height;
      sh = src->width;
      dw = dest->height;
      dh = dest->width;
    }
  else
    {
      sw = src->width;
      sh = src->height;
      dw = dest->width;
      dh = dest->height;
    }

  if (flags & PUT_ROTATED)
    {
      Pixmap temp;
      int rx, ry, nx, ny;
      if (!src->pixels->rotated_pixmap)
	{
	  temp = XCreatePixmap(display, window, sw, sh,
			       DefaultDepth(display, screen));
	  src->pixels->rotated_pixmap = XCreatePixmap(display, window, sw, sh,
						      DefaultDepth(display, screen));
	  for (rx=0; rx<sw; rx++)
	    XCopyArea (display, which, temp, pgc,
		       rx, 0, 1, sh, sw-rx-1, 0);
	  for (ry=0; ry<sh; ry++)
	    XCopyArea (display, temp, src->pixels->rotated_pixmap, pgc,
		       0, ry, sw, 1, 0, sh-ry-1);
	  XFreePixmap(display, temp);
	}
      if (src->pixels->image_mask && !src->pixels->rotated_mask)
	{
	  temp = XCreatePixmap(display, window, sw, sh, 1);
	  src->pixels->rotated_mask = XCreatePixmap(display, window, sw, sh, 1);
	  for (rx=0; rx<sw; rx++)
	    XCopyArea (display, mask, temp, maskgc,
		       rx, 0, 1, sh, sw-rx-1, 0);
	  for (ry=0; ry<sh; ry++)
	    XCopyArea (display, temp, src->pixels->rotated_mask, maskgc,
		       0, ry, sw, 1, 0, sh-ry-1);
	  XFreePixmap(display, temp);
	}

      which = src->pixels->rotated_pixmap;
      mask = src->pixels->rotated_mask;
      nx = sw - x - w;
      ny = sh - y - h;
      dx += x-nx;
      dy += y-ny;
      x = nx;
      y = ny;
    }

  if (flags & PUT_INVERTED)
    {
      XImage *img;
      int x, y, b = pixel_for(0, 0, 0), w = pixel_for(255, 255, 255);
      if (!src->pixels->inverted_pixmap)
	{
	  src->pixels->inverted_pixmap
	    = XCreatePixmap(display, window, sw, sh,
			    DefaultDepth(display, screen));
	  XSetClipMask(display, pgc, None);
	  img = XGetImage (display, src->pixels->image_pixmap,
			   0, 0, sw, sh, ~0, ZPixmap);
	  for (x=0; x<sw; x++)
	    for (y=0; y<sh; y++)
	      {
		int p = XGetPixel(img, x, y);
		if (vip->class != PseudoColor)
		  {
		    p = ~p & 0xffffff;
		  }
		else
		  {
		    if (p == w)
		      p = b;
		    else if (p == b)
		      p = w;
		  }
		XPutPixel(img, x, y, p);
	      }
	  XPutImage (display, src->pixels->inverted_pixmap, pgc, img,
		     0, 0, 0, 0, sw, sh);
	  xwin_restore_clip();
	}

      which = src->pixels->inverted_pixmap;
      mask = src->pixels->image_mask;
    }

  if (mask && !broken_xserver)
    {
      XSetClipMask(display, pgc, mask);
      XSetClipOrigin(display, pgc, dx, dy);
    }
  XCopyArea(display, which, dest->pixels->image_pixmap, pgc,
	    x, y, w, h, dx+x, dy+y);
  XSync(display, 0);
  if (mask && !broken_xserver)
    {
      if (pgc == gc)
	xwin_restore_clip();
      else
	XSetClipMask(display, pgc, None);
    }
}

void
put_mask (image *src, int x, int y, int w, int h,
	  image *dest, int dx, int dy, int flags)
{
  Pixmap which, mask;
#if 0
  printf("put_mask %s.%s.%dx%d (%dx%d+%d+%d) to %s at %d,%d%s%s\n",
	 src->list->name, type_names[src->type], src->width, src->height,
	 w, h, x, y, dest->list->name, dx, dy,
	 flags & PUT_INVERTED ? " inverted" : "",
	 flags & PUT_ROTATED ? " rotated" : "");
#endif

  if (!src->pixels)
    build_image (src);
  if (!dest->pixels)
    build_image (src);

  if (!src->pixels->image_pixmap)
    return;
  if (!src->pixels->image_mask)
    return;

  if (xrotate)
    {
      int t = x;
      x = y;
      y = src->width - t - w;
      t = w;
      w = h;
      h = t;
      t = dy;
      dy = table_height - dx - src->width;
      dx = t;
    }

  if (!dest->pixels->image_mask)
    {
      dest->pixels->image_mask = XCreatePixmap(display, window, dest->width, dest->height, 1);
      if (maskgc == 0)
	{
	  maskgc = XCreateGC(display, dest->pixels->image_mask, 0, 0);
	  XSetClipMask(display, maskgc, None);
	}
      XSetForeground(display, maskgc, 1);
      XFillRectangle(display, dest->pixels->image_mask, maskgc, 0, 0, dest->width, dest->height);
    }

  XCopyArea(display, src->pixels->image_mask, dest->pixels->image_mask, maskgc,
	    x, y, w, h, dx+x, dy+y);
}

void fill_image (image *dest, int x, int y, int w, int h,
		 int r, int g, int b)
{
  GC pgc;
  if (dest == &static_display_image)
    pgc = gc;
  else
    pgc = imggc;

  if (!dest->pixels)
    build_image (dest);
  if (!dest->pixels->image_pixmap)
    return;

  if (xrotate)
    {
      int t = x;
      x = dest->height - y - h;
      y = t;
      t = w;
      w = h;
      h = t;
    }

  XSetForeground (display, pgc, pixel_for (r, g, b));
  XFillRectangle (display, dest->pixels->image_pixmap, pgc, x, y, w, h);
}

