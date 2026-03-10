#include "mem.h"

void* reallocate(void* pointer,[[maybe_unused]] size_t old_size, size_t new_size)
{
  if (new_size == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, new_size);
  if (result == NULL) exit(1);
  return result;
}
