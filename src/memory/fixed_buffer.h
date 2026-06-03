#pragma once

#include "allocator.h"

Allocator* fixed_buffer_allocator_create(u8* buffer, usize buffer_size);

