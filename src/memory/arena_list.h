#pragma once

#include "allocator.h"

Allocator* arena_list_allocator_create(usize block_size);
