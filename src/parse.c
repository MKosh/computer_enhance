#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"

struct JsonDocument;

//#ifdef __STDC_ALLOC_LIB__
//#endif

////////////////////////////////////////////////////////////////////////////////
///
char* readFile(const char* filename, u64* size) {
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

  *size = file_size + 1;

  fclose(file);
  return buffer;
}

////////////////////////////////////////////////////////////////////////////////
///
typedef struct {
  f64 x0, y0;
  f64 x1, y1;
} Pair;

typedef struct {
  u64 num_pairs;
  f64* x0;
  f64* y0;
  f64* x1;
  f64* y1;
} Pairs;

typedef struct {

} Number;

typedef struct {

} Array;

typedef struct {
  char* data;
  size_t count;
} String;

typedef enum {
  JSON_NUMBER,
  JSON_STRING,
  JSON_OBJECT
} ValueType;

// struct JsonObject;
typedef struct JsonObject JsonObject;

typedef struct {
  ValueType type;
  union {
    f64 number;
    String string;
    JsonObject* obj;
  } as;
} JsonValue;


struct JsonObject {
  char* name;
  JsonValue* value;
  JsonObject* next;
};

typedef struct {
  JsonObject* first;
  JsonObject* last;
} JsonDocument;

typedef struct {
  String source;
  u64 at;
  bool had_error;
} JsonParser;

JsonObject* parseObject(JsonParser* parser);
////////////////////////////////////////////////////////////////////////////////
///
u64 allocPairs(Pairs* pairs, u64 num_pairs) {
  if (num_pairs == 0) {
    return 0;
  }
  pairs->x0 = malloc(sizeof(f64)*num_pairs);
  pairs->y0 = malloc(sizeof(f64)*num_pairs);
  pairs->x1 = malloc(sizeof(f64)*num_pairs);
  pairs->y1 = malloc(sizeof(f64)*num_pairs);
  if (pairs->x0 == NULL || pairs->y0 == NULL || pairs->x1 == NULL || pairs->y1 == NULL) {
    return 0;
  }
  pairs->num_pairs = num_pairs;
  return num_pairs;
}

////////////////////////////////////////////////////////////////////////////////
///
void freePairs(Pairs* pairs) {
  free(pairs->x0);
  free(pairs->y0);
  free(pairs->x1);
  free(pairs->y1);
}


static JsonDocument json;

////////////////////////////////////////////////////////////////////////////////
///
void initParser(JsonParser* parser, String source) {
  parser->source = source;
  parser->at = 0;
  parser->had_error = false;
}

////////////////////////////////////////////////////////////////////////////////
///
u64 consumeWhitespace(JsonParser* parser) {
  char current = parser->source.data[parser->at];
  u64 count = 0;
  bool is_aplhanumeric = false;
  while (current) {
    switch (current) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        current = parser->source.data[++parser->at];
        count++;
        break;
      default:
        is_aplhanumeric = true;
        break;
    }
    if (is_aplhanumeric) break;
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////
///
char peek(JsonParser* parser) {
  return parser->source.data[parser->at];
}

////////////////////////////////////////////////////////////////////////////////
///
char peekNext(JsonParser* parser) {
  if ((parser->at+1) >= parser->source.count) {
    return EOF;
  }

  return parser->source.data[parser->at+1];
}

////////////////////////////////////////////////////////////////////////////////
///
char advance(JsonParser* parser) {
  char current = parser->source.data[parser->at];
  parser->at++;
  return current;
}


bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

void freeJsonValue(JsonValue* value) {
  switch (value->type) {
    case JSON_NUMBER: {
      // printf("%f", value->as.number);
      free(value);
      break;
    }
    case JSON_STRING: {
      // printf("%s", value->as.string.data);
      free(value);
      break;
    }
    case JSON_OBJECT: {
      freeJsonValue(value->as.obj->value);
      break;
    }
  }
}

JsonValue* parseValue(JsonParser* parser) {
  printf("STARTING %s\n", __FUNCTION__);
  JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
  char buffer[256];
  size_t index = 0;
  switch (peek(parser)) {
    case '{': {
      value->type = JSON_OBJECT;
      value->as.obj = parseObject(parser);
      printf("Parsed: %s\n", value->as.obj->name);
      break;
    }
    case '"': {
      value->type = JSON_STRING;
      advance(parser);
      while (peek(parser) != '"') {
        buffer[index++] = advance(parser);
      }
      buffer[index] = '\0';
      value->as.string.count = index;
      value->as.string.data = strdup(buffer);
      advance(parser);
      printf("Parsed: %s\n", value->as.string.data);
      break;
    }
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      value->type = JSON_NUMBER;
      while (isDigit(peek(parser)) || peek(parser) == '.') {
        buffer[index++] = advance(parser);
      }
      buffer[index] = '\0';
      value->as.number = atof(buffer);
      printf("Parsed: %lf\n", value->as.number);
      break;
    }
  }

  printf("FINISHED %s\n", __FUNCTION__);
  return value;
}

