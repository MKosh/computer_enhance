#pragma once

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "types.h"
#include "mem.h"

typedef struct String String;
typedef struct StringView StringView;

////////////////////////////////////////////////////////////////////////////////
/// \brief Owning string
struct String {
  usize len; ///< Length of the string not including the null terminator.
  char* str; ///< Pointer to the start of the string.
};

String string_create(const char* string, usize length);
String string_readFile(const char* filename);
void   string_print(String string);
void   string_println(String string);
void   string_free(String string);
void   string_append(String* string, StringView suffix);
String string_clone(const String* string);

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
void   sv_print(FILE* f, StringView sv);
void   sv_println(FILE* f, StringView sv);
StringView sv_fromString(const String* string);
StringView sv_fromLiteral(const char* string);

