# CMake port of the Ace of Penguins autotools build.
#
# The original project is built with autoconf/automake (configure.in +
# Makefile.am).  The defining feature of that build is that two helper
# programs are compiled first and then run to *generate* C source that the
# real programs are compiled against:
#
#   * lib/make-imglib  - turns PNG images into C arrays (the image libraries)
#   * lib/text2c       - turns HTML help pages into C string arrays
#
# This file reproduces that two-stage build with CMake and combines all the
# games into a single launcher executable.  The actual source code is fetched
# from the `upstream` branch of the upstream git repository at configure time.
#
# Two targets are supported from the same file:
#   * native  : an X11 executable `ace`            (default toolchain)
#   * WASM     : `ace.html`/.js/.wasm via emscripten (configure with `emcmake`)
# See the EMSCRIPTEN branches below and the Dockerfile for how WASM is built.

cmake_minimum_required(VERSION 3.14)
project(ace-online C)

# --------------------------------------------------------------------------
# Fetch the Ace of Penguins source (upstream branch).
# --------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  acesrc
  GIT_REPOSITORY https://github.com/wooky/ace-online.git
  GIT_TAG        upstream
  GIT_SHALLOW    TRUE
  # Rewrite the fetched sources for the combined launcher build (issue #160):
  # rename each game's main() -> <game>_main() and redirect exit() back to the
  # launcher.  This is the "FetchContent patch capability" the issue calls for.
  PATCH_COMMAND  sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/patch-sources.sh
)
FetchContent_GetProperties(acesrc)
if(NOT acesrc_POPULATED)
  FetchContent_Populate(acesrc)
endif()

set(SRC ${acesrc_SOURCE_DIR})
set(GEN ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN}/lib ${GEN}/games ${CMAKE_BINARY_DIR}/gameobj)

# --------------------------------------------------------------------------
# Dependencies.  Only the native build links X11/libpng/zlib; the WASM build
# has none of them (its xwin.c is a shim and it links no image libraries).
# --------------------------------------------------------------------------
if(NOT EMSCRIPTEN)
  find_package(PNG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(X11 REQUIRED)
endif()

# --------------------------------------------------------------------------
# This is ~1998-era C.  Keep its rejected-by-default idioms as warnings.
# -fcommon (pre-GCC-10 tentative-definition behaviour) is required by the
# source, but clang/wasm does not implement common symbols -- so for WASM we
# instead compile -fno-common and tolerate the duplicate definitions at link
# time with --allow-multiple-definition (see the `ace` target below).
# --------------------------------------------------------------------------
add_compile_options(
  -Wno-implicit-function-declaration
  -Wno-implicit-int
  -Wno-int-conversion
  -Wno-incompatible-pointer-types
)
if(NOT EMSCRIPTEN)
  add_compile_options(-fcommon)
endif()

# ==========================================================================
# Stage 1: the code-generation tools.
#
# The tools must run on the BUILD machine to generate C source.  For the
# native build they are ordinary targets.  For WASM they cannot run as wasm
# during the build, so (per issue #161, option 1) they are compiled with the
# host C compiler and run natively; the C they emit is plain byte arrays that
# compile fine under emcc.
# ==========================================================================
if(EMSCRIPTEN)
  find_program(HOST_CC NAMES cc gcc clang REQUIRED)
  set(MAKE_IMGLIB ${CMAKE_BINARY_DIR}/host-tools/make-imglib)
  set(TEXT2C      ${CMAKE_BINARY_DIR}/host-tools/text2c)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/host-tools)
  add_custom_command(
    OUTPUT ${MAKE_IMGLIB}
    COMMAND ${HOST_CC} -w -fcommon ${SRC}/lib/make-imglib.c -o ${MAKE_IMGLIB} -lpng -lz
    DEPENDS ${SRC}/lib/make-imglib.c VERBATIM)
  add_custom_command(
    OUTPUT ${TEXT2C}
    COMMAND ${HOST_CC} -w ${SRC}/lib/text2c.c -o ${TEXT2C}
    DEPENDS ${SRC}/lib/text2c.c VERBATIM)
  # Wrap the tool outputs in targets so each is built ONCE; depending on the
  # bare file from multiple codegen commands makes `make -j` rebuild the tool
  # concurrently and run a half-written binary.
  add_custom_target(make_imglib_tool DEPENDS ${MAKE_IMGLIB})
  add_custom_target(text2c_tool      DEPENDS ${TEXT2C})
  set(MAKE_IMGLIB_DEP make_imglib_tool)
  set(TEXT2C_DEP      text2c_tool)
  set(TEXT2C_RUN      ${TEXT2C})
