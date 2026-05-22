#include "arena_list.h"
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

constexpr usize MB = 1024 * 1024;

typedef struct ArenaChunk {
  struct ArenaChunk* prev;
  u8* pos;
  u8* end;
  u8 data[];
} ArenaChunk;

struct ArenaList {
  ArenaChunk* current;
};

static ArenaChunk* arena_create_chunk(usize size)
{
  ArenaChunk* chunk = (ArenaChunk*)malloc(sizeof(ArenaChunk) + size);
  assert(chunk && "Could not allocate memory for arena");
  if (chunk == NULL) {
    fprintf(stderr, "Error allocating ArenaChunk.\n");
    exit(1);
  }

  chunk->prev = NULL;
  chunk->pos = chunk->data;
  chunk->end = chunk->data + size;

  return chunk;
}

ArenaList* arenaList_create(usize capacity)
{
  if (capacity <= 0) {
    return NULL;
  }

  ArenaList* arena = (ArenaList*)malloc(sizeof(*arena));
  if (arena == NULL) {
    return NULL;
  }

  arena->current = arena_create_chunk(capacity);
  return arena;
}

// void   arena_init(Arena* arena, void* buffer, usize capacity)
// {
//   assert(arena && "Passed nullptr for arena");
//   assert(buffer && "Passed nullptr for buffer");
//   assert(capacity > 0 && "Passed invalid capacity");
//
//   arena->current = (ArenaChunk*)malloc(sizeof(ArenaChunk));
//   arena->current->data[0] = buffer;
//   arena->current->pos  = buffer;
//   arena->current->end  = buffer + capacity;
// }

void arenaList_destroy(ArenaList** arena)
{
  assert(arena && *arena && "Passed a nullptr to arena_reset");
  if (arena == NULL || *arena == NULL) {
    return;
  }

  while ((*arena)->current->prev) {
    ArenaChunk* temp = (*arena)->current;
    (*arena)->current = temp->prev;
    free(temp);
  }

  free((*arena)->current);
  (*arena)->current = NULL;
  free(*arena);
  *arena = NULL;
}

void arenaList_reset_current(ArenaList* arena)
{
  assert(arena && "Passed a nullptr to arena_reset");
  if (arena == NULL) {
    return;
  }
  arena->current->pos = arena->current->data;
}

void arenaList_reset_list(ArenaList* arena)
{
  assert(arena && "Passed a nullptr to arena_reset");
  if (arena == NULL) {
    return;
  }

  while (arena->current->prev != NULL) {
    ArenaChunk* temp = arena->current;
    arena->current = temp->prev;
    free(temp);
  }

  arena->current->pos = arena->current->data;
}

void* arenaList_alloc(ArenaList* arena, usize size)
{
  assert(arena && "Passed nullptr to arena_alloc");
  if (arena == NULL) {
    return NULL;
  }
  assert(size > 0 && "Requested size invalid");
  if (!(size > 0)) {
    return NULL;
  }

  ArenaChunk* current = arena->current;
  uptr pos = (uptr)current->pos;
  uptr aligned = align_up(pos, alignof(max_align_t));
  uptr next = aligned + size;

  if (next > (uptr)current->end) {
    // Allocate a new chunk
    usize chunk_size = size >= 10 * MB ? size : 10 * MB;
    ArenaChunk* new_chunk = arena_create_chunk(chunk_size);
    new_chunk->prev = arena->current;
    arena->current = new_chunk;
    pos = (uptr)current->pos;
    aligned = align_up(pos, alignof(max_align_t));
    next = aligned + size;
    new_chunk->pos = (u8*)next;
    return (void*)aligned;
  }

  current->pos = (u8*)next;
  return (void*)aligned;

}

void* arenaList_alloc_aligned(ArenaList* arena, usize size, usize align)
{
  assert(arena && "Passed nullptr to arena_alloc");
  if (arena == NULL) {
    return NULL;
  }
  assert(size > 0 && "Requested size invalid");
  if (!(size > 0)) {
    return NULL;
  }

  ArenaChunk* current = arena->current;
  uptr pos = (uptr)current->pos;
  uptr aligned = align_up(pos, align);
  uptr next = aligned + size;

  if (next > (uptr)current->end) {
    // Allocate a new chunk
    usize chunk_size = size >= 10 * MB ? size : 10 * MB;
    ArenaChunk* new_chunk = arena_create_chunk(chunk_size);
    new_chunk->prev = arena->current;
    arena->current = new_chunk;
    pos = (uptr)current->pos;
    aligned = align_up(pos, align);
    next = aligned + size;
    new_chunk->pos = (u8*)next;
    return (void*)aligned;
  }

  current->pos = (u8*)next;
  return (void*)aligned;

}
// void   arena_pop(Arena* arena, usize size);
