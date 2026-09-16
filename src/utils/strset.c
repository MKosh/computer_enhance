#include "strset.h"
#include <limits.h>
#include <stdalign.h>

StringSet* StringSet_create(u32 hint, Allocator* allocator, i32* status)
{
  assert(allocator);

  StringSet* set;
  *status = SET_STATUS_OK;

  static u32 primes[] = { 509, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, INT_MAX, UINT_MAX };

  u32 i;
  for (i = 1; primes[i] < hint; i++) {};

  set = allocator_alloc(allocator, sizeof(*set) + primes[i-1] * sizeof(set->buckets[0]), alignof(StringSet));

  if (set != nullptr) {
    set->capacity = primes[i-1];
    set->count = 0;
    set->buckets = (struct Entry**)(set + 1);
    set->allocator = allocator;
    for (i = 0; i < set->capacity; ++i) {
      set->buckets[i] = NULL;
    }
  } else {
    *status = SET_STATUS_ALLOCATION_FAIL;
  }

  return set;
}

static constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
static constexpr u64 FNV_PRIME = 1099511628211ULL;

////////////////////////////////////////////////////////////////////////////////
///
static u64 hashStringView(StringView sv)
{
  u64 hash = FNV_OFFSET_BASIS;
  for (usize i = 0; i < sv.len; i++) {
    hash ^= (u8)sv.str[i];
    hash *= FNV_PRIME;
  }
  return hash == 0 ? 1 : hash;
}

////////////////////////////////////////////////////////////////////////////////
///
StringView StringSet_tryInsert(StringSet* set, StringView entry)
{
  assert(set);
  [[maybe_unused]] StringSetStatus status = SET_STATUS_OK;
  // StringView sv = NULL_SV;
  u64 hash = hashStringView(entry);
  usize bucket = hash % set->capacity;
  struct Entry* link = NULL;
  // printf("Sizeof entry: %lu\n", sizeof(struct Entry));

  for (link = set->buckets[bucket]; link; link = link->next) {
    if (sv_equal(entry, link->value)) {
      status = SET_STATUS_ENTRY_EXISTS;
      break;
    }
  }
  
  if (link == NULL) {
    // Allocate a new link in the chain
    link = allocator_new(set->allocator, struct Entry);
    if (link == NULL) { status = SET_STATUS_ALLOCATION_FAIL; return NULL_SV;  } 
    link->next = set->buckets[bucket];
    set->buckets[bucket] = link;
    // Allocate space for the backing string and set its value as the input
    String s = String_alloc(set->allocator, entry.str, entry.len);
    link->value = sv_create(s);
    // printf("Added: %.*s\n", (int)link->value.len, link->value.str);
  }

  return link->value;
}

////////////////////////////////////////////////////////////////////////////////
/// Don't worry about for now, because the allocator should free it's backing memory?
// void StringSet_destroy(StringSet** set)
// {
//
// }