else()
  add_executable(make-imglib ${SRC}/lib/make-imglib.c)
  target_link_libraries(make-imglib PRIVATE PNG::PNG ZLIB::ZLIB)
  add_executable(text2c ${SRC}/lib/text2c.c)
  set(MAKE_IMGLIB     make-imglib)    # target name: resolves + adds dependency
  set(MAKE_IMGLIB_DEP make-imglib)
  set(TEXT2C_DEP      text2c)
  set(TEXT2C_RUN      $<TARGET_FILE:text2c>)
endif()

# ==========================================================================
# Stage 2: code generation (identical for both targets -- the tools run on
# the host either way).
# ==========================================================================
add_custom_command(
  OUTPUT ${GEN}/lib/images.c
  COMMAND ${MAKE_IMGLIB} -n cards -i ${SRC}/lib/png
          -d ${GEN}/lib/images.d
          ${SRC}/lib/images.in ${GEN}/lib/images.c
  DEPENDS ${MAKE_IMGLIB_DEP} ${SRC}/lib/images.in
  VERBATIM
)

function(add_imglib outvar dir name)
  set(pngpaths "")
  foreach(p ${ARGN})
    list(APPEND pngpaths ${SRC}/${dir}/${p})
  endforeach()
  add_custom_command(
    OUTPUT ${GEN}/${dir}/${name}-img.c
    COMMAND ${MAKE_IMGLIB} -i ${SRC}/${dir} -n appimglib -
            ${GEN}/${dir}/${name}-img.c ${ARGN}
    DEPENDS ${MAKE_IMGLIB_DEP} ${pngpaths}
    VERBATIM
  )
  set(${outvar} ${GEN}/${dir}/${name}-img.c PARENT_SCOPE)
endfunction()

function(add_help outvar name)
  add_custom_command(
    OUTPUT ${GEN}/games/${name}-help.c
    COMMAND sh -c "${TEXT2C_RUN} ${name}_help < ${SRC}/games/${name}.html > ${GEN}/games/${name}-help.c"
    DEPENDS ${TEXT2C_DEP} ${SRC}/games/${name}.html
    VERBATIM
  )
  set(${outvar} ${GEN}/games/${name}-help.c PARENT_SCOPE)
endfunction()

# ==========================================================================
# Stage 3: libcards (the support library the launcher and every game use).
# For WASM the X11+libpng backend xwin.c is swapped for the shim and the X11
# headers for stubs; nothing image/X11 is linked.
# ==========================================================================
if(EMSCRIPTEN)
  set(XWIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/xwin.c)
  set(WASM_STUBS ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/stubs)
else()
  set(XWIN_SRC ${SRC}/lib/xwin.c)
endif()

add_library(cards STATIC
  ${SRC}/lib/table.c
  ${SRC}/lib/help.c
  ${SRC}/lib/stack.c
  ${SRC}/lib/imagelib.c
  ${XWIN_SRC}
  ${SRC}/lib/table_rn.c
  ${SRC}/lib/funcs.c
  ${GEN}/lib/images.c
)
target_include_directories(cards PUBLIC ${SRC}/lib ${SRC}/games)
if(EMSCRIPTEN)
  target_include_directories(cards PUBLIC ${WASM_STUBS})
else()
  target_include_directories(cards PUBLIC ${X11_INCLUDE_DIR})
  target_link_libraries(cards PUBLIC PNG::PNG ZLIB::ZLIB ${X11_LIBRARIES} m)
endif()

# ==========================================================================
# Stage 4: the games.
#
# Each game was written as its own program: every game's generated -img.c
# defines `appimglib_imagelib` and games share other globals.  We compile each
# game to an OBJECT library with -Dappimglib_imagelib=<game>_appimglib so each
# references its own image table.  Remaining cross-game collisions are handled
# differently per target:
#   * native : partial-link (cc -r) each game and localize every symbol except
#              <game>_main (objcopy --keep-global-symbol) so per-game globals
#              stay isolated.
#   * WASM    : objcopy cannot localize wasm objects; link the objects directly
#              and let --allow-multiple-definition (on the `ace` target) absorb
#              the duplicates (equivalent to the -fcommon merge the code wants).
#
# add_game(<name> [NOHELP] PNGS <png>... [EXTRA <src>...])
# ==========================================================================
set(GAME_OBJECTS "")

