#pragma once

#include "types.h"
#include "string8.h"
#include "allocator.h"

typedef enum StringSetStatus : i32 {
  SET_STATUS_ALLOCATION_FAIL = -90,
  SET_STATUS_CREATION_FAIL,
  SET_STATUS_LOOKUP_FAIL,
  SET_STATUS_OK = 0,
  SET_STATUS_ENTRY_EXISTS
} StringSetStatus;

typedef struct StringSet {
  usize count;                       ///< Number of entries in the set
  usize capacity;                    ///< Number of buckets/slots
  Allocator* allocator;              ///< Pointer to the parsers intern allocator
  struct Entry {                     ///< Entry
    struct Entry* next;              ///< Link to next entry in the list
    StringView value;                ///< Entry's value
  }** buckets;                       ///< Array of slots for Entries
} StringSet;

////////////////////////////////////////////////////////////////////////////////
///
extern StringSet* StringSet_create(u32 hint, Allocator* allocator, i32* status);

////////////////////////////////////////////////////////////////////////////////
///
extern StringView StringSet_tryInsert(StringSet* set, StringView entry);

////////////////////////////////////////////////////////////////////////////////
///
extern void StringSet_destroy(StringSet** set);
