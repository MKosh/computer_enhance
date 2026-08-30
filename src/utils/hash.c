#include "types.h"
#include "hash.h"
#include "string8.h"

static const u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
static const u64 FNV_PRIME = 1099511628211ULL;

////////////////////////////////////////////////////////////////////////////////
///
u64 hash_fnv1aStringView(const void* x)
{
    StringView* s = (StringView*)x;
    u64 hash = FNV_OFFSET_BASIS;
    for (usize i = 0; i < s->len; i++) {
        hash ^= (u8)s->str[i];
        hash *= FNV_PRIME;
    }
    return hash == 0 ? 1 : hash;
}

////////////////////////////////////////////////////////////////////////////////
///
u64 hash_fnv1aCString(const void* x)
{
    char* s = (char*)x;
    u32 len = strlen(s);
    u64 hash = FNV_OFFSET_BASIS;
    for (usize i = 0; i < len; i++) {
        hash ^= (u8)s[i];
        hash *= FNV_PRIME;
    }
    return hash == 0 ? 1 : hash;
}