function(add_game name)
  cmake_parse_arguments(G "NOHELP" "" "PNGS;EXTRA" ${ARGN})

  add_imglib(imgsrc games ${name} ${G_PNGS})
  set(sources ${SRC}/games/${name}.c ${imgsrc})
  foreach(e ${G_EXTRA})
    list(APPEND sources ${SRC}/games/${e})
  endforeach()
  if(NOT G_NOHELP)
    add_help(helpsrc ${name})
    list(APPEND sources ${helpsrc})
  endif()

  add_library(${name}_obj OBJECT ${sources})
  target_include_directories(${name}_obj PRIVATE ${SRC}/lib ${SRC}/games)
  target_compile_definitions(${name}_obj PRIVATE appimglib_imagelib=${name}_appimglib)
  if(EMSCRIPTEN)
    target_include_directories(${name}_obj PRIVATE ${WASM_STUBS})
    list(APPEND GAME_OBJECTS $<TARGET_OBJECTS:${name}_obj>)
    set(GAME_OBJECTS "${GAME_OBJECTS}" PARENT_SCOPE)
  else()
    set(part ${CMAKE_BINARY_DIR}/gameobj/${name}_part.o)
    set(obj  ${CMAKE_BINARY_DIR}/gameobj/${name}.o)
    add_custom_command(
      OUTPUT ${obj}
      COMMAND ${CMAKE_C_COMPILER} -r -nostdlib $<TARGET_OBJECTS:${name}_obj> -o ${part}
      COMMAND ${CMAKE_OBJCOPY} --keep-global-symbol=${name}_main ${part} ${obj}
      DEPENDS $<TARGET_OBJECTS:${name}_obj>
      COMMENT "Localizing ${name} (keep only ${name}_main global)"
      COMMAND_EXPAND_LISTS
      VERBATIM
    )
    set_source_files_properties(${obj} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
    set(GAME_OBJECTS "${GAME_OBJECTS};${obj}" PARENT_SCOPE)
  endif()
endfunction()

add_game(canfield   NOHELP PNGS canfield.png)
add_game(freecell   PNGS freecell.png)
add_game(golf       PNGS golf.png golf-arrow.png golf-noarrow.png)
add_game(mastermind PNGS mastermind-b.png mastermind-c.png mastermind-e.png
                         mastermind-eb.png mastermind-g.png mastermind-k.png
                         mastermind-o.png mastermind-p.png mastermind-r.png
                         mastermind-w.png mastermind-y.png mastermind.png)
add_game(merlin     PNGS merlin.png merlin-c.png merlin-b.png)
add_game(minesweeper PNGS minesweeper.png minesweeper-x.png minesweeper-t.png
                          minesweeper-c12.png minesweeper-c24.png
                          minesweeper-c36.png minesweeper-c48.png)
add_game(pegged     PNGS pegged.png pegged-h.png pegged-p.png)
add_game(solitaire  PNGS solitaire.png)
add_game(spider     PNGS spider.png golf-arrow.png golf-noarrow.png)
add_game(thornq     PNGS thornq.png thornq-arrow.png thornq-noarrow.png)
add_game(taipedit   PNGS taipedit.png taipedit-bu.png taipedit-bs.png
                         taipedit-bt.png taipei-tiles.png
                    EXTRA taipeilib.c)
add_game(taipei     PNGS taipei.png taipei-tiles.png
                    EXTRA taipeilib.c)

# ==========================================================================
# Stage 5: the combined executable (launcher + all games).
# ==========================================================================
add_executable(ace ${CMAKE_CURRENT_SOURCE_DIR}/src/launcher.c ${GAME_OBJECTS})
target_include_directories(ace PRIVATE ${SRC}/lib ${SRC}/games)
target_link_libraries(ace PRIVATE cards)
if(EMSCRIPTEN)
  target_include_directories(ace PRIVATE ${WASM_STUBS})
  # Absorb the legacy duplicate (tentative) definitions that -fcommon would
  # merge natively but which wasm-ld rejects by default.
  target_link_options(ace PRIVATE -Wl,--allow-multiple-definition -sALLOW_MEMORY_GROWTH=1)
  set_target_properties(ace PROPERTIES SUFFIX ".html")
endif()
