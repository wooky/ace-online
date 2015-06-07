#ifndef _IMAGELIB_H_
#define _IMAGELIB_H_

#ifdef _cards_h_
#error include imagelib.h before cards.h
#endif

#ifndef PIXELS_TYPE
#define PIXELS_TYPE void *
#endif

typedef struct image {
  /* filled in by make-imglib */
  int width;
  int height;
  const unsigned char *file_data;
  /* available to program */
  struct image *next;
  int type;
  PIXELS_TYPE pixels; /* platform and display specific format, like XImage or Pixmap */
  struct image_list *list;
  void (*synth_func)(struct image *); /* if set, is synthetic */
  int synth_flags;
} image;

struct image_list;

typedef image *(*synth_imagelist_function)(struct image_list *list, int type, int width, int height);

typedef struct image_list {
  /* filled in by make-imglib */
  char *name;
  int across;
  int down;
  image *subimage[3];
  /* available to program */
  struct image_list *next;
  synth_imagelist_function synth_func; /* if set, is synthetic */
  int synth_flags;
} image_list;

/* Put to this to display on the display's main window. */
extern image *display_image;

#ifdef __cplusplus
extern "C" {
#if 0
} /* To keep emacs from indenting all the below functions. */
#endif
#endif

extern image_list appimglib_imagelib[];
int register_imagelib(image_list *);

image *alloc_synth_image (image_list *list, int width, int height, int type);

image *get_image (char *name, int preferred_width, int preferred_height, int flags);
/* This means the preferred dimensions are absolute maximums. */
#define GI_NOT_BIGGER	0x01
/* This means that a close size match is more important than the right type. */
#define GI_ANY_TYPE	0x02

#define PUT_INVERTED	0x01
#define PUT_ROTATED	0x02
/* dx dy relative to where src's origin would have been, not relative
   to the x, y point indicated here. */
void put_image (image *src, int x, int y, int w, int h,
		image *dest, int dx, int dy, int flags);
/* Same, but copies mask (creating one if needed) */
void put_mask (image *src, int x, int y, int w, int h,
	       image *dest, int dx, int dy, int flags);
/* Similar, but always the whole subimage (based on across, down in image_list) */
void put_subimage (image *src, int col, int row,
		   image *dest, int dx, int dy, int flags);
/* For synthesizing images. */
void fill_image (image *dest, int x, int y, int w, int h,
		 int r, int g, int b);

#ifdef __cplusplus
}
#endif

#endif
