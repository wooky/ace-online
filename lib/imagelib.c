#include <stdio.h>
#include <string.h>
#include "imagelib.h"
#include "cards.h"

image_list *image_root = 0;

int
register_imagelib (image_list *images)
{
  int i, j, k;
  for (i=0; images[i].name; i++)
    {
      if (images[i].next)
	continue;
      images[i].next = image_root;
      image_root = images+i;
      for (j=0; j<3; j++)
	if (images[i].subimage[j])
	  for (k=0; images[i].subimage[j][k].width; k++)
	    {
	      if (images[i].subimage[j][k+1].width)
		images[i].subimage[j][k].next = images[i].subimage[j] + k + 1;
	      images[i].subimage[j][k].list = images+i;
	      images[i].subimage[j][k].type = j;
	    }
    }
}

static int type_search[3][3] = {
  { TABLE_MONO,  TABLE_GRAY, TABLE_COLOR },
  { TABLE_GRAY,  TABLE_MONO, TABLE_COLOR },
  { TABLE_COLOR, TABLE_GRAY, TABLE_MONO  }};

#define abs(a) ((a) < 0 ? -(a) : (a))

image *
get_image (char *name,
	   int preferred_width,
	   int preferred_height,
	   int flags)
{
  image_list *list;
  image *img, *best;
  int type, best_w, best_h;

  for (list=image_root; list; list=list->next)
    if (strcmp(name, list->name) == 0)
      break;
  if (!list)
    {
      printf("No image named `%s' available\n", name);
      return 0;
    }
  if (list->synth_func)
    return list->synth_func (list, table_type, preferred_width, preferred_height);

  best = 0;
  best_w = 0;
  best_h = 0;
  for (type = 0; type<3; type++)
    {
      for (img = list->subimage[type_search[table_type][type]]; img; img=img->next)
	{
	  if (flags & GI_NOT_BIGGER)
	    {
	      if (img->width > best_w && img->width <= preferred_width
		  && img->height > best_h && img->height <= preferred_height)
		{
		  best = img;
		  best_w = img->width;
		  best_h = img->height;
		}
	    }
	  else
	    {
	      int diff = abs(preferred_width - img->width) + abs(preferred_height - img->height);
	      if (diff < best_w || !best)
		{
		  best = img;
		  best_w = diff;
		}
	    }
	}
      if (best && !(flags & GI_ANY_TYPE))
	break;
    }

  return best;
}

void put_subimage (image *src, int col, int row,
		   image *dest, int dx, int dy, int flags)
{
  int x, y, w, h;
  if (src->list->across == 1 && src->list->down == 1)
    col = row = 0;
  w = src->width / src->list->across;
  h = src->height / src->list->down;
  x = w * col;
  y = h * row;
  put_image (src, x, y, w, h, dest, dx-x, dy-y, flags);
}

image *
alloc_synth_image (image_list *list, int width, int height, int type)
{
  image *rv;
  rv = (image *)malloc (sizeof(image));
  rv->width = width;
  rv->height = height;
  rv->file_data = 0;
  rv->next = list->subimage[type];
  list->subimage[type] = rv;
  rv->type = type;
  rv->pixels = 0;
  rv->list = list;
  return rv;
}
