/// imagelib is doing too much for our needs, so this should do.

#include <emscripten.h>
#include <map>
#include <string>
#include "imagelib.h"
#include "table.h"

extern "C"
{
  extern const unsigned char* allocateSynthImage(int width, int height);

  std::map<std::string, image_list *> _image_lists;

  /**
   * @internal
   */
  inline void register_image(image_list *images)
  {
    emscripten_log(EM_LOG_DEBUG, "Registering image %s", images->name);
    _image_lists[std::string(images->name)] = images;
  }

  int register_imagelib(image_list *images)
  {
    for (image_list *il = images; il->name; il++)
    {
      register_image(il);
    }
    return 0;
  }

  image *get_image(char *name,
                   int preferred_width,
                   int preferred_height,
                   int flags)
  {
    // for (auto& asdf : _image_lists) {
    //   emscripten_log(EM_LOG_DEBUG, "I have a %s", asdf.first.c_str());
    // }
    emscripten_log(EM_LOG_DEBUG, "Requesting image %s", name);
    auto il = _image_lists.at(name);
    if (il->synth_func)
    {
      return il->synth_func(il, table_type, preferred_width, preferred_height);
    }
    return il->subimage[0];
  }

  void put_subimage(image *src, int col, int row,
                    image *dest, int dx, int dy, int flags)
  {
    put_image(src, 0, 0, 0, 0, dest, dx, dy, flags);
  }

  image *alloc_synth_image(image_list *list, int width, int height, int type)
  {
    auto name = allocateSynthImage(width, height);
    return new image{.width = width, .height = height, .file_data = name, .list = list};
  }
}
