const ev_none = 0;
const ev_keypress = 1;
const ev_buttondown = 2;
const ev_buttonup = 3;
const ev_motion = 4;
const ev_resize = 5;
const ev_expose = 6;
const ev_quit = 7;

/** @type Function */ let wakeUpFn = null;
/** @type HTMLCanvasElement */ let canvasObj;
let setValueFn;
let ptrObj;

/**
 * @param {HTMLCanvasElement} canvas 
 */
export function initEvents(canvas) {
  canvasObj = canvas;
}

export function setUpEvents(wakeUp, setValue, ptr) {
  const initialize = !wakeUpFn;
  wakeUpFn = wakeUp;
  if (initialize) {
    setValueFn = setValue;
    ptrObj = ptr;
    canvasObj = document.getElementById("game");
    canvasObj.addEventListener("click", onCanvasClick);
    setEventPointer(ev_expose, {
      "x": 0,
      "y": 0,
      "w": canvasObj.width,
      "h": canvasObj.height,
    });
    wakeUpFn();
  }
}

/**
 * @param {MouseEvent} event 
 */
function onCanvasClick(event) {
  wakeUpFn();
}

function setEventPointer(type, {
  x = 0,
  y = 0,
  w = 0,
  h = 0,
  button = 0,
  shifts = 0,
  key = 0,
  time = 0,
}) {
  setValueFn(ptrObj + 0x00, type, 'i32');
  setValueFn(ptrObj + 0x04, x, 'i32');
  setValueFn(ptrObj + 0x08, y, 'i32');
  setValueFn(ptrObj + 0x0c, w, 'i32');
  setValueFn(ptrObj + 0x10, h, 'i32');
  setValueFn(ptrObj + 0x14, button, 'i32');
  setValueFn(ptrObj + 0x18, shifts, 'i32');
  setValueFn(ptrObj + 0x1c, key, 'i32');
  setValueFn(ptrObj + 0x20, time, 'i32');
}