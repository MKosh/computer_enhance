################################################################################
### 8086 decoder
################################################################################
add_executable(decode decode.cpp)

################################################################################
### Haversine
################################################################################
add_executable(haversine
  haversine.c
  parser.c
  utils/string8.c
  utils/metrics.c
  memory/mem.c
)

target_include_directories(haversine PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/memory
)

target_link_directories(haversine PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/memory
)

target_compile_definitions(haversine PUBLIC
  "$<$<CONFIG:DEBUG>:DEBUG>"
)

target_compile_options(haversine PRIVATE
  -fsanitize=address,leak
  -static-libsan
  -fno-omit-frame-pointer
  -Wall
  -Wextra
  -Werror
)

target_link_options(haversine PRIVATE
  -fsanitize=address,leak
  -static-libsan
  -fno-omit-frame-pointer
  -lm
)
################################################################################
### Haversine 2
################################################################################
add_executable(haversine2
  haversine2.c
  json2.c
  parser2.c
  utils/string8.c
  utils/metrics.c
  memory/mem.c
  memory/arena_list.c
  memory/fixed_buffer.c
)

target_include_directories(haversine2 PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/memory
)

target_link_directories(haversine2 PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/memory
)

target_compile_definitions(haversine2 PUBLIC
  "$<$<CONFIG:DEBUG>:DEBUG>"
)
# add_compile_options(
#   "-Wall" "-Wpedantic" "-Wextra" "-fexceptions"
#   "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
# )
target_compile_options(haversine2 PRIVATE
    "$<$<CONFIG:DEBUG>:-fsanitize=address,leak;-static-libsan;-fno-omit-frame-pointer>"
  -Wall
  -Wextra
  -Werror
)

target_link_options(haversine2 PRIVATE
    "$<$<CONFIG:DEBUG>:-fsanitize=address,leak;-static-libsan;-fno-omit-frame-pointer>"
  # -fsanitize=address,leak
  # -static-libsan
  # -fno-omit-frame-pointer
  -lm
)

################################################################################
### Test exe
################################################################################
add_executable(test
  test.c
  memory/mem.c
  memory/arena_list.c
  memory/fixed_buffer.c
)

target_include_directories(test PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/memory
)

target_link_directories(test PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/memory
)

target_compile_options(test PRIVATE
  -fsanitize=address,leak
  -static-libsan
  -fno-omit-frame-pointer
  -Wall
  -Wextra
  -Werror
)

target_link_options(test PRIVATE
  -fsanitize=address,leak
  -static-libsan
  -fno-omit-frame-pointer
  -lm
)
