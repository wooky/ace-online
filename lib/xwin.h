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

int xwin_nextevent (XWin_Event *event);

#endif
