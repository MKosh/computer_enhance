#pragma once

#include "types.h"
#include "string8.h"
#include "allocator.h"

typedef struct Set Set;
typedef struct SetError SetError;
typedef struct SetCreateResult SetCreateResult;
typedef struct SetResult SetResult;

typedef enum SetStatusCode : i32 {
  SET_STATUS_ALLOCATION_FAIL = -1,
  SET_STATUS_OK = 0,
  SET_STATUS_ENTRY_EXISTS,
  SET_STATUS_COUNT
} SetStatusCode;

/// Create a set allocating space for the buckets
extern SetCreateResult* set_create(u32 hint, Allocator* allocator, i32 cmp(const void* x, const void* y), u64 hash(const void* x));

/// Create a set using an existing buffer for the buckets
// extern Set* set_init(void* buffer);
extern SetCreateResult set_init([[maybe_unused]] void* buffer, i32 cmp(const void* x, const void* y), u64 hash(const void* x));

/// Destroy a set created with set_create
extern void set_destroy(Set** set, Allocator* allocator);

/// Call apply on each member of the set
extern void set_map(Set* set, void apply(const void* member, void* cl), void* cl);

/// Check if entry is in the set
extern bool set_exists(Set* set, void* entry);

/// Retrieve the existing entry
extern const void* set_get(Set* set, void* entry);

/// Insert an entry into the set
extern SetStatusCode set_tryInsert(Set* set, Allocator* allocator, void* entry);
extern const void* set_tryIntern(Set* set, Allocator* allocator, void* entry);

extern bool set_ok(SetCreateResult* res);
Set* set_getSet(SetCreateResult* res);

/// FNV-1a hash function for StringView
// u64 set_FnvHashStringView(StringView s);
u64 set_FnvHashStringView(const void* x);
i32 set_compareStringView(const void* x, const void* y);
i32 set_compareCString(const void* x, const void* y);
u64 set_FnvHashCString(const void* x);
