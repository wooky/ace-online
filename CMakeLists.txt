cmake_minimum_required(VERSION 3.18)
project(ace-online)

find_program(EMCC
  NAMES emcc
  REQUIRED
)
find_program(NPM
  NAMES npm
  REQUIRED
)

set(CMAKE_C_COMPILER ${EMCC})
set(CMAKE_CXX_COMPILER ${EMCC})
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wno-implicit-function-declaration)
add_link_options(
  -sMODULARIZE -sEXPORT_NAME=createAce
  -sEXPORTED_FUNCTIONS=_main,setValue
  -sEXPORTED_RUNTIME_METHODS=allocateUTF8
  -sNO_FILESYSTEM=1
  -sASYNCIFY -sASYNCIFY_IMPORTS=nextEvent,emscripten_asm_const_int
)

add_executable(ace-online
  "${CMAKE_CURRENT_SOURCE_DIR}/games/thornq.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/lib/funcs.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/imagelib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/stack.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/table.c"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/wasm/js_callback.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/wasm/native_shim.c"
)

target_include_directories(ace-online PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)
set_target_properties(ace-online PROPERTIES
  OUTPUT_NAME "ace-online.js"
)
