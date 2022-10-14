import { getImage } from "@/imagelib";
import textureUrl from "@build/dist/ace-texture.png";

const PUT_INVERTED = 0x01;
const PUT_ROTATED = 0x02;

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
  resizeCanvases();
}

export function resizeCanvases() {
  mainCanvas.width = tempCanvas.width = mainCanvas.parentElement.offsetWidth;
  mainCanvas.height = tempCanvas.height = mainCanvas.parentElement.offsetHeight - 4;
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
 * @param {Number} flags
 */
export function drawImage(src, x, y, w, h, destIsTemp, dx, dy, flags) {
  let srcCanvas = tempCanvas, sx = 0, sy = 0;
  if (src) {
    const frame = getImage(src);
    sx = frame.frame.x;
    sy = frame.frame.y;
    srcCanvas = texture;
  }
  const destCanvas = destIsTemp ? tempCanvas : mainCanvas;
  const destCtx = destCanvas.getContext("2d");
  destCtx.save();

  if (flags & PUT_ROTATED) {
    destCtx.translate(dx + x + Math.floor(w/2), dy + y + Math.floor(h/2));
    destCtx.rotate(Math.PI);
    destCtx.translate(-Math.floor(w/2), -Math.floor(h/2));
  }
  else {
    destCtx.translate(dx + x, dy + y);
  }

  if (flags & PUT_INVERTED) {
    // TODO
    console.warn("TODO drawImage inverted");
  }

  destCtx.drawImage(srcCanvas, sx + x, sy + y, w, h, 0, 0, w, h);
  destCtx.restore();
}
