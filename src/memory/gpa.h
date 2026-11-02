#pragma once

#include "allocator.h"

typedef struct GeneralPurposeAllocator GeneralPurposeAllocator;

/// GPA, basically a malloc, and free wrapper
struct GeneralPurposeAllocator {
  Allocator base;
};

Allocator* gpa_allocator_create(void);
