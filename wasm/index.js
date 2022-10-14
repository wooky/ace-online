import createAce from "@build/ace-online";
import { initDrawer } from "@/drawer";
import { initEvents } from "@/event";
import { loadImages } from "@/imagelib";

(function () {
  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  initDrawer(canvas);
  initEvents(canvas);

  createAce({ noInitialRun: true }).then((Module) => {
    loadImages(Module);
    Module._setTableSize(canvas.width, canvas.height);
    Module._main();
  });
})();
