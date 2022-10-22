#include <stdlib.h>
#include "funcs.h"
#include "table.h"
#include "cards.h"

/*
 * TODO this file is required because the original funcs.c does not include the "table.h" header.
 * As a consequence, emcc produces a call to table_no_resize that's invalid.
 * Figure out if something can be done while using the original func.c file.
 */

static void
default_click_cb(int x, int y, int b)
{
}

void (*click_cb)(int, int, int) = default_click_cb;

static void
default_drag_cb(int x, int y, int b)
{
}

void (*drag_cb)(int, int, int) = default_drag_cb;

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

void (*drop_cb)(int, int, int) = default_drop_cb;

static void
default_key_cb(int x, int y, int k)
{
  if (k == 3 || k == 27 || k == 'q')
    exit(0);
}

void (*key_cb)(int, int, int) = default_key_cb;

static void
default_resize_cb(int w, int h)
{
  table_no_resize();
}

void (*resize_cb)(int, int) = default_resize_cb;

static void
default_double_click_cb(int x, int y, int b)
{
  click_cb(x, y, b);
}

void (*double_click_cb)(int, int, int) = default_double_click_cb;
