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
  utils/set.c
  utils/compare.c
  utils/hash.c
  # utils/svset.c
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

####  Debug info flags ####
# Debug and RelWithDebInfo get debug symbols; Release does not.
target_compile_options(haversine2 PRIVATE
  $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-g -Wall -Wextra -Werror>
)

#### Optimization flags ###
# Release: full optimization, no debug info.
# RelWithDebInfo: still optimized (lightly, per CMake convention) but with debug info.
# Debug: no optimization.
target_compile_options(haversine2 PRIVATE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:Release>:-O3>
)

#### Link math library ###
target_link_options(haversine2 PRIVATE
    $<$<CONFIG:Debug>:-lm>
    $<$<CONFIG:RelWithDebInfo>:-lm>
    $<$<CONFIG:Release>:-lm>
)

#### AddressSanitizer: Debug only ###
target_compile_options(haversine2 PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address -static-libasan -fno-omit-frame-pointer>
)

target_link_options(haversine2 PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address -static-libasan>
)

#### NDEBUG handling ###
# Release and RelWithDebInfo typically define NDEBUG to strip asserts;
# Debug should NOT define it.
target_compile_definitions(haversine2 PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:RelWithDebInfo>:NDEBUG>
)

################################################################################
### Test exe
################################################################################
add_executable(test
  test.c
  utils/string8.c
  utils/set.c
  utils/compare.c
  utils/hash.c
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
