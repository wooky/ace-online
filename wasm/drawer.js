import textureUrl from "@build/ace-texture.png";
import { deinitResizeEvent } from "./event";

const PUT_INVERTED = 0x01;
const PUT_ROTATED = 0x02;

/** @type HTMLCanvasElement */ let mainCanvas;
const tempCanvas = document.createElement("canvas");
const texture = new Image();
const textureRotated = document.createElement("canvas");

/**
 * @param {HTMLCanvasElement} canvas 
 */
export async function initDrawer(canvas) {
  mainCanvas = canvas;

  texture.src = textureUrl;
  await texture.decode();
  textureRotated.width = texture.width;
  textureRotated.height = texture.height;
  const textureRotatedCtx = textureRotated.getContext("2d");
  textureRotatedCtx.translate(texture.width / 2, texture.height / 2);
  textureRotatedCtx.rotate(Math.PI);
  textureRotatedCtx.drawImage(texture, -texture.width / 2, -texture.height / 2);
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
  const ctx = canvas.getContext("2d", { alpha: false });
  ctx.beginPath();
  ctx.rect(x, y, w, h);
  ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
  ctx.fill();
}

/**
 * @param {Object|null} src
 * @param {Number} src.x
 * @param {Number} src.y
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
  let srcCanvas = tempCanvas, sx = x, sy = y;
  const destCanvas = destIsTemp ? tempCanvas : mainCanvas;
  const destCtx = destCanvas.getContext("2d", { alpha: false });

  if (flags & PUT_ROTATED) {
    if (src) {
      sx = textureRotated.width - src.x - w - x;
      sy = textureRotated.height - src.y - h - y;
      srcCanvas = textureRotated;
    }
    else {
      console.error("drawImage rotated from a non-texture");
    }
  }
  else if (src) {
    sx += src.x;
    sy += src.y;
    srcCanvas = texture;
  }

  if (flags & PUT_INVERTED) {
    // TODO
    console.warn("TODO drawImage inverted");
  }

  destCtx.drawImage(srcCanvas, sx, sy, w, h, dx + x, dy + y, w, h);
}

/**
 * @param {string} text
 * @param {number} x
 * @param {number} y
 */
export function drawText(text, x, y) {
  const ctx = mainCanvas.getContext("2d", { alpha: false });
  ctx.font = "13pt monospace"

  const metrics = ctx.measureText(text);
  const rectHeight = metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent;
  drawRect(false, x, y - rectHeight, metrics.width, rectHeight, 0x00, 0x66, 0x00);
  
  ctx.fillStyle = "white";
  ctx.fillText(text, x, y);
}

/**
 * @param {number} width
 * @param {number} height
 */
export function setFixedSize(width, height) {
  deinitResizeEvent();
  mainCanvas.width = tempCanvas.width = width;
  mainCanvas.height = tempCanvas.height = height;
}
