#include "allocator.h"
#include "gpa.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static void* gpa_alloc(Allocator *a, usize size, usize align);
static void  gpa_reset(Allocator *a);
static void  gpa_destroy(Allocator *a);
static void* gpa_realloc(Allocator *a, void *ptr, usize old_size, usize new_size, usize align);
static void  gpa_free(Allocator *a, void *ptr, usize size, usize align);

static const AllocatorVTable gpa_vtable = {
  .alloc   = gpa_alloc,
  .reset   = gpa_reset,
  .destroy = gpa_destroy,
  .realloc = gpa_realloc,
  .free    = gpa_free
};

Allocator *gpa_allocator_create(void)
{
  GeneralPurposeAllocator* a = malloc(sizeof(*a));
  a->base.vtable = &gpa_vtable;
  return &a->base; // Upcast
}

static void* gpa_alloc(Allocator *a, usize size, [[maybe_unused]] usize align)
{
  assert(a && "null allocator");
  assert(size > 0 && "Invalid size");
  assert(align > 0 && "Invalid alignment");

  [[maybe_unused]] GeneralPurposeAllocator *allocator = (GeneralPurposeAllocator*)a;

  return malloc(size);
}

static void  gpa_reset(Allocator *a)
{
  [[maybe_unused]] GeneralPurposeAllocator *allocator = (GeneralPurposeAllocator*)a;
  fprintf(stderr, "Error: Function 'reset' is not supported for GPA allocators.\n");
}

static void  gpa_destroy(Allocator *a)
{
  assert(a && "null allocator");
  [[maybe_unused]] GeneralPurposeAllocator *allocator = (GeneralPurposeAllocator*)a;
  free(allocator);
}

static void* gpa_realloc(Allocator *a, [[maybe_unused]] void *ptr, [[maybe_unused]] usize old_size, [[maybe_unused]] usize new_size, [[maybe_unused]] usize align)
{
  [[maybe_unused]] GeneralPurposeAllocator *allocator = (GeneralPurposeAllocator*)a;
  assert(false && "Realloc not yet implemented for GPA.");
  return nullptr;
}
static void gpa_free(Allocator *a, void *ptr, [[maybe_unused]] usize size, [[maybe_unused]] usize align)
{
  [[maybe_unused]] GeneralPurposeAllocator *allocator = (GeneralPurposeAllocator*)a;
  // assert(false && "Free not yet implemented for GPA");
  free(ptr);
}
