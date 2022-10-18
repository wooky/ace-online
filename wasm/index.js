import createAce from "@build/ace-online";
import { initDrawer, resizeCanvases } from "@/drawer";
import { emitResizeEvent, initEvents } from "@/event";

(async function () {
  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  canvas.addEventListener("contextmenu", e => {
    e.preventDefault();
    return false;
  });
  await initDrawer(canvas);
  initEvents(canvas);

  createAce({ noInitialRun: true }).then((Module) => {
    Module._thornq_main();
    window.addEventListener("resize", onWindowResize);
  });
})();

function onWindowResize() {
  resizeCanvases();
  emitResizeEvent();
}
