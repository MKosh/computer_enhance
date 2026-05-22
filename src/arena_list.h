#pragma once

#include "mem.h"
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "utils.h"

typedef struct ArenaList ArenaList;

// typedef ptrdiff_t dptr;
// typedef uintptr_t uptr;
// typedef intptr_t  iptr;
// typedef size_t   usize;    

////////////////////////////////////////////////////////////////////////////////
///
// static inline uptr align_up(uptr ptr, size_t align) {
//   assert((align & (align - 1)) == 0 && "align must be a power of two");
//   return (ptr + align - 1) & ~(align - 1);
// }

////////////////////////////////////////////////////////////////////////////////
///
#define arenaList_alloc_type(a, T) \
   ((T*)arenaList_alloc_aligned(a, sizeof(T), alignof(T)))

////////////////////////////////////////////////////////////////////////////////
/// Alternative to max_align_t
// typedef union {
//   int i;
//   long l;
//   long *lp;
//   void* p;
//   void (*fp)(void);
//   float f;
//   double d;
//   long double ld;
// } max_alignment;

////////////////////////////////////////////////////////////////////////////////
///

ArenaList* arenaList_create (usize capacity);
void       arenaList_init   (ArenaList* arena, void* buffer, usize capacity);
void       arenaList_destroy(ArenaList** arena);
void       arenaList_reset  (ArenaList* arena);
void*      arenaList_alloc  (ArenaList* arena, usize size);
void*      arenaList_alloc_aligned(ArenaList* arena, usize size, usize align);
void       arenaList_pop    (ArenaList* arena, usize size);


typedef struct Arena {
  u8* pos;
  u8* end;
  u8* dat;
} Arena;

void* arena_alloc(Arena* arena, usize size);
#define allocate(x, size) _Generic((x),    \
                ArenaList *: arenaList_alloc, \
                Arena *:     arena_alloc     \
                )(x, size)
