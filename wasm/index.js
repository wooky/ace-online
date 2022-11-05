import createAce from "@build/ace-online";
import { initDrawer, resizeCanvases } from "@/drawer";
import { initEvents, sendKeyPress } from "@/event";
import games from "@/games.json";
import { initBeeper } from "./beeper";

const gameButtons = document.getElementById("game-buttons");

(async function () {
  if ("serviceWorker" in navigator) {
    try {
      const registration = await navigator.serviceWorker.register("service-worker.js", {
        scope: ".",
      });
    }
    catch (e) {
      console.log("service-worker registration failed:", e);
    }
  }

  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  canvas.addEventListener("contextmenu", e => {
    e.preventDefault();
    return false;
  });
  await initDrawer(canvas);

  /** @type {HTMLCollectionOf<HTMLAnchorElement>} */ const gameLinks = document.getElementsByClassName("game-link");
  for (let i = 0; i < gameLinks.length; i++) {
    gameLinks[i].onclick = e => loadGame(e, gameLinks[i].dataset.gamename);
  }
})();

/**
 * @param {Event} event
 * @param {String} name
 */
function loadGame(event, name) {
  event.preventDefault();
  setPage("loading");

  for (const icon of games[name].icons) {
    const a = document.createElement("a");
    a.href = "#";
    a.onclick = e => sendKeyPress(e, icon.key);
    a.text = icon.icon;
    a.title = icon.english;
    a.className = "homelink emoji";
    gameButtons.appendChild(a);
  }

  /** @type HTMLCanvasElement */ const canvas = document.getElementById("game");
  initEvents(canvas, unloadGame);
  initBeeper(canvas);

  createAce({ noInitialRun: true }).then((Module) => {
    setPage("playfield");
    resizeCanvases();
    Module[`_${name}_main`]();
  });
}

/**
 * @param {Event} event
 */
function unloadGame() {
  gameButtons.textContent = "";
  setPage("description");
}

/**
 * @param {string} name
 */
function setPage(name) {
  for (const el of document.getElementById("content").children) {
    el.id === name ? el.classList.remove("is-hidden") : el.classList.add("is-hidden");
  }
}
