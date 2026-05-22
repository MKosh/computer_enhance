#pragma once

#include "mem.h"
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "utils.h"

typedef struct Arena Arena;

// typedef ptrdiff_t dptr;
// typedef uintptr_t uptr;
// typedef intptr_t  iptr;
// typedef size_t   usize;    

// static inline uptr align_up(uptr ptr, size_t align) {
//   assert((align & (align - 1)) == 0 && "align must be a power of two");
//   return (ptr + align - 1) & ~(align - 1);
// }

#define arena_alloc_type(a, T) \
   ((T*)arena_alloc_aligned(a, sizeof(T), alignof(T)))

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

Arena* arena_create(u64 capacity);
void   arena_init(Arena* arena, void* buffer, usize capacity);
void   arena_destroy(Arena** arena);
void   arena_reset(Arena* arena);
void*  arena_alloc(Arena* arena, u64 size);
void*  arena_alloc_aligned(Arena* arena, u64 size, usize align);
void   arena_pop(Arena* arena, usize size);


