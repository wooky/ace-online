import aceTextures from "@build/dist/ace-texture.json";
import textureSizes from "../lib/images.in";

export function loadImages(Module) {
  const images = {};
  for (const frame in aceTextures.frames) {
    const filenameParts = frame.split(".");
    const filename = filenameParts[0];
    if (!(filename in images)) {
      images[filename] = {};
    }
    images[filename][frame] = aceTextures.frames[frame];
  }

  for (const filename in images) {
    let across = 1, down = 1;
    const row = textureSizes.find(e => e[0] == filename);
    if (row && row.length == 3) {
      across = row[1];
      down = row[2];
    }

    const subImageNames = Object.keys(images[filename]);
    const subImageCount = subImageNames.length;
    const subImages = Module._malloc(4 * 3 * subImageCount);
    for (let i = 0; i < subImageCount; i++) {
      const subImageName = subImageNames[i];
      Module.setValue(subImages + i*12 + 0, images[filename][subImageName].frame.w, 'i32');
      Module.setValue(subImages + i*12 + 4, images[filename][subImageName].frame.h, 'i32');
      Module.setValue(subImages + i*12 + 8, Module.allocateUTF8(subImageName), 'i32');
    }

    const filenamePtr = Module.allocateUTF8(filename);
    Module._addImageFilename(filenamePtr, across, down, subImageCount, subImages);
    Module._free(subImages);
  }
}

export function getImage(name) {
  return aceTextures.frames[name];
}
