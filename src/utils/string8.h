#pragma once

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "types.h"
#include "mem.h"
#include "allocator.h"

typedef struct String String;
typedef struct StringView StringView;

////////////////////////////////////////////////////////////////////////////////
///@brief Make a Str from a string literal. The length is computed at compile time.
///
///The `"" lit` trick: adjacent string literals are concatenated, so a real
///literal passes through unchanged, but if @p lit is a `char *` variable the
///expression `"" ptr` is a compile error. Without it, `sizeof(ptr) - 1` would
///silently give the pointer size minus one as the "length".
///
///@param lit A string literal.
///
#define S(lit)        ((String){ sizeof("" lit) - 1, (char *)("" lit) })
#define SV(lit)   ((StringView){ sizeof("" lit) - 1, (char *)("" lit) })

////////////////////////////////////////////////////////////////////////////////
/// \brief Owning string
struct String {
  usize len; ///< Length of the string not including the null terminator.
  char *str; ///< Pointer to the start of the string.
};

////////////////////////////////////////////////////////////////////////////////
/// String Creation/Destruction
String string_from_cstr(const char *string, Allocator *allocator);
String string_from_stringview(const StringView sv, Allocator *allocator);
String string_create(const char *string, usize length, Allocator *allocator);
String string_clone(const String string, Allocator *alloactor);
String string_read_file(const char *filename, Allocator *allocator);
void   string_free(String *string, Allocator *allocator);
String string_cstr_concat(const char *str1, const char *str2, Allocator *allocator);

/// String Operations
void   string_append(String* string, StringView suffix, Allocator *allocator);
bool   string_equal(String a, String b);
i32    string_compare(String a, String b);
StringView string_sub(String string, i64 start, i64 len);
StringView string_prefix(String string, i64 len);
StringView string_suffix(String string, i64 len);
u64    string_hash(String string);

/// Printing
void   string_print(FILE *f, String string);
void   string_println(FILE *f, String string);

////////////////////////////////////////////////////////////////////////////////
/// \brief Non-owning view into a String
struct StringView {
  usize len;       ///< Length of the string view
  const char* str; ///< Pointer to the start of the string
};

extern const StringView NULL_SV;
#define SV_IS_NULL(sv_ptr) ((sv_ptr) == &NULL_SV)

bool   sv_equal(StringView a, StringView b);
i32    sv_compare(StringView a, StringView b);
void   sv_print(FILE *f, StringView sv);
void   sv_println(FILE *f, StringView sv);
StringView sv_from_string(const String string);
StringView sv_from_literal(const char *string);
StringView sv_from_stringptr(const String *string);

#define sv_create(x) _Generic((x), \
            String : sv_from_string, \
            String* : sv_from_stringptr, \
            char* : sv_from_literal \
        )(x)
