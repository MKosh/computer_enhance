#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "parser2.h"

i32 DEBUG_ = 0;

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
  printf("Hello world.\n");
  if (argc >= 2) {
    pretend_main(argv[1]);
  } else {
    pretend_main("data/data_10k.json");
  }
  return EXIT_SUCCESS;
}
