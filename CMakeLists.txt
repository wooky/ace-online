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

set(ACE_ONLINE_EXPORTED_FUNCTIONS "setValue")
function(add_game filename)
  set(SOURCE_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/games/${filename}.c")
  set(CODEGEN_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/codegen/${filename}_codegen.c")
  set(NEW_MAIN "${filename}_main")

  file(WRITE ${CODEGEN_FILENAME} "const char *${filename}_help = 0;")

  target_sources(ace-online PRIVATE ${SOURCE_FILENAME} ${CODEGEN_FILENAME})
  set_property(SOURCE ${SOURCE_FILENAME} PROPERTY COMPILE_OPTIONS
    -Dmain=${NEW_MAIN}
  )
  set(ACE_ONLINE_EXPORTED_FUNCTIONS "${ACE_ONLINE_EXPORTED_FUNCTIONS},_${NEW_MAIN}" PARENT_SCOPE)
endfunction()

set(CMAKE_C_COMPILER ${EMCC})
set(CMAKE_CXX_COMPILER ${EMCC})
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wno-implicit-function-declaration)

add_executable(ace-online
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/funcs.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/imagelib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/stack.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/table.c"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/wasm/js_callback.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/wasm/native_shim.c"
)
add_game(thornq)

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
  COMMAND ${NPM} run build || true
  COMMAND ${NPM} run build
)
add_dependencies(pages ace-online)
