#include <emscripten.h>
#include "imagelib.h"
#include "table.h"
#include "cards.h"
#include "xwin.h"

/** "Alias" for the screen. May be used as the `dest` parameter in some functions. */
image *display_image;

int font_width, font_height;

/**
 * Initialize the screen. In WASM, shouldn't do much, except set some hardcoded values.
 */
int xwin_init(int argc, char **argv)
{
  table_type = TABLE_COLOR;
  display_width = __INT_MAX__;
  display_height = __INT_MAX__;

  int font_size = EM_ASM_INT({
    const {calculateTextSize} = require("@/drawer");
    const measurements = calculateTextSize("@");
    return (measurements.width << 16) | (measurements.height);
  });
  font_width = font_size >> 16;
  font_height = font_size & 0xFFFF;

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
    const {drawRect} = require("@/drawer");
    drawRect($0, $1, $2, $3, $4, $5, $6, $7);
  },
         dest != display_image, x, y, w, h, r, g, b);
}

void put_image(image *src, int x, int y, int w, int h,
               image *dest, int dx, int dy, int flags)
{
  if (src->synth_func)
  {
    src->synth_func(src);
  }
  // TODO why is EM_ASM so picky when it comes to creating objects/arrays?
  EM_ASM(
      {
        const {drawImage} = require("@/drawer");
        let src = null;
        if ($0)
        {
          src = {};
          src.x = ($0 >> 16);
          src.y = ($0 & 0xFFFF);
        }
        drawImage(src, $1, $2, $3, $4, $5, $6, $7, $8);
      },
      src->file_data, x, y, w, h, dest != display_image, dx, dy, flags);
}

void text(char *s, int x, int y)
{
  EM_ASM({
    const {drawText} = require("@/drawer");
    drawText(UTF8ToString($0), $1, $2);
  },
         s, x, y);
}

void xwin_fixed_size(int width, int height)
{
  EM_ASM({
    const {setFixedSize} = require("@/drawer");
    setFixedSize($0, $1);
  },
         width, height);
}

int xwin_nextevent(XWin_Event *ev)
{
  EM_ASM(
      {
        const {setUpEvents} = require("@/event");
        return Asyncify.handleSleep(function(wakeUp) {
          setUpEvents(wakeUp, setValue, $0);
        });
      },
      ev);
  return 0;
}

void beep()
{
  EM_ASM(
      {
        const {beep} = require("@/beeper");
        beep();
      });
}

void flushsync()
{
  emscripten_sleep(0);
}

void help(char *filename, char *text)
{
  EM_ASM({
    window.open("http://www.delorie.com/store/ace/docs/" + UTF8ToString($0));
  },
         filename);
}

void put_mask(image *src, int x, int y, int w, int h,
              image *dest, int dx, int dy, int flags)
{
  // Do nothing.
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
