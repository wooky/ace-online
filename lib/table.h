#ifndef _TABLE_H_
#define _TABLE_H_

#define TABLE_MONO	0
#define TABLE_GRAY	1
#define TABLE_COLOR	2
extern int table_type;

extern int display_width, display_height;
extern int table_width, table_height;

#define OPTION_BOOLEAN	1
#define OPTION_STRING	2
#define OPTION_INTEGER	3
typedef struct {
  char *option;
  int type;
  void *ptr;
} OptionDesc;
/* Apps do `OptionDesc *app_options = app_option_table;' if needed, last zero */

typedef struct {
  char *name;
  void *function;
} FunctionMapping;

/* This sets display_width/height, sets table_width/height to preferred or zero */
void init_ace(int argc, char **argv, FunctionMapping *funcs);
/* This creates the initial window */
void init_table(int table_width, int table_height);

extern int get_picture_default_width, get_picture_default_height;

/* Really, see image in imagelib.h */
typedef struct Picture {
  int w, h;
} Picture;

Picture *get_picture(char *name);

/* The x,y,w,h are relative to (0,0) on the picture.  The dx,dy
   indicate where (0,0) on the picture would go */
void put_picture(Picture *picture, int dx, int dy,
		 int x, int y, int w, int h);
void put_picture_inverted(Picture *picture, int dx, int dy,
			  int x, int y, int w, int h);

/* This is drawn over everything else, centered.  Pass zero to remove */
void set_centered_pic(Picture *picture);
Picture *get_centered_pic();

extern int help_is_showing;

extern void (*help_redraw)(void);
extern void (*help_click)(int x, int y, int b);
extern void (*help_key)(int c, int x, int y);

void table_no_resize();

/* Call this to begin processing events; */
void table_loop();

void clip(int x, int y, int w, int h);
void clip_more(int x, int y, int w, int h);
void unclip();
void invalidate(int x, int y, int w, int h);
void invalidate_nc(int x, int y, int w, int h);
void invalidate_exposure(int ox, int oy, int ow, int oh,
			 int nx, int ny, int nw, int nh);

void snap_to_grid(int *x, int *y,
		  int step_x, int step_y,
		  int origin_x, int origin_y,
		  int max_distance);

#endif
