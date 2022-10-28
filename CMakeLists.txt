cmake_minimum_required(VERSION 3.19)
project(ace-online)

find_program(EMCC
  NAMES emcc
  REQUIRED
)
find_program(NPM
  NAMES npm
  REQUIRED
)
find_package(Patch
  REQUIRED
)

set(CODEGEN_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/codegen")
file(MAKE_DIRECTORY ${CODEGEN_DIRECTORY})

set(IMAGES_FILENAME "${CODEGEN_DIRECTORY}/images.c")
add_custom_command(
  OUTPUT ${IMAGES_FILENAME}
  COMMAND ${NPM} run imagegen -- "${CMAKE_CURRENT_SOURCE_DIR}/lib/images.in" ${IMAGES_FILENAME} "${CMAKE_CURRENT_BINARY_DIR}/ace-texture.png"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/wasm/imagegen.mjs" "${CMAKE_CURRENT_SOURCE_DIR}/wasm/imagegen.c.mustache"
)

set(ACE_ONLINE_EXPORTED_FUNCTIONS "setValue")
function(add_game filename codegen)
  if(${codegen} STREQUAL ON)
    set(SOURCE_FILENAME "${CODEGEN_DIRECTORY}/${filename}.c")
  else()
    set(SOURCE_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/games/${filename}.c")
  endif()

  set(CODEGEN_FILENAME "${CODEGEN_DIRECTORY}/${filename}_codegen.c")
  set(NEW_MAIN "${filename}_main")

  file(WRITE ${CODEGEN_FILENAME} "const char *${filename}_help = 0;")

  target_sources(ace-online PRIVATE ${SOURCE_FILENAME} ${CODEGEN_FILENAME})
  set_property(SOURCE ${SOURCE_FILENAME} PROPERTY COMPILE_OPTIONS
    -Dmain=${NEW_MAIN}
  )
  set(ACE_ONLINE_EXPORTED_FUNCTIONS "${ACE_ONLINE_EXPORTED_FUNCTIONS},_${NEW_MAIN}" PARENT_SCOPE)
endfunction()

function(patch_file source_path source_file)
  set(SOURCE_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/${source_path}/${source_file}")
  set(PATCH_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/wasm/${source_file}.patch")
  set(CODEGEN_FILENAME "${CODEGEN_DIRECTORY}/${source_file}")

  add_custom_command(
    OUTPUT ${CODEGEN_FILENAME}
    COMMAND ${Patch_EXECUTABLE} -o ${CODEGEN_FILENAME} ${SOURCE_FILENAME} ${PATCH_FILENAME}
    DEPENDS ${SOURCE_FILENAME} ${PATCH_FILENAME}
  )
  target_sources(ace-online PRIVATE ${CODEGEN_FILENAME})
endfunction()

set(CMAKE_C_COMPILER ${EMCC})
set(CMAKE_CXX_COMPILER ${EMCC})
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wno-implicit-function-declaration)

add_executable(ace-online
  ${IMAGES_FILENAME}

  "${CMAKE_CURRENT_SOURCE_DIR}/lib/imagelib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/stack.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/table.c"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/wasm/native_shim.c"
)

patch_file("games" "solitaire.c")
patch_file("lib" "funcs.c")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/wasm/games.json" GAMES_JSON)
string(JSON GAMES_LENGTH LENGTH ${GAMES_JSON})
math(EXPR GAMES_LENGTH "${GAMES_LENGTH}-1")
foreach(GAME_IDX RANGE ${GAMES_LENGTH})
  string(JSON GAME_JSON GET ${GAMES_JSON} ${GAME_IDX})
  string(JSON GAME_FILENAME GET ${GAME_JSON} "name")
  string(JSON GAME_CODEGEN ERROR_VARIABLE _ GET ${GAME_JSON} "codegen")
  add_game(${GAME_FILENAME} ${GAME_CODEGEN})
endforeach()

target_include_directories(ace-online PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)
set_property(TARGET ace-online PROPERTY OUTPUT_NAME "ace-online.js")
set_property(TARGET ace-online PROPERTY LINK_OPTIONS
  -sMODULARIZE -sEXPORT_NAME=createAce
  -sEXPORTED_FUNCTIONS=${ACE_ONLINE_EXPORTED_FUNCTIONS}
  -sEXPORTED_RUNTIME_METHODS=allocateUTF8
  -sNO_FILESYSTEM=1
  -sASYNCIFY -sASYNCIFY_IMPORTS=nextEvent,emscripten_asm_const_int
)

add_custom_target(pages
  COMMAND ${NPM} run build
)
add_dependencies(pages ace-online)
