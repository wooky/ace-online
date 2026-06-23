# Ace of Penguins — CMake / WebAssembly build

Build files for [DJ Delorie's Ace of Penguins](https://github.com/wooky/ace-online)
solitaire collection. The 12 games are combined into a single launcher
executable, built either as a **native X11 program** or as a **WebAssembly
program**.

The game source itself is **not** in this branch — it is fetched automatically
from the `upstream` branch at build time (CMake `FetchContent`). All you need
here are `Dockerfile`, `CMakeLists.txt`, `src/`, and `tools/`.

## Prerequisites

Everything builds inside Docker, so the only requirement is **Docker**. The
image bundles both toolchains (native GCC + X11/libpng, and emscripten).

Build the image once:

```sh
docker build -t ace-online-build .
```

The build files are mounted into the container at run time, so the compiled
output is written back to your working directory.

## Build for the native system

```sh
docker run --rm -v "$PWD":/src ace-online-build
```

Output: `./build/ace` — the combined launcher with all 12 games.

This is equivalent to running, inside the container:

```sh
cmake -S . -B build
cmake --build build -j"$(nproc)"
```

## Build for WebAssembly

Configure with `emcmake` (which selects the emscripten toolchain) into a
separate build directory:

```sh
docker run --rm -v "$PWD":/src ace-online-build \
  sh -c 'emcmake cmake -S . -B build-wasm && cmake --build build-wasm -j"$(nproc)"'
```

Output in `./build-wasm/`:

| file        | description                          |
|-------------|--------------------------------------|
| `ace.html`  | host page that loads the module      |
| `ace.js`    | JavaScript loader / runtime glue     |
| `ace.wasm`  | the compiled WebAssembly module      |

> **Note:** the WebAssembly target is currently scaffolding — it compiles and
> links, but does not render yet (the rendering backend is a stub). The native
> build is fully playable.

## Running the native build

`ace` is an X11 GUI program. The binaries are built inside the container, so
the simplest way to run them is from the container, pointing at your host's X
server. On WSL2 this is WSLg (`DISPLAY=:0`):

```sh
docker run --rm \
  -e DISPLAY="$DISPLAY" \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$PWD":/src \
  ace-online-build /src/build/ace
```

A window listing the games appears; click a game to play it, and press `q` (or
Esc) in a game to return to the launcher.

## Cleaning

The `build/` and `build-wasm/` directories are created by the container as
root, so remove them through the container rather than with a host `rm`:

```sh
docker run --rm -v "$PWD":/src ace-online-build sh -c 'rm -rf /src/build /src/build-wasm'
```
