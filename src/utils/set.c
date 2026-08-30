#include "types.h"
#include "set.h"
#include "string8.h"

#include <stdio.h>
#include <limits.h>

////////////////////////////////////////////////////////////////////////////////
///
struct Set {
  usize count;                              ///< Number of entries in the set
  usize capacity;                           ///< Number of buckets/slots
  i32 (*cmp)(const void* x, const void* y); ///< Function for comparing two entries
  u64 (*hash)(const void* x);               ///< Function for hashing an entry
  struct Entry {                            ///< Entry
    struct Entry* next;                     ///< Link to next entry in the list
    const void* value;                      ///< Entry's value
  }** buckets;                              ///< Array of slots for Entries
};

////////////////////////////////////////////////////////////////////////////////
///
struct SetError {
  SetStatusCode code;
  StringView message;
};

////////////////////////////////////////////////////////////////////////////////
///
struct SetCreateResult {
  bool ok;
  union {
    Set* result;
    SetError error;
  };
};

////////////////////////////////////////////////////////////////////////////////
///
struct SetResult {
  bool ok;
  union {
    void* value;
    SetError error;
  };
};

////////////////////////////////////////////////////////////////////////////////
///
static i32 cmpatom(const void *x, const void *y)
{
  return x != y;
}

////////////////////////////////////////////////////////////////////////////////
///
static u64 hashatom(const void *x)
{
  return (unsigned long)x>>2;
}

////////////////////////////////////////////////////////////////////////////////
///
SetCreateResult* set_create(u32 hint, Allocator* allocator, i32 cmp(const void* x, const void* y), u64 hash(const void* x))
{
  SetCreateResult* res = allocator_new(allocator, SetCreateResult); // = { .ok = true };
  res->ok = true;
  Set* set = NULL;
  static u32 primes[] = { 509, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, INT_MAX, UINT_MAX };

  u32 i;
  for (i = 1; primes[i] < hint; i++) {};

  set = allocator_alloc(allocator, sizeof(*set) + primes[i-1] * sizeof(set->buckets[0]), alignof(Set));

  if (set == NULL) {
    *res = (SetCreateResult){ .ok = false, .error = (SetError){ .code = SET_STATUS_ALLOCATION_FAIL }};
  }

  if (res->ok == true) {
    set->capacity = primes[i-1];
    set->count = 0;
    set->buckets = (struct Entry**)(set + 1);
    set->cmp = cmp ? cmp : cmpatom;
    set->hash = hash ? hash : hashatom;
    for (i = 0; i < set->capacity; ++i) {
      set->buckets[i] = NULL;
    }
    res->result = set;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
///
bool set_ok(SetCreateResult* res)
{
  if (res->ok == true) { return true; }
  else { return false; }
}

////////////////////////////////////////////////////////////////////////////////
///
Set* set_getSet(SetCreateResult* res)
{
  return res->result;
}

////////////////////////////////////////////////////////////////////////////////
///
SetStatusCode set_tryInsert(Set* set, Allocator* allocator, void* entry)
{
  assert(set);
  assert(allocator);
  assert(entry);

  SetStatusCode status = SET_STATUS_OK;
  u64 hash = set->hash(entry);
  usize bucket = hash % set->capacity;
  struct Entry* link = NULL;

  for (link = set->buckets[bucket]; link; link = link->next) {
    // Check if the exact entry is already in the bucket linked list
    if (set->cmp(entry, link->value) == 0) {
      status = SET_STATUS_ENTRY_EXISTS;
      break;
    }
  }

  if (link == NULL) {
    // The entry was not in the list, allocate space for a new one
    link = allocator_new(allocator, struct Entry);
    if (link == NULL) {
      status = SET_STATUS_ALLOCATION_FAIL;
    } else {
      link->next = set->buckets[bucket];
      set->buckets[bucket] = link;
      link->value = entry;
    }
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
///
const void* set_tryIntern(Set* set, Allocator* allocator, void* entry)
{
  assert(set);
  assert(allocator);
  assert(entry);

  [[maybe_unused]] SetStatusCode status = SET_STATUS_OK;
  u64 hash = set->hash(entry);
  usize bucket = hash % set->capacity;
  struct Entry* link = NULL;

  for (link = set->buckets[bucket]; link; link = link->next) {
    // Check if the exact entry is already in the bucket linked list
    if (set->cmp(entry, link->value) == 0) {
      status = SET_STATUS_ENTRY_EXISTS;
      break;
    }
  }

  if (link == NULL) {
    // The entry was not in the list, allocate space for a new one
    link = allocator_new(allocator, struct Entry);
    if (link == NULL) {
      status = SET_STATUS_ALLOCATION_FAIL;
    } else {
      link->next = set->buckets[bucket];
      set->buckets[bucket] = link;
      link->value = entry;
    }
  }

  return link ? link->value : NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Check if entry is in the set
extern bool set_exists(Set* set, void* entry)
{
  assert(set);
  assert(entry);

  bool ret = false;

  u64 hash = set->hash(entry);
  usize bucket = hash % set->capacity;
  struct Entry* link = NULL;

  for (link = set->buckets[bucket]; link; link = link->next) {
    // Check if the exact entry is already in the bucket linked list
    if (set->cmp(entry, link->value) == 0) {
      ret = true;
      break;
    }
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
///
const void* set_get(Set* set, void* entry)
{
  assert(set);
  assert(entry);

  u64 hash = set->hash(entry);
  usize bucket = hash % set->capacity;
  struct Entry* link = NULL;
  const void* value = NULL;

  for (link = set->buckets[bucket]; link; link = link->next) {
    if (set->cmp(entry, link->value) == 0) {
      value = link->value;
    }
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
///
void set_map(Set* set, void apply(const void* member, void* cl), void* cl)
{
  struct Entry* p;

  assert(set);
  assert(apply);
  for (usize i = 0; i < set->capacity; i++) {
    for (p = set->buckets[i]; p; p = p->next) {
      apply(p->value, cl);
    }
  }
  
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy a set created with set_create
/// Note: currently not needed, because the allocator manages the memory
void set_destroy([[maybe_unused]] Set** set, Allocator* allocator)
{
  assert(set && *set);
  if ((*set)->capacity > 0) {
    struct Entry* p;
    struct Entry* q;
    for (usize i = 0; i < (*set)->capacity; ++i) {
      for (p = (*set)->buckets[i]; p; p = q) {
        q = p->next;
        allocator_free(allocator, p, sizeof(*p), alignof(struct Entry));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
SetCreateResult set_init([[maybe_unused]] void* buffer, i32 cmp(const void* x, const void* y), u64 hash(const void* x))
{
  assert(buffer);
  SetCreateResult ret = { .ok = true };
  Set set;
  set.cmp = cmp;
  set.hash = hash;
  return ret;
}
