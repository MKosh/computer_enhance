#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "string8.h"

const StringView NULL_SV = { .len = 0, .str = NULL };

////////////////////////////////////////////////////////////////////////////////
/// \brief read a file into a normal C buffer
char* readFile(const char* filename, u64* size)
{
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file.\n");
    exit(74);
  }

  printf("Reading file %s\n", filename);

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);
  
  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", filename);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    fprintf(stderr, "Could not read file \"%s\".\n", filename);
    exit(74);
  }
  buffer[file_size] = '\0';

  if (size != NULL) {
    *size = file_size + 1;
  }

  fclose(file);
  return buffer;
}

////////////////////////////////////////////////////////////////////////////////
///
String string_readFile(const char* filename)
{
  String ret;
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file.\n");
    exit(74);
  }

  printf("Reading file %s\n", filename);

  fseek(file, 0L, SEEK_END);
  usize file_size = ftell(file);
  file_size += 1; // Add an extra byte for the null terminator
  rewind(file);
  
  char* buffer = (char*)malloc(file_size);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", filename);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size - 1, file);
  if (bytes_read < file_size - 1) {
    fprintf(stderr, "Could not read file \"%s\".\n", filename);
    exit(74);
  }
  buffer[file_size - 1] = '\0';

  fclose(file);
  ret.str = buffer;
  ret.len = file_size;
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
///
void string_free(String string)
{
  free(string.str);
  string.str = NULL;
  string.len = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
void string_println(String string)
{
  printf("%.*s\n", (int)string.len, string.str);
}

////////////////////////////////////////////////////////////////////////////////
///
void string_print(String string)
{
  printf("%.*s", (int)string.len, string.str);
}

StringView sv_fromString(const String* string)
{
  return (StringView){.len = string->len, .str = string->str};
}

StringView sv_fromLiteral(const char* string)
{
  return (StringView){ .len = strlen(string), .str = string };
}
