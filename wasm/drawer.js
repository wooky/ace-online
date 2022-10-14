import { getImage } from "@/imagelib";
import textureUrl from "@build/dist/ace-texture.png";

const texture = new Image();
/** @type HTMLCanvasElement */ let mainCanvas;
/** @type HTMLCanvasElement */ let tempCanvas;

/**
 * @param {HTMLCanvasElement} canvas 
 */
export async function initDrawer(canvas) {
  mainCanvas = canvas;
  texture.src = textureUrl;
  await texture.decode();
  tempCanvas = document.createElement("canvas");
  tempCanvas.width = canvas.width;
  tempCanvas.height = canvas.height;
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
  let srcCanvas = tempCanvas, sx = 0, sy = 0;
  if (src) {
    const frame = getImage(src);
    sx = frame.frame.x;
    sy = frame.frame.y;
    srcCanvas = texture;
  }
  const destCanvas = destIsTemp ? tempCanvas : mainCanvas;
  const destCtx = destCanvas.getContext("2d");
  destCtx.drawImage(srcCanvas, sx + x, sy + y, w, h, dx + x, dy + y, w, h);
}