////////////////////////////////////////////////////////////////////////////////
/// Might need a whole function for paring out sub elements of a json object
void parseElement();

////////////////////////////////////////////////////////////////////////////////
/// Not working for anything more complicated than a simple single key/value pair
JsonObject* parseObject(JsonParser* parser) {
  printf("STARTING %s\n", __FUNCTION__);
  JsonObject* current = (JsonObject*)malloc(sizeof(JsonObject));
  current->value = (JsonValue*)malloc(sizeof(JsonValue));
  char buffer[256];
  size_t index = 0;
  size_t stuck = 0;
  while (parser->at < parser->source.count-1) {
    if (peek(parser) == '{') {

      advance(parser);
      consumeWhitespace(parser);

      // Get name
      if (peek(parser) == '"') {
        advance(parser);
        while (peek(parser) != '"') {
          buffer[index++] = advance(parser);
        }
        buffer[index] = '\0';
        current->name = strdup(buffer);
        printf("Parsing object: %s\n", current->name);
        // current->name = (char*)malloc(sizeof(char)*index);
        // memcpy(current->name, buffer, index);
      }

      advance(parser);
      consumeWhitespace(parser);

      // Get value
      if (peek(parser) == ':') {
        advance(parser);
        consumeWhitespace(parser);
        current->value = parseValue(parser);
      }

      // How am I going to handle multiple key value pairs in a single object?
      //
      // if (peek(parser) == ',') {
      //   advance(parser);
      //   consumeWhitespace(parser);
      // }

      if (peek(parser) == '}') {
        advance(parser);
        consumeWhitespace(parser);
        // break;
      }

      if (peek(parser) == '\r') {
        break;
      }
      if (peek(parser) == '\n') {
        break;
      }
      if (peek(parser) == EOF) {
        break;
      }
    }

    if (stuck == parser->at) {
      printf("Parser got stuck!\n");
      printf("  parser at position %ld, out of %ld\n", parser->at, parser->source.count);
      printf("  Current character: '%c'\n", peek(parser));
      exit(EXIT_FAILURE);
    } else {
      stuck = parser->at;
    }

    consumeWhitespace(parser);
  }

  printf("FINISHED %s\n", __FUNCTION__);
  return current;
}

void initDocument(JsonDocument* doc) {
  printf("STARTING %s\n", __FUNCTION__);
  doc->first = NULL;
  doc->last = NULL;
  printf("FINISHED %s\n", __FUNCTION__);
}

void freeDocument(JsonDocument* doc) {
  for (JsonObject* current = doc->first; current != NULL;) {
    JsonObject* temp = current->next;
    free(current->name);
    freeJsonValue(current->value);
    free(current);
    current = NULL;
    if (temp != NULL) {
      current = temp;
    }
  }
  // free(doc);
  // freeObject(doc->first);
  // doc->first = NULL;
  // freeObject(doc->last);
  // doc->last = NULL;
}

JsonDocument* parse(JsonParser* parser) {
  printf("STARTING %s", __FUNCTION__);
  JsonDocument* doc = (JsonDocument*)malloc(sizeof(JsonDocument));
  initDocument(doc);

  while (parser->at < parser->source.count-1) {
    consumeWhitespace(parser);

    JsonObject* new = parseObject(parser);

    if (new == NULL) {
      exit(EXIT_FAILURE);
    }

    if (doc->first == NULL) {
      doc->first = new;
      doc->first->next = doc->last;
    } else {
      JsonObject* old_last = doc->last;
      doc->last = new;
      old_last->next = doc->last;
      doc->last->next = NULL;
    }
  }

  printf("FINISHED %s\n", __FUNCTION__);
  return doc;
}

void printJsonValue(JsonValue* value) {
  switch (value->type) {
    case JSON_NUMBER: {
      printf("%f", value->as.number);
      break;
    }
    case JSON_STRING: {
      printf("%s", value->as.string.data);
      break;
    }
    case JSON_OBJECT: {
      printJsonValue(value->as.obj->value);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
int main(int argc, char* argv[]) {
  printf("Starting\n");
  u64 file_size = 0;
  String source = {};
  if (argc >= 2) {
    source.data = readFile(argv[1], &file_size);
  } else {
    printf("Too few arguments.\n");
    exit(EXIT_FAILURE);
  }
  source.count = file_size;
  JsonParser parser = {};

  initParser(&parser, source);
  JsonDocument* doc = parse(&parser);
  for (JsonObject* current = doc->first; current != NULL; current = current->next) {
    printf("%s: ", current->name);
    printJsonValue(current->value);
    printf("\n");
  }

  freeDocument(doc);

  printf("FINISHED %s\n", __FUNCTION__);
  // free(source.data);

  return 0;
}
