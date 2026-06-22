# Build environment for the Ace of Penguins CMake port.
#
# Alpine 3.18 ships GCC 12, which still treats the legacy C idioms in the
# ~1998-era Ace of Penguins sources (implicit function declarations, etc.)
# as warnings rather than hard errors -- so the unmodified upstream source
# compiles cleanly here.
#
# This image only provides the toolchain; the build files (CMakeLists.txt)
# are mounted in from the host at run time so the build output is written
# back to the host.  Build with:
#
#   docker build -t ace-online-build .
#   docker run --rm -v "$PWD":/src ace-online-build
#
# Artifacts then appear in ./build on the host.
FROM alpine:3.18

RUN apk add --no-cache \
        cmake \
        make \
        gcc \
        musl-dev \
        git \
        libpng-dev \
        zlib-dev \
        libx11-dev

WORKDIR /src

# The host directory holding the build files (and receiving the output) is
# expected to be mounted at /src.  The game sources themselves are fetched
# from the upstream branch by CMake's FetchContent at configure time.
CMD ["sh", "-c", "cmake -S . -B build && cmake --build build -j\"$(nproc)\""]
