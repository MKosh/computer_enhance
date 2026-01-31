#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

////////////////////////////////////////////////////////////////////////////////
///
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
String readFileStr(const char* filename)
{
  String ret;
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

  fclose(file);
  ret.data = buffer;
  ret.count = file_size + 1;
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
///
void freeString(String* string)
{
  free(string->data);
  string->data = NULL;
  string->count = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
void printString(String string)
{
  printf("%s", string.data);
}
