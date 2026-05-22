#include "mem.h"
#include "arena.h"
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

struct Arena {
  u8* pos;
  u8* end;
  u8* data;
};

////////////////////////////////////////////////////////////////////////////////
///
void arena_init(Arena* arena, void* buffer, usize capacity)
{
  assert (arena && "Passed NULL arena to arena_init");
  assert (buffer && "Passed NULL buffer to arena_init");
  assert (capacity > 0 && "Passed invalid capacity to arena_init");

  arena->data = (u8*)buffer;
  arena->pos  = (u8*)buffer;
  arena->end  = (u8*)buffer + capacity;
}

////////////////////////////////////////////////////////////////////////////////
///
Arena* arena_create(u64 capacity)
{
  // puts("Stating arena_create");
  if (capacity <= 0) {
    return NULL;
  }

  Arena* arena = (Arena*)malloc(sizeof(Arena));
  if (arena == NULL) {
    return NULL;
  }

  u8* buf = (u8*)malloc(capacity);
  // arena->data = (u8*)malloc(capacity);
  if (buf == NULL) {
    free(arena);
    return NULL;
  }

  arena_init(arena, buf, capacity);
  // puts("Finshing arena_create");
  return arena;
}

////////////////////////////////////////////////////////////////////////////////
///
void arena_destroy(Arena** arena)
{
  // puts("Stating arena_destroy");
  assert(arena && "Passed nullptr to arena_destroy");
  if (arena == NULL) {
    return;
  }

  free((*arena)->data);
  (*arena)->data = NULL;
  (*arena)->end = NULL;
  (*arena)->pos = NULL;
  free(*arena);
  *arena = NULL;
  // puts("Finshing arena_destroy");
}

////////////////////////////////////////////////////////////////////////////////
/// Reset the position in the arena, don't actually deallocate anything 
void arena_reset(Arena* arena)
{
  assert(arena && "Passed nullptr to arena_reset");
  if (arena == NULL) {
    return;
  }
  arena->pos = arena->data;
}

////////////////////////////////////////////////////////////////////////////////
/// Bump the allocator pointer if there is enough space in the buffer
void* arena_alloc(Arena* arena, u64 size)
{
  // puts("Stating arena_alloc");
  assert(arena && "Passed nullptr to arena_alloc");
  if (arena == NULL) {
    return NULL;
  }
  assert(size > 0 && "Requested invalid amount of memory");
  if (!(size > 0)) {
    return NULL;
  }

  uptr current = (uptr)arena->pos; // Points to where the start of the current allocation
  uptr aligned = (uptr)align_up(current, alignof(max_align_t)); // points to the start of this aligned allocation
  uptr next    = aligned + size; // Points to where the next allocation would start.

  // Alternative alignment using the max_alignment union if using a standard
  // older than C11
  // uptr aligned = (uptr)align_up(current, sizeof(max_alignment)); // points to the start of this aligned allocation

  // Don't allocate anything extra for now. Future implementation could allocate
  // a new buffer and link to it.
  if (next > (uptr)arena->end) {
    return NULL;
  }

  // Set the position to where the next allocationc could.
  arena->pos = (u8*)next;
  // puts("Finshing arena_alloc");
  return (void*)aligned;
}

////////////////////////////////////////////////////////////////////////////////
/// Bump the allocator pointer if there is enough space in the buffer with specific alignment
void* arena_alloc_aligned(Arena* arena, u64 size, usize align)
{
  // puts("Stating arena_alloc_aligned");
  assert(arena && "Passed nullptr to arena_alloc");
  if (arena == NULL) {
    return NULL;
  }
  assert(size > 0 && "Requested invalid amount of memory");
  if (!(size > 0)) {
    return NULL;
  }

  uptr current = (uptr)arena->pos; // Points to where the start of the current allocation
  uptr aligned = (uptr)align_up(current, align); // points to the start of this aligned allocation
  uptr next    = aligned + size; // Points to where the next allocation would start.

  // Don't allocate anything extra for now. Future implementation could allocate
  // a new buffer and link to it.
  if (next > (uptr)arena->end) {
    return NULL;
  }

  // Set the position to where the next allocationc could.
  arena->pos = (u8*)next;
  // puts("Finshing arena_alloc_aligned");
  return (void*)aligned;
}

////////////////////////////////////////////////////////////////////////////////
/// Pop 'size' bytes off of the arena buffer
void arena_pop(Arena* arena, usize size)
{
  assert(arena && "Passed nullptr to arena_pop");
  if (arena == NULL) {
    return;
  }

  usize amount = MIN(size, (uptr)arena->pos - (uptr)arena->data);

  arena->pos -= amount;
}

