#pragma once

#include <stddef.h>
#include <stdalign.h>
#include "types.h"

typedef struct Allocator Allocator;

typedef struct AllocatorVTable {
  void* (*alloc)(Allocator* a, usize size, usize align);
  void  (*free)(Allocator* a, void* ptr, usize size, usize align);
  void* (*realloc)(Allocator* a, void* ptr, usize old_size, usize new_size, usize align);
  void  (*reset)(Allocator* a);
  void  (*destroy)(Allocator* a);
} AllocatorVTable;

struct Allocator {
  const AllocatorVTable* vtable;
};

////////////////////////////////////////////////////////////////////////////////
///
static inline void* allocator_alloc(Allocator* a, usize size, usize align)
{
  return a->vtable->alloc(a, size, align);
}

////////////////////////////////////////////////////////////////////////////////
///
static inline void allocator_free(Allocator* a, void* ptr, usize size, usize align)
{
  return a->vtable->free(a, ptr, size, align);
}

////////////////////////////////////////////////////////////////////////////////
///
static inline void* allocator_realloc(Allocator* a, void* ptr, usize old_size, usize new_size, usize align)
{
  return a->vtable->realloc(a, ptr, old_size, new_size, align);
}

////////////////////////////////////////////////////////////////////////////////
///
static inline void allocator_reset(Allocator* a)
{
  return a->vtable->reset(a);
}

////////////////////////////////////////////////////////////////////////////////
///
static inline void allocator_destroy(Allocator* a)
{
  return a->vtable->destroy(a);
}

#define allocator_new(a, T) \
  ((T*)allocator_alloc((a), sizeof(T), alignof(T)))

#define allocator_new_array(a, T, n) \
  ((T*)allocator_alloc((a), sizeof(T) * (n), alignof(T)))
