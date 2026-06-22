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
# This file reproduces that two-stage build with CMake.  The actual source
# code is not vendored here; it is fetched from the `upstream` branch of the
# upstream git repository at configure time (see FetchContent below).

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
# The upstream branch has no CMakeLists.txt, so we only populate it (we must
# not let FetchContent try to add_subdirectory() it as a sub-project).
FetchContent_GetProperties(acesrc)
if(NOT acesrc_POPULATED)
  FetchContent_Populate(acesrc)
endif()

set(SRC ${acesrc_SOURCE_DIR})
set(GEN ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN}/lib ${GEN}/games ${CMAKE_BINARY_DIR}/gameobj)

# --------------------------------------------------------------------------
# Dependencies (these replace AC_PATH_XTRA / --with-png / --with-z).
# --------------------------------------------------------------------------
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(X11 REQUIRED)

# --------------------------------------------------------------------------
# This is ~1998-era C.  Modern compilers reject some of its idioms by
# default; keep them as warnings so the unmodified upstream source builds.
# -fcommon restores the pre-GCC-10 tentative-definition behaviour the code
# relies on.
# --------------------------------------------------------------------------
add_compile_options(
  -fcommon
  -Wno-implicit-function-declaration
  -Wno-implicit-int
  -Wno-int-conversion
  -Wno-incompatible-pointer-types
)

# ==========================================================================
# Stage 1: the code-generation tools (built with the build compiler).
# In a native build these are ordinary executables that we run during the
# build to produce generated sources.
# ==========================================================================
add_executable(make-imglib ${SRC}/lib/make-imglib.c)
target_link_libraries(make-imglib PRIVATE PNG::PNG ZLIB::ZLIB)

add_executable(text2c ${SRC}/lib/text2c.c)

# ==========================================================================
# Stage 2: code generation.
# ==========================================================================

# lib/images.c -- the shared "cards" image library, driven by lib/images.in
# which lists the logical image names; make-imglib scans lib/png for the
# matching PNG files.
add_custom_command(
  OUTPUT ${GEN}/lib/images.c
  COMMAND make-imglib -n cards -i ${SRC}/lib/png
          -d ${GEN}/lib/images.d
          ${SRC}/lib/images.in ${GEN}/lib/images.c
  DEPENDS make-imglib ${SRC}/lib/images.in
  VERBATIM
)

# Helper: generate <game>-img.c from a list of PNGs (make-imglib `-' mode,
# where image names come from the command line rather than an .in file).
function(add_imglib outvar dir name)
  set(pngpaths "")
  foreach(p ${ARGN})
    list(APPEND pngpaths ${SRC}/${dir}/${p})
  endforeach()
  add_custom_command(
    OUTPUT ${GEN}/${dir}/${name}-img.c
    COMMAND make-imglib -i ${SRC}/${dir} -n appimglib -
            ${GEN}/${dir}/${name}-img.c ${ARGN}
    DEPENDS make-imglib ${pngpaths}
    VERBATIM
  )
  set(${outvar} ${GEN}/${dir}/${name}-img.c PARENT_SCOPE)
endfunction()

# Helper: generate <game>-help.c from <game>.html via text2c.  text2c reads
# stdin and writes stdout, so the command runs through a shell for the
# redirections (matching the `%-help.c : %.html' pattern rule).
function(add_help outvar name)
  add_custom_command(
    OUTPUT ${GEN}/games/${name}-help.c
    COMMAND sh -c "$<TARGET_FILE:text2c> ${name}_help < ${SRC}/games/${name}.html > ${GEN}/games/${name}-help.c"
    DEPENDS text2c ${SRC}/games/${name}.html
    VERBATIM
  )
  set(${outvar} ${GEN}/games/${name}-help.c PARENT_SCOPE)
endfunction()

# ==========================================================================
# Stage 3: libcards (the support library the launcher and every game use).
# Static, so the result is a single self-contained executable and the games'
# references to launcher-provided symbols (ace_return_to_launcher, etc.)
# resolve at the final link.
# ==========================================================================
add_library(cards STATIC
  ${SRC}/lib/table.c
  ${SRC}/lib/help.c
  ${SRC}/lib/stack.c
  ${SRC}/lib/imagelib.c
  ${SRC}/lib/xwin.c
  ${SRC}/lib/table_rn.c
  ${SRC}/lib/funcs.c
  ${GEN}/lib/images.c
)
target_include_directories(cards PUBLIC ${SRC}/lib ${X11_INCLUDE_DIR})
target_link_libraries(cards PUBLIC PNG::PNG ZLIB::ZLIB ${X11_LIBRARIES} m)

# ==========================================================================
# Stage 4: the games, each compiled into one relocatable object exposing only
# <game>_main().
#
# All 12 games are linked into a single executable, but they were written as
# independent programs: every game's generated -img.c defines the same global
# `appimglib_imagelib`, and games share other global names.  To avoid clashes,
# each game's objects are partial-linked (cc -r) into one object and then all
# symbols except <game>_main are made local (objcopy --keep-global-symbol).
#
# add_game(<name> [NOHELP] PNGS <png>... [EXTRA <src>...])
#   PNGS   -> images compiled into <name>-img.c
#   NOHELP -> game has no .html help page (only canfield)
#   EXTRA  -> additional source files (taipeilib.c for the taipei games)
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
