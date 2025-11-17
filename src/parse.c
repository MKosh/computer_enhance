#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

////////////////////////////////////////////////////////////////////////////////
///
char* readFile(const char* filename) {
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
  return buffer;
}

////////////////////////////////////////////////////////////////////////////////
///
typedef struct {
  f64 x0, y0;
  f64 x1, y1;
} Pair;

////////////////////////////////////////////////////////////////////////////////
///
int main() {
  printf("Starting\n");
  char* contents = readFile("data/template.json");

  free(contents);

  return 0;
}
