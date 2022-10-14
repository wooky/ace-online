import createAce from "@build/ace-online";
import { initDrawer } from "@/drawer";
import { initEvents } from "@/event";
import { loadImages } from "@/imagelib";

(async function () {
  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  canvas.addEventListener("contextmenu", e => {
    e.preventDefault();
    return false;
  });
  await initDrawer(canvas);
  initEvents(canvas);

  createAce({ noInitialRun: true }).then((Module) => {
    loadImages(Module);
    Module._main();
  });
})();
