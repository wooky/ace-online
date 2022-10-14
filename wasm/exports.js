mergeInto(LibraryManager.library, {
  nextEvent: function (ptr) {
    const { setUpEvents } = require("../wasm/event");
    return Asyncify.handleSleep((wakeUp) => {
      setUpEvents(wakeUp, setValue, ptr);
    });
  },

  /**
   * @param {Number} width
   * @param {Number} height
   * @returns Number
   */
  allocateSynthImage: function (width, height) {
    const { allocateTempCanvas } = require("../wasm/drawer");
    const name = allocateTempCanvas(width, height);
    const ptr = allocateUTF8(name);
    return ptr;
  },

  /**
   * @param {Number} ptr
   * @param {Number} x 
   * @param {Number} y 
   * @param {Number} w 
   * @param {Number} h 
   * @param {Number} r 
   * @param {Number} g 
   * @param {Number} b 
   */
  fillImage: function (ptr, x, y, w, h, r, g, b) {
    console.debug("fillImage:", x, y, w, h, r, g, b);
    const { drawRect } = require("../wasm/drawer");
    const name = ptr ? UTF8ToString(ptr) : null;
    drawRect(name, x, y, w, h, r, g, b);
  },

  /**
   * @param {Number} srcPtr
   * @param {Number} x
   * @param {Number} y
   * @param {Number} w
   * @param {Number} h
   * @param {Number} destPtr
   * @param {Number} dx
   * @param {Number} dy
   */
  putImage: function (srcPtr, x, y, w, h, destPtr, dx, dy) {
    const { drawImage } = require("../wasm/drawer");
    const src = UTF8ToString(srcPtr);
    const dest = srcPtr ? UTF8ToString(destPtr) : null;
    drawImage(src, x, y, w, h, dest, dx, dy);
  },

  /**
   * @param {string} image 
   */
  getImage: function (image) {

  },
});
