#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "parser2.h"

i32 DEBUG_ = 0;

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
  printf("Hello world.\n");
  pretend_main();
  return EXIT_SUCCESS;
}
