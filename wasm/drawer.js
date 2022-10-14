import uuid from "uuid";

/** @type HTMLCanvasElement */ let mainCanvas;
/** @type Object.<string, HTMLCanvasElement> */ const tempCanvases = {};

/**
 * @param {HTMLCanvasElement} canvas 
 */
export function initDrawer(canvas) {
  mainCanvas = canvas;
}

/**
 * @param {Number} width
 * @param {Number} height
 * @returns String
 */
export function allocateTempCanvas(width, height) {
  const key = uuid.v4();
  const canvas = document.createElement("canvas");
  canvas.width = width;
  canvas.height = height;
  tempCanvases[key] = canvas;
  return key;
}

/**
 * @param {String|null} name
 * @param {Number} x 
 * @param {Number} y 
 * @param {Number} w 
 * @param {Number} h 
 * @param {Number} r 
 * @param {Number} g 
 * @param {Number} b 
 */
export function drawRect(name, x, y, w, h, r, g, b) {
  const canvas = getCanvas(name);
  const ctx = canvas.getContext("2d");
  ctx.beginPath();
  ctx.rect(x, y, w, h);
  ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
  ctx.fill();
}

/**
 * @param {String} src
 * @param {Number} x
 * @param {Number} y
 * @param {Number} w
 * @param {Number} h
 * @param {String|null} dest
 * @param {Number} dx
 * @param {Number} dy
 */
export function drawImage(src, x, y, w, h, dest, dx, dy) {
  if (!(src in tempCanvases)) {
    // TODO
    console.warn("TODO drawImage from", src);
    return;
  }
  const srcCanvas = getCanvas(src);
  const destCanvas = getCanvas(dest);
  const destCtx = destCanvas.getContext("2d");
  destCtx.drawImage(srcCanvas, x, y, w, h, dx+x, dy+y, w, h);
  // delete tempCanvases[src]; // TODO
}

/**
 * 
 * @param {String|null} name
 * @returns HTMLCanvasElement
 */
function getCanvas(name) {
  return name ? tempCanvases[name] : mainCanvas;
}
