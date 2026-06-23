/* Stub <X11/Xlib.h> for the WASM build (issue #161).
 *
 * The WASM target has no X11 library.  The framework's window/drawing code is
 * replaced by the shim src/wasm/xwin.c, but a few upstream files (lib/help.c,
 * games/taipedit.c, games/golf.c, games/pegged.c) still pull in <X11/...> and
 * call a handful of Xlib functions directly.  This header provides just enough
 * dummy types + no-op inline functions so those files compile and link with no
 * X11 library.  Real behaviour will come later. */
#ifndef _ACE_WASM_XLIB_STUB_H_
#define _ACE_WASM_XLIB_STUB_H_

typedef unsigned long XID;
typedef XID Window;
typedef XID Pixmap;
typedef XID Drawable;
typedef XID Font;
typedef XID Colormap;
typedef XID Atom;
typedef unsigned long KeySym;
typedef int Bool;
typedef int Status;
typedef int Screen;
typedef struct _Display Display;
typedef struct _Visual Visual;
typedef struct _XGC *GC;

typedef struct {
  int lbearing, rbearing, width, ascent, descent;
  unsigned short attributes;
} XCharStruct;

typedef struct {
  int ascent;
  int descent;
  Font fid;
  XCharStruct min_bounds, max_bounds;
} XFontStruct;

#ifndef None
#define None 0L
#endif

#define BlackPixel(dpy, scr) (0UL)
#define WhitePixel(dpy, scr) (0xffffffUL)

static inline XFontStruct *
XLoadQueryFont(Display *d, const char *name)
{
  static XFontStruct f = { 12, 4, 0, { 0, 0, 6, 12, 4, 0 }, { 0, 0, 6, 12, 4, 0 } };
  (void) d; (void) name;
  return &f;
}
static inline int XSetFont(Display *d, GC g, Font f) { (void)d;(void)g;(void)f; return 0; }
static inline int XSetForeground(Display *d, GC g, unsigned long c) { (void)d;(void)g;(void)c; return 0; }
static inline int XFillRectangle(Display *d, Drawable w, GC g, int x, int y, unsigned int ww, unsigned int hh)
{ (void)d;(void)w;(void)g;(void)x;(void)y;(void)ww;(void)hh; return 0; }
static inline int XDrawString(Display *d, Drawable w, GC g, int x, int y, const char *s, int n)
{ (void)d;(void)w;(void)g;(void)x;(void)y;(void)s;(void)n; return 0; }
static inline int XDrawLine(Display *d, Drawable w, GC g, int x1, int y1, int x2, int y2)
{ (void)d;(void)w;(void)g;(void)x1;(void)y1;(void)x2;(void)y2; return 0; }
static inline int XDrawPoint(Display *d, Drawable w, GC g, int x, int y)
{ (void)d;(void)w;(void)g;(void)x;(void)y; return 0; }
static inline int XTextWidth(XFontStruct *f, const char *s, int n)
{ (void)f;(void)s; return n * 6; }
static inline int
XTextExtents(XFontStruct *f, const char *s, int n,
	     int *dir, int *asc, int *desc, XCharStruct *overall)
{
  (void) f; (void) s;
  if (dir) *dir = 0;
  if (asc) *asc = 12;
  if (desc) *desc = 4;
  if (overall) { overall->lbearing = 0; overall->rbearing = n * 6;
                 overall->width = n * 6; overall->ascent = 12; overall->descent = 4; }
  return 0;
}

#endif
