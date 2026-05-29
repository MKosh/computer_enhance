#include "arena_list.h"

#include "mem.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct ArenaBlock {
  struct ArenaBlock* prev;
  u8* pos;
  u8* end;
  u8  buf[];
} ArenaBlock;

typedef struct ArenaListAllocator {
  Allocator base;
  ArenaBlock* current;
  usize block_size;
} ArenaListAllocator;

static void* arena_list_alloc(Allocator* a, usize size, usize align);
static void  arena_list_reset(Allocator* a);
static void  arena_list_destroy(Allocator* a);
static void* arena_list_realloc(Allocator* a, void* ptr, usize old_size, usize new_size, usize align);
static void  arena_list_free(Allocator* a, void* ptr, usize size, usize align);

// vtable (one per allocator type, not per instance)
static const AllocatorVTable arena_list_vtable = {
  .alloc = arena_list_alloc,
  .realloc = arena_list_realloc,
  .reset = arena_list_reset,
  .destroy = arena_list_destroy,
  .free = arena_list_free
};

Allocator* arena_list_allocator_create(usize block_size)
{
  ArenaListAllocator* a = malloc(sizeof(ArenaListAllocator));
  a->base.vtable = &arena_list_vtable;
  a->block_size = block_size;
  a->current = NULL;
  return &a->base; // Upcast safe due to layout guarantee
}

static void* arena_list_alloc(Allocator* a, usize size, usize align)
{
  assert(a && "nullptr");
  assert(size > 0 && "Invalid size");
  assert(align > 0 && "Invalid alignment");

  ArenaListAllocator* arena = (ArenaListAllocator*)a;

  // Current Arena is already allocated
  if (arena->current != NULL) {
    uptr pos     = (uptr)arena->current->pos;
    uptr aligned = align_up(pos, align);
    uptr next    = aligned + size;

    if (next <= (uptr)arena->current->end) {
      arena->current->pos = (u8*)next;
      return (void*)aligned;
    }
  }

  // Current block is full, or one wasn't allocated
  usize block_size = size > arena->block_size ? size : arena->block_size;

  ArenaBlock* block = malloc(sizeof(ArenaBlock) + block_size);
  if (!block) {
    return NULL;
  }

  block->prev = arena->current;
  block->pos = block->buf;
  block->end = block->buf + block_size;
  arena->current = block;

  uptr pos     = (uptr)block->pos;
  uptr aligned = align_up(pos, align);
  uptr next    = aligned + size;

  block->pos = (u8*)next;
  return (void*)aligned;
}

static void arena_list_reset(Allocator* a)
{
  assert(a && "Passed nullptr to arena_list_reset");
  if (!a) {
    return;
  }

  ArenaListAllocator* arena = (ArenaListAllocator*)a;

  while (arena->current->prev != NULL) {
    ArenaBlock* temp = arena->current;
    arena->current = temp->prev;
    free(temp);
  }

  arena->current->pos = arena->current->buf;
}

static void arena_list_destroy(Allocator* a)
{
  assert(a && "Passed nullptr to arena_list_destroy");
  if (!a) {
    return;
  }

  ArenaListAllocator* arena = (ArenaListAllocator*)a;

  while (arena->current->prev != NULL) {
    ArenaBlock* temp = arena->current;
    arena->current = temp->prev;
    free(temp);
  }

  free(arena->current);
  free(arena);
}

static void* arena_list_realloc([[maybe_unused]] Allocator* a, [[maybe_unused]] void* ptr, [[maybe_unused]] usize old_size, [[maybe_unused]] usize new_size, [[maybe_unused]] usize align)
{
  fprintf(stderr, "Error: Function 'realloc' is not supported for ArenaList allocators.\n");
  exit(1);
}

static void  arena_list_free([[maybe_unused]] Allocator* a,[[maybe_unused]] void* ptr,[[maybe_unused]] usize size,[[maybe_unused]] usize align)
{
  fprintf(stderr, "Error: Function 'free' is not supported for ArenaList allocators.\n");
  exit(1);
}
