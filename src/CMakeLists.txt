add_executable(decode decode.cpp)

add_executable(haversine haversine.c parser.c utils.c json.c)

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
