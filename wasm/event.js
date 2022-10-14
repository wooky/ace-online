const ev_none = 0;
const ev_keypress = 1;
const ev_buttondown = 2;
const ev_buttonup = 3;
const ev_motion = 4;
const ev_resize = 5;
const ev_expose = 6;
const ev_quit = 7;

const EVENT_STATE_INITIALIZING = 0;
const EVENT_STATE_EXPOSING = 1;
const EVENT_STATE_NORMAL = 2;

let eventState = EVENT_STATE_INITIALIZING;
/** @type Function */ let wakeUpFn;
/** @type HTMLCanvasElement */ let canvasObj;
let setValueFn;
let ptrObj;
let isMouseDown = false;

/**
 * @param {HTMLCanvasElement} canvas 
 */
export function initEvents(canvas) {
  canvasObj = canvas;
}

export function setUpEvents(wakeUp, setValue, ptr) {
  wakeUpFn = wakeUp;
  if (eventState === EVENT_STATE_INITIALIZING) {
    eventState = EVENT_STATE_EXPOSING;
    setValueFn = setValue;
    ptrObj = ptr;
    canvasObj = document.getElementById("game");
    canvasObj.addEventListener("mousedown", onCanvasMouseDown);
    canvasObj.addEventListener("mousemove", onCanvasDrag);
    canvasObj.addEventListener("mouseup", onCanvasMouseUp);

    setEventPointer(ev_resize, {
      "x": 0,
      "y": 0,
      "w": canvasObj.width,
      "h": canvasObj.height,
    });
    wakeUpFn();
  }
  else if (eventState === EVENT_STATE_EXPOSING) {
    eventState = EVENT_STATE_NORMAL;
    setEventPointer(ev_expose, {
      "x": 0,
      "y": 0,
      "w": canvasObj.width,
      "h": canvasObj.height,
    });
    wakeUpFn();
  }
  else {
    // Just in case
    setEventPointer(ev_none, {});
  }
}

/**
 * @param {MouseEvent} event 
 */
function onCanvasMouseDown(event) {
  isMouseDown = true;
  makeMouseEvent(event, ev_buttondown)
}

/**
 * @param {MouseEvent} event
 */
function onCanvasDrag(event) {
  if (isMouseDown) {
    makeMouseEvent(event, ev_motion);
  }
}

/**
 * @param {MouseEvent} event
 */
function onCanvasMouseUp(event) {
  isMouseDown = false;
  makeMouseEvent(event, ev_buttonup);
}

/**
 * @param {MouseEvent} event
 */
function makeMouseEvent(event, type) {
  let button = 1;
  switch (event.button) {
    case 0: button = 1; break;
    case 1: button = 2; break;
    case 2: button = 3; break;
  }

  const rect = canvasObj.getBoundingClientRect();
  setEventPointer(type, {
    "button": button,
    "x": event.clientX - rect.left,
    "y": event.clientY - rect.top,
    "time": Date.now(),
  })
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
