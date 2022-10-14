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
   */
  allocateSynthImage: function (width, height) {
    const { allocateTempCanvas } = require("../wasm/drawer");
    allocateTempCanvas(width, height);
  },

  /**
   * @param {Number} temp
   * @param {Number} x 
   * @param {Number} y 
   * @param {Number} w 
   * @param {Number} h 
   * @param {Number} r 
   * @param {Number} g 
   * @param {Number} b 
   */
  fillImage: function (temp, x, y, w, h, r, g, b) {
    const { drawRect } = require("../wasm/drawer");
    drawRect(temp, x, y, w, h, r, g, b);
  },

  /**
   * @param {Number} srcPtr
   * @param {Number} x
   * @param {Number} y
   * @param {Number} w
   * @param {Number} h
   * @param {Number} destIsTemp
   * @param {Number} dx
   * @param {Number} dy
   */
  putImage: function (srcPtr, x, y, w, h, destIsTemp, dx, dy) {
    const { drawImage } = require("../wasm/drawer");
    const src = srcPtr ? UTF8ToString(srcPtr) : null;
    drawImage(src, x, y, w, h, destIsTemp, dx, dy);
  },

  /**
   * @param {string} image 
   */
  getImage: function (image) {

  },
});
