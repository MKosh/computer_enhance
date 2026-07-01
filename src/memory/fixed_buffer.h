#pragma once

#include "allocator.h"
typedef struct FixedBufferAllocator FixedBufferAllocator;
struct FixedBufferAllocator {
  Allocator base;
  usize buffer_size;
  bool owns_buffer;
  bool owns_self;
  u8* pos;
  u8* buffer;
};
void fixed_buffer_allocator_init(FixedBufferAllocator* a, u8* buffer, usize buffer_size);
Allocator* fixed_buffer_allocator_create(u8* buffer, usize buffer_size);

