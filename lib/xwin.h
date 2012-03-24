#ifndef _XWIN_H_
#define _XWIN_H_


#define ES_SHIFT	0x01
#define ES_ALT		0x02
#define ES_CONTROL	0x04

typedef enum {
  ev_none,
  ev_keypress,
  ev_buttondown,
  ev_buttonup,
  ev_motion,
  ev_resize,
  ev_expose,
  ev_quit
} XWin_Event_Type;

typedef struct {
  XWin_Event_Type type;
  int x, y, w, h;
  int button;
  int shifts; /* button, key only */
  int key;
  int time; /* milliseconds.  button, motion only */
} XWin_Event;

/* sets display_width, display_height, table_width/height to preferred */
int xwin_init (int argc, char **argv);
/* sets table_width, table_height to actual */
void xwin_create (int width, int height);
int xwin_nextevent (XWin_Event *event);

void xwin_fixed_size (int width, int height);
void xwin_clip (int x, int y, int w, int h);
void xwin_noclip ();
int pixel_for (int r, int g, int b);

#endif
