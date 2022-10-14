#include <emscripten.h>
#include "imagelib.h"
#include "table.h"
#include "cards.h"
#include "xwin.h"

extern void nextEvent(XWin_Event *);
extern void allocateSynthImage(int width, int height);
extern void fillImage(int temp, int x, int y, int w, int h, int r, int g, int b);
extern void putImage(const unsigned char *src, int x, int y, int w, int h, int destIsTemp, int dx, int dy);

/** "Alias" for the screen. May be used as the `dest` parameter in some functions. */
image *display_image;

// TODO auto-purge these _help variables from the codebase
char *thornq_help;

/**
 * Initialize the screen. In WASM, shouldn't do much.
 */
int xwin_init(int argc, char **argv)
{
  table_type = TABLE_COLOR;
  return 0;
}

/**
 * Create the window. In WASM, the window is the browser, which is already created, so nothing needs to be done.
 */
void xwin_create(int width, int height)
{
  // Do nothing.
}

/**
 * Replace a portion of the screen with the "default" color #006600.
 */
void clear(int x, int y, int w, int h)
{
  fill_image(display_image, x, y, w, h, 0x00, 0x66, 0x00);
}

void fill_image(image *dest, int x, int y, int w, int h,
                int r, int g, int b)
{
  fillImage(dest != display_image, x, y, w, h, r, g, b);
}

void put_image(image *src, int x, int y, int w, int h,
               image *dest, int dx, int dy, int flags)
{
  // TODO
  if (src->synth_func)
  {
    allocateSynthImage(w, h);
    src->synth_func(src);
  }
  putImage(src->file_data, x, y, w, h, dest != display_image, dx, dy);
}

void xwin_clip(int x, int y, int w, int h)
{
  // TODO
  emscripten_log(EM_LOG_WARN, "TODO xwin_clip");
}

void xwin_fixed_size(int width, int height)
{
  // TODO
  emscripten_log(EM_LOG_WARN, "TODO xwin_fixed_size");
}

int xwin_nextevent(XWin_Event *ev)
{
  nextEvent(ev);
  emscripten_log(EM_LOG_DEBUG, "Got event type=%d x=%d y=%d w=%d h=%d", ev->type, ev->x, ev->y, ev->w, ev->h);
  return 0;
}

void xwin_noclip()
{
  // TODO
  emscripten_log(EM_LOG_WARN, "TODO xwin_noclip");
}

void help(char *filename, char *text)
{
  // TODO
  emscripten_log(EM_LOG_WARN, "TODO help");
}

void flush()
{
  // Do nothing.
}

void flushsync()
{
  // Do nothing.
}
