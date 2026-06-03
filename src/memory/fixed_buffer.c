#include "fixed_buffer.h"

#include "mem.h"
#include "types.h"

#include <stdlib.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

typedef struct FixedBufferAllocator {
  Allocator base;
  usize buffer_size;
  u8* pos;
  u8* buffer;
} FixedBufferAllocator;

static void* fixed_buffer_alloc(Allocator* a, usize size, usize align);
static void  fixed_buffer_reset(Allocator* a);
static void  fixed_buffer_destroy(Allocator* a);
static void* fixed_buffer_realloc(Allocator* a, void* ptr, usize old_size, usize new_size, usize align);
static void  fixed_buffer_free(Allocator* a, void* ptr, usize size, usize align);

static const AllocatorVTable fixed_buffer_vtable = {
  .alloc   = fixed_buffer_alloc,
  .reset   = fixed_buffer_reset,
  .destroy = fixed_buffer_destroy,
  .realloc = fixed_buffer_realloc,
  .free    = fixed_buffer_free
};

Allocator* fixed_buffer_allocator_create(u8* buffer, usize buffer_size)
{
  FixedBufferAllocator* a = malloc(sizeof(FixedBufferAllocator));
  a->base.vtable = &fixed_buffer_vtable;
  a->buffer_size = buffer_size;
  a->buffer = buffer;
  a->pos = buffer;
  return &a->base; // Upcast safe due to layout guarantee
}

static void* fixed_buffer_alloc(Allocator* a, usize size, usize align) {
  FixedBufferAllocator* buffer = (FixedBufferAllocator*)a;

  uptr pos = (uptr)buffer->pos;
  uptr aligned = align_up(pos, align);
  uptr next = aligned + size;

  if (next > (uptr)(buffer->buffer + buffer->buffer_size)) {
    fprintf(stderr, "FixedBuffer allocation failed.\n");
    return NULL;
  }

  buffer->pos = (u8*)next;
  return (void*)aligned;
}

static void fixed_buffer_reset(Allocator* a)
{
  FixedBufferAllocator* buffer = (FixedBufferAllocator*)a;

  buffer->pos = buffer->buffer;
}

static void fixed_buffer_destroy([[maybe_unused]] Allocator* a)
{
  fprintf(stderr, "Fixed buffer doesn't own it's memory.\n");
}

static void* fixed_buffer_realloc(Allocator* a, [[maybe_unused]] void* ptr, [[maybe_unused]] usize old_size, [[maybe_unused]] usize new_size, [[maybe_unused]] usize align)
{
  [[maybe_unused]] FixedBufferAllocator* buffer = (FixedBufferAllocator*)a;
  return NULL;
}

static void fixed_buffer_free(Allocator* a, [[maybe_unused]] void* ptr, [[maybe_unused]] usize size, [[maybe_unused]] usize align)
{
  [[maybe_unused]] FixedBufferAllocator* buffer = (FixedBufferAllocator*)a;
}
