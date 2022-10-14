#include <emscripten.h>
#include <stdlib.h>
#include "imagelib.h"
#include "table.h"

extern void register_image(image_list *images);

/** List of per-application images, located in games/. In WASM, images are added through `addImageFilename`, so this variable is unused. */
image_list appimglib_imagelib[] = {{0}};

/** List of card images, located in lib/png. In WASM, images are added through `addImageFilename`, so this variable is unused. */
image_list cards_imagelib[] = {{0}};

void EMSCRIPTEN_KEEPALIVE setTableSize(int width, int height)
{
  table_width = width;
  table_height = height;
}

void EMSCRIPTEN_KEEPALIVE addImageFilename(const char *filename, int width, int height)
{
  image *img = malloc(sizeof(image));
  *img = (image){.width = width, .height = height, .file_data = filename};
  image_list *il = malloc(sizeof(image_list));
  *il = (image_list){filename, 1, 1, {img, 0, 0}};
  img->list = il;
  register_image(il);
}
