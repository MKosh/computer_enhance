#pragma once

#include "types.h"

u64 hash_fnv1aCString(const void* x);
u64 hash_fnv1aStringView(const void* x);
