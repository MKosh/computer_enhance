add_executable(decode decode.cpp)

add_executable(parse parser.c utils.c json.c)

target_compile_definitions(parse PUBLIC
  "$<$<CONFIG:DEBUG>:DEBUG>"
)

target_compile_options(parse PRIVATE
  -fsanitize=address,leak
  -static-libsan
  -fno-omit-frame-pointer
  -Wall
  -Wextra
  -Werror
)

target_link_options(parse PRIVATE
  -fsanitize=address,leak
  -static-libsan
  -fno-omit-frame-pointer
  -lm
)
