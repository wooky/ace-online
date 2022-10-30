const CLASS_BEEPING = "is-beeping";

/** @type HTMLCanvasElement */ let canvasObj;

/**
 * @param {HTMLCanvasElement} canvas
 */
export function initBeeper(canvas) {
  canvasObj = canvas;
}

export function beep() {
  window.navigator.vibrate(10);
  canvasObj.classList.add(CLASS_BEEPING);
  setTimeout(() => canvasObj.classList.remove(CLASS_BEEPING), 500);
}
