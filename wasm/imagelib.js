import aceTextures from "../build/dist/ace-texture.json";

const images = {};

export function loadImages(Module) {
  for (const frame in aceTextures.frames) {
    const filenameParts = frame.split(".");
    const filename = filenameParts[0];
    if (filename in images) {
      // If this frame has no size, bail, because the one in `images` is bigger.
      if (filenameParts.length < 2) {
        continue;
      }
      const existingFilenameParts = images[filename].split(".");
      if (existingFilenameParts.length > 1 && parseInt(existingFilenameParts[1]) > parseInt(filenameParts[1])) {
        continue;
      }
    }
    images[filename] = frame;
  }

  for (const filename in images) {
    const frame = aceTextures.frames[images[filename]];
    const ptr = Module.allocateUTF8(filename);
    Module._addImageFilename(ptr, frame.frame.w, frame.frame.h);
  }
}
