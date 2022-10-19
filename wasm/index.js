import createAce from "@build/ace-online";
import { initDrawer, resizeCanvases } from "@/drawer";
import { initEvents } from "@/event";
import games from "@/games.json";

(async function () {
  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  canvas.addEventListener("contextmenu", e => {
    e.preventDefault();
    return false;
  });
  await initDrawer(canvas);

  const gameList = document.getElementById("games");
  for (const game of games) {
    const a = document.createElement("a");
    a.href = "#";
    a.onclick = e => loadGame(e, game.name);
    a.text = game.english;
    const li = document.createElement("li");
    li.appendChild(a);
    gameList.appendChild(li);
  }
})();

/**
 * @param {Event} event
 * @param {String} name
 */
async function loadGame(event, name) {
  event.preventDefault();
  setPage("loading");

  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  initEvents(canvas);

  createAce({ noInitialRun: true }).then((Module) => {
    setPage("playfield");
    resizeCanvases();
    Module[`_${name}_main`]();
  });
}

/**
 * @param {string} name
 */
function setPage(name) {
  for (const el of document.getElementById("content").children) {
    el.id === name ? el.classList.remove("is-hidden") : el.classList.add("is-hidden");
  }
}
