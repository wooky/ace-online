# Build environment for the Ace of Penguins CMake port.
#
# Alpine 3.18 ships GCC 12, which still treats the legacy C idioms in the
# ~1998-era Ace of Penguins sources (implicit function declarations, etc.)
# as warnings rather than hard errors -- so the unmodified upstream source
# compiles cleanly here.
#
# This image carries two toolchains:
#   * the native one (gcc + X11 + libpng) for the X11 `ace` executable, and
#   * emscripten (installed from GitHub) for the WASM build (issue #161).
#
# The build files (CMakeLists.txt etc.) are mounted in at /src at run time.
#
#   docker build -t ace-online-build .
#   # native -> ./build/ace
#   docker run --rm -v "$PWD":/src ace-online-build
#   # WASM   -> ./build-wasm/ace.html (+ .js/.wasm)
#   docker run --rm -v "$PWD":/src ace-online-build \
#     sh -c 'emcmake cmake -S . -B build-wasm && cmake --build build-wasm -j"$(nproc)"'
FROM alpine:3.18

# Native toolchain + libraries, plus emsdk prerequisites.  emsdk ships
# glibc-built binaries; gcompat lets its LLVM run on musl, but its bundled
# node does not work -- so we install Alpine's native nodejs and point
# emscripten at it (see the sed below).
RUN apk add --no-cache \
        cmake make gcc musl-dev git \
        libpng-dev zlib-dev libx11-dev \
        python3 bash xz gcompat libstdc++ nodejs

# Install emscripten from GitHub.
RUN git clone --depth 1 https://github.com/emscripten-core/emsdk.git /emsdk \
 && cd /emsdk && ./emsdk install latest && ./emsdk activate latest \
 && sed -i 's#^NODE_JS.*#NODE_JS = "/usr/bin/node"#' /emsdk/.emscripten

ENV EMSDK=/emsdk \
    EM_CONFIG=/emsdk/.emscripten \
    EMSDK_NODE=/usr/bin/node \
    PATH=/emsdk:/emsdk/upstream/emscripten:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

WORKDIR /src

# Default: the native build.  (For WASM, override the command with emcmake as
# shown in the header comment.)
CMD ["sh", "-c", "cmake -S . -B build && cmake --build build -j\"$(nproc)\""]
