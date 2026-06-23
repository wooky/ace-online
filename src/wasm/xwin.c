/* The Ace of Penguins - WASM shim for lib/xwin.c (issue #161).
 *
 * The real lib/xwin.c is the X11 + libpng window/drawing backend.  The WASM
 * target has neither, so for the WASM build this file is compiled in its
 * place.  It provides every symbol the rest of the framework, the launcher
 * and the games reference from xwin.c, as stubs that let the program build
 * (and link) cleanly.  The actual canvas/rendering implementation will be
 * filled in later (this is scaffolding).
 */

#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>   /* the WASM stub header (src/wasm/stubs) */

#include "imagelib.h"
#include "table.h"
#include "cards.h"
#include "xwin.h"

/* --- globals other translation units reference (extern) from xwin.c ------ */
Display *display = 0;
Window   window  = 0;
int      screen  = 0;
GC       gc      = 0;

int font_width  = 6;
int font_height = 13;

int table_background;
int help_background;

OptionDesc *xwin_options = 0;

image *display_image = 0;
static image      shim_display_image;
static image_list shim_display_list;

/* --- option/setup ------------------------------------------------------- */
int
xwin_init(int argc, char **argv)
{
  (void) argc; (void) argv;
  /* The same green as the X11 build (pixel_for(0, 0x66, 0)). */
  table_background = pixel_for(0x00, 0x66, 0x00);
  help_background  = pixel_for(0x00, 0x00, 0x00);
  return 0;
}

void
xwin_create(int width, int height)
{
  shim_display_image.width  = width;
  shim_display_image.height = height;
  shim_display_image.list   = &shim_display_list;
  shim_display_list.name    = "WASM canvas";
  shim_display_list.across  = 1;
  shim_display_list.down    = 1;
  display_image = &shim_display_image;
}

void xwin_fixed_size(int width, int height) { (void) width; (void) height; }

/* --- event loop --------------------------------------------------------- */
int
xwin_nextevent(XWin_Event *ev)
{
  /* No event source yet; report "nothing".  The emscripten main-loop driver
     will be wired up when the implementation lands. */
  ev->type = ev_none;
  return ev_none;
}

/* --- clipping / drawing primitives (no-ops for now) --------------------- */
void xwin_clip(int x, int y, int w, int h) { (void)x;(void)y;(void)w;(void)h; }
void xwin_noclip(void) {}

void clear(int x, int y, int w, int h) { (void)x;(void)y;(void)w;(void)h; }
void text(char *t, int x, int y) { (void)t;(void)x;(void)y; }

void flush(void) {}
void flushsync(void) {}
void beep(void) {}

int
pixel_for(int r, int g, int b)
{
  return ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

void
put_image(image *src, int x, int y, int w, int h,
	  image *dest, int dx, int dy, int flags)
{
  (void)src;(void)x;(void)y;(void)w;(void)h;(void)dest;(void)dx;(void)dy;(void)flags;
}

void
put_mask(image *src, int x, int y, int w, int h,
	 image *dest, int dx, int dy, int flags)
{
  (void)src;(void)x;(void)y;(void)w;(void)h;(void)dest;(void)dx;(void)dy;(void)flags;
}

void
fill_image(image *dest, int x, int y, int w, int h, int r, int g, int b)
{
  (void)dest;(void)x;(void)y;(void)w;(void)h;(void)r;(void)g;(void)b;
}
