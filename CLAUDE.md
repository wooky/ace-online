# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

This branch (`master`) contains **only build files** — it has no game source code of its own. The actual Ace of Penguins source (a frozen ~1998–2015 CVS import of DJ Delorie's games) lives on the **`upstream` branch** and is fetched at CMake configure time via `FetchContent`. The goal of this repo is to build those games with a modern CMake/Docker toolchain and combine them into a single launcher executable.

So the tracked files are: `Dockerfile`, `CMakeLists.txt`, `src/launcher.c`, `tools/patch-sources.sh`. When you need to understand a game's behaviour, read it from the upstream branch (e.g. `git show origin/upstream:lib/table.c`) or, after a build, from `build/_deps/acesrc-src/`.

## Build & run

The build environment is Alpine 3.18 on purpose: it ships **GCC 12**, which treats the legacy C idioms in the upstream source (implicit function declarations, `isgraph` without `<ctype.h>`, etc.) as *warnings*. GCC 14+ makes those hard errors, so do not "upgrade" the base image without adding `-Wno-error=` flags. `-fcommon` (in `CMakeLists.txt`) is likewise required for the pre-GCC-10 tentative-definition behaviour.

```sh
# Build the image (toolchain only — it does NOT bake in the source/build)
docker build -t ace-online-build .

# Build the project; CMakeLists.txt is mounted in, artifacts land in ./build on the host
docker run --rm -v "$PWD":/src ace-online-build
# -> ./build/ace  (the combined launcher + all 12 games)
```

There is also a WASM target (issue #161). The image carries emscripten (installed from GitHub; it uses Alpine's native `node` since the bundled glibc node won't run on musl). Configure with `emcmake` into a separate build dir:

```sh
docker run --rm -v "$PWD":/src ace-online-build \
  sh -c 'emcmake cmake -S . -B build-wasm && cmake --build build-wasm -j"$(nproc)"'
# -> ./build-wasm/ace.html (+ .js/.wasm)
```

The WASM build is scaffolding: it compiles and links but does not render yet (`src/wasm/xwin.c` is a no-op shim). It is gated on the CMake `EMSCRIPTEN` variable.

`./build` and `./build-wasm` are created root-owned by the container; remove them with `docker run --rm -v "$PWD":/src ace-online-build sh -c 'rm -rf /src/build /src/build-wasm'` rather than host `rm`.

The `PATCH_COMMAND` only runs on a fresh `FetchContent` populate. **After editing `tools/patch-sources.sh` you must wipe `build/` (above) so the upstream source is re-fetched and re-patched** — the script is also marker-file guarded (`.launcher-patched`) so it never double-applies.

`ace` is an X11 GUI program. On WSL2 it needs WSLg; binaries built in the musl/Alpine container only run inside that container (musl loader), so run them with the X socket mounted:

```sh
docker run --rm -e DISPLAY=:0 -v /tmp/.X11-unix:/tmp/.X11-unix -v "$PWD":/src \
  ace-online-build sh -c '/src/build/ace'
```

## Testing changes (there is no unit-test suite)

Verification is visual. Run `ace` headless under Xvfb plus a real window manager and screenshot. A window manager (e.g. `openbox`) is **required** — without one there is no input focus, so `xdotool key` silently no-ops, and windows stack at (0,0) hiding window-creation bugs. Click coordinates must be offset by the window's geometry (`xdotool getwindowgeometry --shell`) to account for the WM title bar.

```sh
# inside a throwaway container, with /src mounted:
echo "https://dl-cdn.alpinelinux.org/alpine/v3.18/community" >> /etc/apk/repositories
apk add xvfb xdotool imagemagick openbox
Xvfb :99 -screen 0 1000x900x24 & export DISPLAY=:99; openbox &
/src/build/ace &
import -window root /src/build/shot.png   # screenshot for inspection
```

## Architecture

### Two-stage build (`CMakeLists.txt`)

The upstream autotools build compiles two helper programs and *runs them at build time* to generate C source the games compile against. This is reproduced as custom commands:

- **`make-imglib`** (`lib/make-imglib.c`, links png/zlib) turns PNGs into C image arrays. Two invocation modes: an `.in`-listing mode that produces `lib/images.c` (the shared "cards" deck), and a `-` command-line mode that produces each `<game>-img.c`.
- **`text2c`** (`lib/text2c.c`) turns each `<game>.html` into `<game>-help.c`.

`libcards` (the support library: `table.c help.c stack.c imagelib.c xwin.c table_rn.c funcs.c images.c`) is built **static**, then each game and the launcher link against it.

### The combined launcher (issue #160)

All 12 games + the launcher build into one executable `ace`. Two problems are solved here:

1. **Symbol collisions.** Every game's generated `-img.c` defines the same global `appimglib_imagelib`, and games share other globals (e.g. `grid` in merlin/minesweeper/pegged, taipeilib symbols in taipei/taipedit). Each game is compiled to an OBJECT library with `-Dappimglib_imagelib=<game>_appimglib` (so it references its own image table). The *remaining* collisions are handled per target (see WASM section): natively each game object is partial-linked (`cc -r`) and run through `objcopy --keep-global-symbol=<game>_main` so per-game globals stay isolated. Verify with `nm build/gameobj/<game>.o` (one uppercase `<game>_main`).

2. **Source rewriting via FetchContent `PATCH_COMMAND`** (`tools/patch-sources.sh`). It rewrites the fetched upstream sources to: rename each `main()` → `<game>_main()`; redirect `exit(...)` → `ace_return_to_launcher(...)` (regex `[^A-Za-z0-9_]exit(` so `atexit` is untouched); and append reset/reuse hooks (below).

### The X11 framework and why launcher re-entry is hard

The games run on a shared framework in `lib/` that is **heavily global-state and single-window**:

- `init_ace()` parses options and installs per-game callbacks into global function pointers in `funcs.c`; `init_table()` creates the window; `table_loop()` is the blocking event loop. Callbacks (`click_cb`, `redraw_cb`, …) are globals — `init_ace` only *sets* the ones a game lists and never resets the others.
- The launcher (`src/launcher.c`) draws the menu (white via `text()` on the framework's green `table_background`), and on click runs `<game>_main()` inside a `setjmp`. A game "exit" longjmps back via `ace_return_to_launcher()`.

Making one window/connection serve every game required these patches (appended to the upstream source by `patch-sources.sh`) — each fixes a concrete, non-obvious bug:

- **`xwin.c` reuse:** `xwin_init` returns early if the display is already open (else `XOpenDisplay` is called again and the global `display`/`window` get clobbered, spawning a new window per game). `xwin_create` resizes the existing window instead of `XCreateWindow`, and **resets WM size hints to `PSize`** — the launcher's `table_no_resize` pins `PMinSize==PMaxSize`, which makes a real WM *refuse* the next game's resize.
- **`ace_reset_table()` (table.c):** resets the static `initted` one-shot guard (or the next game's `init_cb` never runs), `no_resize`, `centered_pic`, `help_is_showing`, and **zeroes `table_width`/`table_height`** so each game sizes its own window (games only self-size when these are 0).
- **`ace_reset_funcs()` (funcs.c):** restores all callback pointers to defaults so callbacks don't bleed between games.
- **`ace_reset_stacks()` (stack.c):** frees the file-global `stacks` list (and drag/undo state). Without it a card game inherits the previous card game's stacks and `stack_redraw` paints their cards onto the new game — the "cards from previous games" bleed. (Only card games hit this; non-card games don't call `stack_redraw`.)

`register_imagelib` is idempotent and `get_image` returns the first name match (later-registered games shadow earlier), so image registration does not need resetting.

### The WASM build (issue #161, scaffolding)

Gated on `if(EMSCRIPTEN)` in CMakeLists.txt. It diverges from native in four ways, each forced by a real emscripten limitation:

- **Codegen tools run on the host.** `make-imglib`/`text2c` can't run as wasm during the build, so under emscripten they're compiled with the host `cc` (found via `find_program`) and run natively; the byte-array C they emit compiles fine under emcc. They're wrapped in custom *targets* (`make_imglib_tool`/`text2c_tool`) — depending on the bare output file instead makes `make -j` rebuild the tool concurrently and run a half-written binary.
- **No X11/libpng.** `lib/xwin.c` is swapped for the no-op shim `src/wasm/xwin.c`, and `<X11/*.h>` resolve to stubs in `src/wasm/stubs/` (dummy types + inline no-op functions) so the files that use X11 directly — `lib/help.c`, `games/taipedit.c` (also golf/pegged) — still compile. Nothing image/X11 is linked.
- **`-fcommon` is unsupported on wasm** ("common symbols are not yet implemented for Wasm"), so the WASM build compiles `-fno-common` and links with `-Wl,--allow-multiple-definition`, which absorbs the legacy duplicate tentative definitions (semantically equivalent to the `-fcommon` merge — all refs bind to the first definition). This replaces the native `objcopy` localization, which `llvm-objcopy` can't do on wasm objects.
- The shim leaves the program non-interactive for now; "implementation details added later" per the issue.
