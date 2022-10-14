/** @type HTMLCanvasElement */ let mainCanvas;
/** @type HTMLCanvasElement */ let tempCanvas;

/**
 * @param {HTMLCanvasElement} canvas 
 */
export function initDrawer(canvas) {
  mainCanvas = canvas;
}

/**
 * @param {Number} width
 * @param {Number} height
 */
export function allocateTempCanvas(width, height) {
  tempCanvas = document.createElement("canvas");
  tempCanvas.width = width;
  tempCanvas.height = height;
}

/**
 * @param {boolean} temp
 * @param {Number} x 
 * @param {Number} y 
 * @param {Number} w 
 * @param {Number} h 
 * @param {Number} r 
 * @param {Number} g 
 * @param {Number} b 
 */
export function drawRect(temp, x, y, w, h, r, g, b) {
  const canvas = temp ? tempCanvas : mainCanvas;
  const ctx = canvas.getContext("2d");
  ctx.beginPath();
  ctx.rect(x, y, w, h);
  ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
  ctx.fill();
}

/**
 * @param {String|null} src
 * @param {Number} x
 * @param {Number} y
 * @param {Number} w
 * @param {Number} h
 * @param {boolean} destIsTemp
 * @param {Number} dx
 * @param {Number} dy
 */
export function drawImage(src, x, y, w, h, destIsTemp, dx, dy) {
  if (src) {
    // TODO
    console.warn("TODO drawImage from", src);
    return;
  }
  const srcCanvas = tempCanvas;
  const destCanvas = destIsTemp ? tempCanvas : mainCanvas;
  const destCtx = destCanvas.getContext("2d");
  destCtx.drawImage(srcCanvas, x, y, w, h, dx+x, dy+y, w, h);
}
