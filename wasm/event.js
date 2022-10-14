const ev_none = 0;
const ev_keypress = 1;
const ev_buttondown = 2;
const ev_buttonup = 3;
const ev_motion = 4;
const ev_resize = 5;
const ev_expose = 6;

const KEY_MAPPINGS = {
  "F1": 0x101,
  "F2": 0x102,
  "F3": 0x103,
  "F4": 0x104,
  "F5": 0x105,
  "F6": 0x106,
  "F7": 0x107,
  "F8": 0x108,
  "F9": 0x109,
  "F10": 0x10a,
  "F11": 0x10b,
  "F12": 0x10c,
  "Delete": 0x200,
  "Backspace": 0x200,
  "ArrowUp": 0x201,
  "ArrowDown": 0x202,
  "ArrowLeft": 0x203,
  "ArrowRight": 0x204,
  "PageUp": 0x205,
  "PageDown": 0x206,
  "Home": 0x207,
};

const EVENT_STATE_INITIALIZING = 0;
const EVENT_STATE_EXPOSING = 1;
const EVENT_STATE_NORMAL = 2;

let eventState = EVENT_STATE_INITIALIZING;
/** @type Function */ let wakeUpFn;
/** @type HTMLCanvasElement */ let canvasObj;
let setValueFn;
let ptrObj;
let isMouseDown = false;
/** @type MouseEvent */ let lastMouseEvent;

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
    canvasObj.addEventListener("keypress", onCanvasKeyPress);

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
  lastMouseEvent = event;
  isMouseDown = true;
  makeMouseEvent(ev_buttondown)
}

/**
 * @param {MouseEvent} event
 */
function onCanvasDrag(event) {
  lastMouseEvent = event;
  if (isMouseDown) {
    makeMouseEvent(ev_motion);
  }
}

/**
 * @param {MouseEvent} event
 */
function onCanvasMouseUp(event) {
  lastMouseEvent = event;
  isMouseDown = false;
  makeMouseEvent(ev_buttonup);
}

function makeMouseEvent(type) {
  let button = 1;
  switch (lastMouseEvent.button) {
    case 0: button = 1; break;
    case 1: button = 2; break;
    case 2: button = 3; break;
  }

  const rect = canvasObj.getBoundingClientRect();
  setEventPointer(type, generateMouseEvent());
  wakeUpFn();
}

function generateMouseEvent(button) {
  const rect = canvasObj.getBoundingClientRect();
  return {
    "button": button,
    "x": lastMouseEvent.clientX - rect.left,
    "y": lastMouseEvent.clientY - rect.top,
    "time": Date.now(),
  };
}

/**
 * @param {KeyboardEvent} event
 */
function onCanvasKeyPress(event) {
  //Stop quit events from going through
  if (event.key == 'q') {
    return;
  }

  const mouseEvent = generateMouseEvent(null);
  let key = 0;
  if (event.key.length == 1) {
    key = event.key.charCodeAt(0);
  }
  else if (event.key in KEY_MAPPINGS) {
    key = KEY_MAPPINGS[event.key];
  }

  setEventPointer(ev_keypress, {
    "key": key,
    "x": mouseEvent.x,
    "y": mouseEvent.y,
  });
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
