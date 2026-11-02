#include "string8.h"
#include "base_assert.h"

const StringView NULL_SV = { .len = 0, .str = NULL };
////////////////////////////////////////////////////////////////////////////////
/// String
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// String Creation/Destruction

////////////////////////////////////////////////////////////////////////////////
///
String string_from_cstr(const char *string, Allocator *allocator)
{
  String ret;
  u64 len = strlen(string);

  ret.str = (char *)allocator_alloc(allocator, sizeof(u8) * len + 1, alignof(u8));
  memcpy(ret.str, string, len);
  ret.str[len] = '\0';
  ret.len = len;

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
String string_from_stringview(const StringView sv, Allocator *allocator)
{
  String ret = { .len = sv.len };
  ret.str = (char *)allocator_alloc(allocator, sizeof(char) * sv.len + 1, alignof(char));
  memcpy(ret.str, sv.str, sv.len);
  ret.str[sv.len] = '\0';

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
String string_create(const char *string, usize length, Allocator *allocator)
{
  String ret = { .len = length };
  ret.str = (char *)allocator_alloc(allocator, sizeof(char) * length + 1, alignof(char));
  memcpy(ret.str, string, length);
  ret.str[length] = '\0';

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
String string_clone(const String string, Allocator *allocator)
{
  String ret = { .len = string.len };

  ret.str = (char *)allocator_alloc(allocator, sizeof(char)*string.len + 1, alignof(char));
  memcpy(ret.str, string.str, string.len);
  ret.str[string.len] = '\0';

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
String string_read_file(const char *filename, Allocator *allocator)
{
  ASSERT(filename && "Null filename");
  ASSERT(allocator && "No allocator");
  String ret;
  FILE *file = fopen(filename, "r");
  if (file == nullptr) {
    fprintf(stderr, "Error: could not open file.\n");
    exit(74);
  }

  printf("Reading file %s\n", filename);

  fseek(file, 0L, SEEK_END);
  usize file_size = ftell(file);
  file_size += 1; // Extra byte for the null terminator
  rewind(file);

  char *buffer = (char *)allocator_alloc(allocator, file_size, alignof(char));
  if (buffer == nullptr) {
    fprintf(stderr, "Error: could not allocate memory for file.\n");
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size - 1, file);
  if (bytes_read < file_size - 1) {
    fprintf(stderr, "Error: could not read file \"%s\".\n", filename);
    exit(74);
  }

  buffer[file_size - 1] = '\0';

  fclose(file); // Add check for fclose return value?
  ret.str = buffer;
  ret.len = file_size - 1; // Don't count the null terminator
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
String string_cstr_concat(const char *str1, const char *str2, Allocator *allocator)
{
  String ret;
  u64 len1 = strlen(str1);
  u64 len2 = strlen(str2);
  u64 total_len = len1 + len2;

  ret = (String){ .len = total_len, .str = (char *)allocator_alloc(allocator, sizeof(char)*(total_len) + 1, alignof(char))};
  memcpy(ret.str, str1, len1);
  memcpy(ret.str+len1, str2, len2);
  ret.str[total_len] = '\0';
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
void   string_free(String *string, Allocator *allocator)
{
  allocator_free(allocator, string->str, sizeof(char)*(string->len+1), alignof(char));
  string->str = nullptr;
  string->len = 0;

  string = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// String Operations

////////////////////////////////////////////////////////////////////////////////
void   string_append(String *string, StringView suffix, Allocator *allocator);

////////////////////////////////////////////////////////////////////////////////
// Check length first; skip memcmp for empty strings
bool   string_equal(String a, String b)
{
  // if (a.len != b.len) { return false; }
  // if (memcmp(a.str, b.str, a.len) == 0) { return true; }
  // else { return false; }
  return a.len == b.len && (a.len == 0 || memcmp(a.str, b.str, a.len) == 0);
}

////////////////////////////////////////////////////////////////////////////////
i32    string_compare(String a, String b);

////////////////////////////////////////////////////////////////////////////////
StringView string_sub(String string, i64 start, i64 len);

////////////////////////////////////////////////////////////////////////////////
StringView string_prefix(String string, i64 len);

////////////////////////////////////////////////////////////////////////////////
StringView string_suffix(String string, i64 len);

////////////////////////////////////////////////////////////////////////////////
u64    string_hash(String string)
{
  static constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
  static constexpr u64 FNV_PRIME = 1099511628211ULL;
  u64 hash = FNV_OFFSET_BASIS;
  for (usize i = 0; i < string.len; i++) {
    hash ^= (u8)string.str[i];
    hash *= FNV_PRIME;
  }
  return hash == 0 ? 1 : hash;
}

/// Printing
void string_print(FILE *f, String string)
{
  // assert(f && "Invalid FILE");
  ASSERT(f && "Invalid FILE");
  fprintf(f, "%.*s", (i32)string.len, string.str);
}

void string_println(FILE *f, String string)
{
  ASSERT(f && "Invalid FILE");
  fprintf(f, "%.*s\n", (i32)string.len, string.str);
}

////////////////////////////////////////////////////////////////////////////////
/// StringView
////////////////////////////////////////////////////////////////////////////////

bool   sv_equal(StringView a, StringView b)
{
  return a.len == b.len && (a.len == 0 || memcmp(a.str, b.str, a.len) == 0);
}

i32    sv_compare(StringView a, StringView b)
{
  // Temp impl
  return sv_equal(a, b);
}

void sv_print(FILE *f, StringView sv)
{
  fprintf(f, "%.*s", (i32)sv.len, sv.str);
}

void sv_println(FILE *f, StringView sv)
{
  fprintf(f, "%.*s\n", (i32)sv.len, sv.str);
}

StringView sv_from_string(const String string)
{
  return (StringView){ .len = string.len, .str = string.str };
}

StringView sv_from_literal(const char *string)
{
  return (StringView){ .len = strlen(string), .str = string };
}

StringView sv_from_stringptr(const String *string)
{
  return (StringView){ .len = string->len, .str = string->str };
}
