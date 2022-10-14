#include <emscripten.h>
#include "imagelib.h"
#include "table.h"
#include "cards.h"
#include "xwin.h"

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
  EM_ASM({
    const { drawRect } = require("@/drawer");
    drawRect($0, $1, $2, $3, $4, $5, $6, $7);
  }, dest != display_image, x, y, w, h, r, g, b);
}

void put_image(image *src, int x, int y, int w, int h,
               image *dest, int dx, int dy, int flags)
{
  if (src->synth_func)
  {
    src->synth_func(src);
  }
  EM_ASM({
    const { drawImage } = require("@/drawer");
    const src = $0 ? UTF8ToString($0) : null;
    drawImage(src, $1, $2, $3, $4, $5, $6, $7);
  }, src->file_data, x, y, w, h, dest != display_image, dx, dy);
}

void xwin_fixed_size(int width, int height)
{
  // TODO
  emscripten_log(EM_LOG_WARN, "TODO xwin_fixed_size");
}

int xwin_nextevent(XWin_Event *ev)
{
  EM_ASM({
    const { setUpEvents } = require("@/event");
    return Asyncify.handleSleep((wakeUp) => {
      setUpEvents(wakeUp, setValue, $0);
    });
  }, ev);
  return 0;
}

void flushsync()
{
  emscripten_sleep(0);
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

void xwin_clip(int x, int y, int w, int h)
{
  // Do nothing.
}

void xwin_noclip()
{
  // Do nothing.
}
