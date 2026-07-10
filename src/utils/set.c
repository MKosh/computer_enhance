#include "set.h"
#include "types.h"
#include "string8.h"

// [[maybe_unused]] static u32 hashStringView(StringView sv);
// [[maybe_unused]] static u32 hashString(String sv);
//
// [[maybe_unused]] static u32 hashStringView(StringView sv) {
//   u32 hash = 2166136261u;
//   for (usize i = 0; i < sv.len; i++) {
//     hash ^= (u8)sv.str[i];
//     hash *= 16777619;
//   }
//   return hash;
// }
//
// [[maybe_unused]] static u32 hashString(String sv) {
//   u32 hash = 2166136261u;
//   for (usize i = 0; i < sv.len; i++) {
//     hash ^= (u8)sv.str[i];
//     hash *= 16777619;
//   }
//   return hash;
// }

// #define FNV_OFFSET_BASIS 14695981039346656037ULL
// #define FNV_PRIME        1099511628211ULL
static const u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
static const u64 FNV_PRIME = 1099511628211ULL;
const SvSetSlot NULL_SVSS = { .hash = 0, .key = { .len = 0, .str = NULL }};

u64 hashStringView(StringView s)
{
    u64 hash = FNV_OFFSET_BASIS;
    for (usize i = 0; i < s.len; i++) {
        hash ^= (u8)s.str[i];
        hash *= FNV_PRIME;
    }
    return hash == 0 ? 1 : hash;
}

////////////////////////////////////////////////////////////////////////////////
/// Create
StringViewSet* svset_create(usize capacity, Allocator* allocator)
{
  StringViewSet* set = allocator_new(allocator, StringViewSet);
  if (set == NULL) {
    return NULL;
  }

  set->capacity = capacity;
  set->count = 0;
  set->keys = allocator_new_array(allocator, SvSetSlot, capacity);

  if (set->keys == NULL) {
    return NULL;
  }

  for (usize i = 0; i < capacity; i++) {
    set->keys[i] = (SvSetSlot){ 0 };
  }

  return set;
}

////////////////////////////////////////////////////////////////////////////////
/// Init
void svset_init(StringViewSet* svs, Allocator* allocator, usize capacity)
{
  assert(svs && "Null StringViewSet");
  svs->capacity = capacity;
  svs->count = 0;
  svs->keys = allocator_new_array(allocator, SvSetSlot, capacity);
  for (usize i = 0; i < capacity; i++) {
    svs->keys[i] = (SvSetSlot){ 0 };
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy
void svset_destroy([[maybe_unused]] StringViewSet* svs)
{
  // No need to free anything, the allocator will do it.
}

////////////////////////////////////////////////////////////////////////////////
/// Search
SvSetSlot* svset_search(StringViewSet* svs, StringView key)
{
  SvSetSlot* slot;
  u64 key_hash = hashStringView(key);
  u64 index = key_hash % svs->capacity;

  for (;;) {
    slot = &svs->keys[index];
    if (slot->hash != 0 && slot->hash != key_hash) {
      index = (index + 1) % svs->capacity;
      continue;
    }

    if (slot->key.str == NULL) {
      slot->hash = key_hash;
      return slot;
    }

    if (strncmp(key.str, slot->key.str, key.len) == 0) {
      return slot;
    }

    index = (index + 1) % svs->capacity;
  }

}

////////////////////////////////////////////////////////////////////////////////
StringView svset_get(StringViewSet* svs, StringView key)
{
  SvSetSlot* slot = svset_search(svs, key);
  if (slot->key.str == NULL) {
    return NULL_SV;
  }
  
  return slot->key;
}

////////////////////////////////////////////////////////////////////////////////
const char* svset_getChar(StringViewSet* svs, StringView key)
{
  return svset_get(svs, key).str;
}

////////////////////////////////////////////////////////////////////////////////
const char* svset_getCharFromString(StringViewSet* svs, StringView key)
{
  return svset_get(svs, key).str;
}

////////////////////////////////////////////////////////////////////////////////
bool svset_keyExists(StringViewSet* svs, StringView key)
{
  SvSetSlot* slot = svset_search(svs, key);
  bool key_exists = false;
  if (slot->key.str == NULL) {
    key_exists = false;
  } else if (strncmp(key.str, slot->key.str, key.len) == 0) {
    key_exists = true;
  }
  return key_exists;
}

////////////////////////////////////////////////////////////////////////////////
bool svset_keyExistsFromLiteral(StringViewSet* svs, const char* key)
{
  SvSetSlot* slot = svset_search(svs, sv_fromLiteral(key));
  bool key_exists = false;
  if (slot->key.str == NULL) {
    key_exists = false;
  } else if (strncmp(key, slot->key.str, slot->key.len) == 0) {
    key_exists = true;
  }
  return key_exists;
}

////////////////////////////////////////////////////////////////////////////////
/// Insert
bool svset_insert(StringViewSet* svs, StringView key)
{
  assert(svs && "Null svs");
  bool is_new_key = false;
  SvSetSlot* slot = svset_search(svs, key);
  if (slot->key.str == NULL) {
    is_new_key = true;
    slot->key = key;
  }

  return is_new_key;
}

////////////////////////////////////////////////////////////////////////////////
/// Delete
void svset_delete([[maybe_unused]] StringViewSet* svs, [[maybe_unused]] StringView key)
{
  // Don't worry about deleting for now.
}
