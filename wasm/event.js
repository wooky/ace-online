import { resizeCanvases } from "@/drawer";

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

const CANVAS_EVENTS = {
  "mousedown": onCanvasMouseDown,
  "touchstart": onCanvasMouseDown,
  "mousemove": onCanvasDrag,
  "touchmove": onCanvasDrag,
  "mouseup": onCanvasMouseUp,
  "touchend": onCanvasMouseUp,
  "keydown": onCanvasKeyPress,
};

/** @type number */ let eventState;
/** @type Function */ let wakeUpFn;
/** @type HTMLCanvasElement */ let canvasObj;
/** @type Function */ let quitFn;
let setValueFn;
let ptrObj;
/** @type boolean */ let isMouseDown = false;
/**
 * @typedef LastMouseEvent
 * @type {Object}
 * @property {number} x
 * @property {number} y
 * @property {number} button
 */
/** @type LastMouseEvent */ let lastMouseEvent;

/**
 * @param {HTMLCanvasElement} canvas
 * @param {Function} quit
 */
export function initEvents(canvas, quit) {
  canvasObj = canvas;
  quitFn = quit;
  eventState = EVENT_STATE_INITIALIZING;
  isMouseDown = false;
}

/**
 * @param {Event} otherEvent
 */
export function sendExitEvent(otherEvent) {
  otherEvent.preventDefault();
  canvasObj.dispatchEvent(new KeyboardEvent("keydown", { key: 'q' }));
}

export function setUpEvents(wakeUp, setValue, ptr) {
  wakeUpFn = wakeUp;
  if (eventState === EVENT_STATE_INITIALIZING) {
    setValueFn = setValue;
    ptrObj = ptr;
    for (const event in CANVAS_EVENTS) {
      canvasObj.addEventListener(event, CANVAS_EVENTS[event]);
    }
    window.addEventListener("resize", onWindowResize);

    emitResizeEvent();
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

export function deinitResizeEvent() {
  window.removeEventListener("resize", onWindowResize);
}

function emitResizeEvent() {
  eventState = EVENT_STATE_EXPOSING;
  setEventPointer(ev_resize, {
    "x": 0,
    "y": 0,
    "w": canvasObj.width,
    "h": canvasObj.height,
  });
  wakeUpFn();
}

/**
 * @param {MouseEvent|TouchEvent} event 
 */
function onCanvasMouseDown(event) {
  setLastEvent(event);
  isMouseDown = true;
  makeMouseEvent(ev_buttondown)
}

/**
 * @param {MouseEvent|TouchEvent} event
 */
function onCanvasDrag(event) {
  setLastEvent(event);
  if (isMouseDown) {
    event.preventDefault();
    makeMouseEvent(ev_motion);
  }
}

/**
 * @param {MouseEvent|TouchEvent} event
 */
function onCanvasMouseUp(event) {
  event.preventDefault();
  setLastEvent(event);
  isMouseDown = false;
  makeMouseEvent(ev_buttonup);
}

/**
 * @param {MouseEvent|TouchEvent} event
 */
function setLastEvent(event) {
  if (event instanceof MouseEvent) {
    lastMouseEvent = {
      x: event.clientX,
      y: event.clientY,
      button: event.button,
    };
  }
  else if (event instanceof TouchEvent) {
    const touchEvent = event.changedTouches.item(0);
    lastMouseEvent = {
      x: touchEvent.clientX,
      y: touchEvent.clientY,
      button: 0,
    };
  }
}

function makeMouseEvent(type) {
  let button = 1;
  switch (lastMouseEvent.button) {
    case 0: button = 1; break;
    case 1: button = 2; break;
    case 2: button = 3; break;
  }

  setEventPointer(type, generateMouseEvent(button));
  wakeUpFn();
}

function generateMouseEvent(button) {
  const rect = canvasObj.getBoundingClientRect();
  return {
    "button": button,
    "x": lastMouseEvent.x - rect.left,
    "y": lastMouseEvent.y - rect.top,
    "time": Date.now(),
  };
}

/**
 * @param {KeyboardEvent} event
 */
function onCanvasKeyPress(event) {
  const mouseEvent = generateMouseEvent(null);
  let key = 0;
  if (event.key.length == 1) {
    key = event.key.charCodeAt(0);
  }
  else if (event.key in KEY_MAPPINGS) {
    key = KEY_MAPPINGS[event.key];
  }
  else {
    return;
  }

  setEventPointer(ev_keypress, {
    "key": key,
    "x": mouseEvent.x,
    "y": mouseEvent.y,
  });
  try {
    wakeUpFn();
  }
  catch (e) {
    if (e.name === "ExitStatus" && e.status === 0) {
      onExit();
    }
    else {
      throw e;
    }
  }
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

function onWindowResize() {
  resizeCanvases();
  emitResizeEvent();
}

function onExit() {
  for (const event in CANVAS_EVENTS) {
    canvasObj.removeEventListener(event, CANVAS_EVENTS[event]);
  }
  deinitResizeEvent();
  quitFn();
}
