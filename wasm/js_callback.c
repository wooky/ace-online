#include <emscripten.h>
#include <stdlib.h>
#include "imagelib.h"
#include "table.h"

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
  image *img = malloc(sizeof(image) * 2);
  img[0] = (image){.width = width, .height = height, .file_data = filename};
  img[1] = (image){0};
  image_list *il = malloc(sizeof(image_list) * 2);
  il[0] = (image_list){filename, 1, 1, {img, 0, 0}, 0, 0, 0};
  il[1] = (image_list){0};
  register_imagelib(il);
}
