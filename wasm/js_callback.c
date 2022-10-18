#include <emscripten.h>
#include <stdlib.h>
#include "imagelib.h"

/** List of per-application images, located in games/. In WASM, images are added through `addImageFilename`, so this variable is unused. */
// image_list appimglib_imagelib[] = {{0}};

/** List of card images, located in lib/png. In WASM, images are added through `addImageFilename`, so this variable is unused. */
// image_list cards_imagelib[] = {{0}};

typedef struct
{
  int width;
  int height;
  int frameName;
} SubImage;

void EMSCRIPTEN_KEEPALIVE addImageFilename(const char *filename, int across, int down, int subImageCount, SubImage *subImages)
{
  image *img = malloc(sizeof(image) * (subImageCount + 1));
  for (int i = 0; i < subImageCount; i++)
  {
    img[i] = (image){.width = subImages[i].width, .height = subImages[i].height, .file_data = (const unsigned char *)subImages[i].frameName};
  }
  img[subImageCount] = (image){0};
  image_list *il = malloc(sizeof(image_list) * 2);
  il[0] = (image_list){filename, across, down, {img, 0, 0}, 0, 0, 0};
  il[1] = (image_list){0};
  // register_imagelib(il);
}
