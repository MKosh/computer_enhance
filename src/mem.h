#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "utils.h"

#define ALLOCATE(type, count) \
  (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, old_count, new_count) \
  (type*)reallocate(pointer, sizeof(type) * (old_count), sizeof(type)*(new_count))

#define FREE_ARRAY(type, pointer, old_count) \
  reallocate(pointer, sizeof(type) * old_count, 0)

void* reallocate(void* pointer, size_t old_size, size_t new_size);

// typedef struct Arena {
//   u8* pos;
//   u8* end;
//   u8* data;
// } Arena;
//
// typedef struct Arena Arena;
//
typedef ptrdiff_t dptr;
typedef uintptr_t uptr;
typedef intptr_t  iptr;
typedef size_t   usize;    

static inline uptr align_up(uptr ptr, size_t align) {
  assert((align & (align - 1)) == 0 && "align must be a power of two");
  return (ptr + align - 1) & ~(align - 1);
}

// #define arena_alloc_type(a, T) \
//    ((T*)arena_alloc_aligned(a, sizeof(T), alignof(T)))

////////////////////////////////////////////////////////////////////////////////
/// Alternative to max_align_t
typedef union {
  int i;
  long l;
  long *lp;
  void* p;
  void (*fp)(void);
  float f;
  double d;
  long double ld;
} max_alignment;
//
// Arena* arena_create(u64 capacity);
// void   arena_init(Arena* arena, void* buffer, usize capacity);
// void   arena_destroy(Arena** arena);
// void   arena_reset(Arena* arena);
// void*  arena_alloc(Arena* arena, u64 size);
// void*  arena_alloc_aligned(Arena* arena, u64 size, usize align);
// void   arena_pop(Arena* arena, usize size);

