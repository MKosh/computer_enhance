#include "parser2.h"
#include "types.h"
#include "string8.h"
#include "allocator.h"
#include "arena_list.h"
#include "fixed_buffer.h"
#include "metrics.h"
#include "haversine2.h"

#include <math.h>

/// 
/// TODO:
///   - [ ] String interning for JSON keys
///   - [ ] Add timers
///   - [ ] Add functions to retrieve values
///   - [ ] Haversine calculation
///

extern Profiler prof;
extern u64 TimeStampIndex;
extern u64 GlobalProfParent;


// Check if we're at the end of the file
bool isAtEnd(JsonParser* jp)
{
  return (jp->at >= jp->source.len);
}

// Look at the next character in the file, but don't advance
char peekNext(JsonParser* jp) {
  if (jp->at + 1 >= jp->source.len) {
    return EOF;
  }
  return jp->source.str[jp->at + 1];
}

// Look at the current character in the file, but don't advance
char peek(JsonParser* jp)
{
  return jp->source.str[jp->at];
}

// Advance to the next character in the file 
void advance(JsonParser* jp)
{
  jp->at++;
}

// Advance to the next character, then consume whitespace
void advanceAndConsumeWhitespace(JsonParser* jp)
{
  jp->at++;
  while (!isAtEnd(jp)) {
    switch(peek(jp)) {
      case ' ':
      case '\t':
      case '\r': {
                   jp->at++;
                   break;
                 }
      case '\n': {
                   jp->at++;
                   jp->line++;
                   break;
                 }
      default:
                 return;
    }
  }
}

// Advance one character then return that new character
char advanceAndPeek(JsonParser* jp)
{
  return jp->source.str[++(jp->at)];
}

// Return the current character then advance to the next one
char peekAndAdvance(JsonParser* jp)
{
  return jp->source.str[(jp->at)++];
}

// Advance n characters ahead if we can, otherwise go as far as possible
void advanceBy(JsonParser* jp, size_t n)
{
  if (jp->at + n > jp->source.len) {
    jp->at += jp->source.len - jp->at;
  } else {
    jp->at += n;
  }
}

// Check if a character is a digit
bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

// Check if a character is AlphaNumeric
bool isAlpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Skip over whitespace
void consumeWhitespace(JsonParser* jp)
{
  while (!isAtEnd(jp)) {
    switch(peek(jp)) {
      case ' ':
      case '\t':
      case '\r': {
                   jp->at++;
                   break;
                 }
      case '\n': {
                   jp->at++;
                   jp->line++;
                   break;
                 }
      default:
                 return;
    }
  }
}

JsonValue* jp_arrayAt(const JsonValue* array, usize index)
{
  assert(array && "nullptr as array");
  // if (array) { }
  // if (index) { }
  if (!IS_ARRAY(array)) {
    fprintf(stderr, "Error, value is not an array\n");
    return 0;
  }

  assert(index <= AS_ARRAY(array).count);

  return &(AS_ARRAY(array).items[index]);
}

// Get the number of values in the JSON array
usize jp_arrayLength(const JsonValue* array)
{
  assert(array && "nullptr as array");
  if (!IS_ARRAY(array)) {
    fprintf(stderr, "Error, value is not an array\n");
    return 0;
  }

  return AS_ARRAY(array).count;
}

// Get the number of fields in the JSON object
usize jp_objectCount(const JsonValue* object)
{
  assert(object && "nullptr as object");
  if (!IS_OBJECT(object)) {
    fprintf(stderr, "Error, value is not an object\n");
    fprintf(stderr, "  Value is type: %d\n", object->type);
    return 0;
  }

  return AS_OBJECT(object).count;
}

// Retrieve the JSON value with the corresponding key
JsonValue* jp_objectGet(const JsonValue* object, StringView key)
{
  assert(object && "nullptr as object");
  if (!IS_OBJECT(object)) {
    fprintf(stderr, "Error, value is not object\n");
    fprintf(stderr, "  Value is type: %d\n", object->type);
    return NULL;
  }
  
  usize count = jp_objectCount(object);
  for (usize i = 0; i < count; ++i) {
    JsonField* field = &AS_OBJECT(object).fields[i];
    if (strncmp(key.str, field->key.str, key.len) == 0) {
      return field->value;
    }
  }

  fprintf(stderr, "Couldn't find key %.*s\n", (int)key.len, key.str);
  return NULL;
}

// Initialize the parser config
JsonParserConfig jp_parserConfigInit(Allocator* allocator, Allocator* intern, bool allow_comments)
{
  return (JsonParserConfig){.allocator = allocator, .intern_allocator = intern, .allow_comments = allow_comments};
}

// Initialize the parser itself
JsonParser jp_parserInit(JsonParserConfig* jpc, StringView source)
{
  return (JsonParser){.config = jpc, .source = source, .at = 0, .line = 1, .had_error = false, .error = {.code = JSON_OK}};
}

// Parse a JsonObject and leave jp->at pointing at the first character after the closing '}'
JsonObject jp_parseJsonObject([[maybe_unused]] JsonParser* jp)
{
  // Stub function to do later. I will need to call jp_parseJsonValue to parse the value for each field
  // return (JsonObject){ 0 };

  JsonObject obj = { 0 };
  JsonFieldNode* head = NULL;
  JsonFieldNode* tail = NULL;
  usize count = 0;

  // Advance off of the opening '{'
  advanceAndConsumeWhitespace(jp);
  
  // Check for empty object
  if (peek(jp) == '}') {
    // obj.count = 0;
    // obj.fields = NULL;
    advance(jp);
    return obj;
  } 

  // TODO: Test both with and without a local arena allocator for building the
  // JsonFieldNode linked list:
  // u8 buf[4096];
  // Allocator* buffer = fixed_buffer_allocator_create(buf, 4096);

  while (!isAtEnd(jp)) {
    // Look for a string to denote the start of a key
    if ((peek(jp) != '"')) {
      // Error the field should have a key
      fprintf(stderr, "Error, expected an opening quote");
      static char err_message[] = "Unexpected token, expected opening quote";
      jp->had_error = true;
      jp->error = (JsonError){.code = JSON_ERROR_UNEXPECTED_TOKEN, .line = jp->line, .column = 0, {.len = strlen(err_message), .str = err_message}}; 
      break;
    }
  
    // We must be at the start of a key so parse it as a string
    StringView key = jp_parseJsonString(jp);
    if (jp->had_error) break;

    // Look for a colon to separate the key and value
    consumeWhitespace(jp);
    if (peek(jp) != ':') {
      fprintf(stderr, "Error, expected colon.\n");
      jp->had_error = true;
      static char err_message[] = "Expected colon";
      jp->error = (JsonError){.code = JSON_ERROR_UNEXPECTED_TOKEN, .line = jp->line, .column = 0, {.len = strlen(err_message), .str = err_message}};
      break;
    }

    // Advance off of the colon and consume whitespace up to the start of the value
    advanceAndConsumeWhitespace(jp);

    JsonField field = { 0 };
    JsonResult value = { 0 };

    field.key = key;

    // Parse the field's value
    value = jp_parseJsonValue(jp);
    if (jp->had_error) break;
    field.value = value.root;

    JsonFieldNode* node = allocator_new(jp->config->allocator, JsonFieldNode);
    node->field = field;
    node->next = NULL;

    if (tail) tail->next = node; 
    else head = node;
    tail = node;
    count++;

    // Consume whitespace after the value and look for a comma to know if the loop
    // should continue
    consumeWhitespace(jp);
    if (peek(jp) == ',') {
      advanceAndConsumeWhitespace(jp);
      continue;
    }
    if (peek(jp) == '}') {
      advance(jp);
      break;
    }
  }

  if (jp->had_error) {
    // Handle error
    return (JsonObject){ 0 };
  }

  // Turn the linked list into an array
  obj.fields = allocator_alloc(jp->config->allocator, count * sizeof(JsonField), alignof(JsonField));
  JsonFieldNode* current = head;
  for (usize i = 0; i < count; i++) {
    obj.fields[i] = current->field;
    current = current->next;
  }
  obj.count = count;

  return obj;
}

// Parse a JsonArray and leave jp->at pointing at the first character after the closing ']'
JsonArray jp_parseJsonArray(JsonParser* jp)
{
  JsonArray array = { 0 };
  usize count = 0;
  
  // Advance off of the starting '['
  advanceAndConsumeWhitespace(jp);

  // Check for an empty array
  if (peek(jp) == ']') {
    return array;
  }

  // Allocate space for the JsonArray values dynamically
  usize capacity = 32;
  JsonValue* temp = malloc(sizeof(JsonValue) * capacity);

  while (!isAtEnd(jp)) {
    JsonResult result = jp_parseJsonValue(jp);
    if (jp->had_error) { break; }

    if (count >= capacity) {
      capacity *= 2;
      temp = realloc(temp, sizeof(JsonValue) * capacity);
    }

    temp[count++] = *result.root;

    consumeWhitespace(jp);
    if (peek(jp) == ']') {
      advance(jp);
      break;
    }
    if (peek(jp) == ',') {
      advanceAndConsumeWhitespace(jp);
      continue;
    } else {
      fprintf(stderr, "Error expected ',' around line %ld\n", jp->line);
      static char err_message[] = "Error parsing array";
      jp->had_error = true;
      jp->error = (JsonError){.code = JSON_ERROR_UNEXPECTED_TOKEN, .line = jp->line, .column = 0, {.len = strlen(err_message), .str = err_message}};
      free(temp);
      break;
    }
  }
  
  if (jp->had_error) {
    return (JsonArray){ 0 };
  }

  array.count = count;
  array.items = allocator_alloc(jp->config->allocator, count * sizeof(JsonValue), alignof(JsonValue));
  memcpy(array.items, temp, count * sizeof(JsonValue));
  free(temp);

  return array;

}

// Parse a JsonNumber and leave jp->at pointing at the first character after the last digit
f64 jp_parseJsonNumber(JsonParser* jp)
{
  f64 number;

  const char* start = &jp->source.str[jp->at];
  char* end = NULL;
  number = strtod(start, &end);

  if (end == start) {
    fprintf(stderr, "Error parsing number, no valid numbers.\n");
  }

  if (isnan(number)) {
    fprintf(stderr, "Error parsing number, got NAN.\n");
  }

  if (jp->had_error) {
    static char err_message[] = "Error parsing number";
    jp->had_error = true;
    jp->error = (JsonError){.code = JSON_ERROR_UNEXPECTED_TOKEN, .line = jp->line, .column = 0, {.len = strlen(err_message), .str = err_message}};
    return NAN;
  }
  // advance by the number of characters strtod consumed.
  jp->at += (end - start);

  return number;
}

// Parse a JsonNumber and leave jp->at pointing at the first character after the closing quote
StringView jp_parseJsonString(JsonParser* jp)
{
  // Advance past the starting quote
  advance(jp); // advance is safe because we know we're at an opening quote
  usize count = 0;
  usize start = jp->at;
  while (!isAtEnd(jp) && peek(jp) != '"') {
    count++; // Count each non-quote character
    advance(jp);
    // Skip escape characters
    // if (peek(jp) == '\\') {
    //   advanceBy(jp, 2);
    // }
  }
  if (isAtEnd(jp)) {
    // At end but haven't seen the closing quote?
    fprintf(stderr, "Error unexpected end.\n");
    jp->had_error = true;
    static char err_message[] = "Unexpected end of file";
    jp->error = (JsonError){.code = JSON_ERROR_UNEXPECTED_END, .line = jp->line, .column = 0, {.len = strlen(err_message), .str = err_message}};
    return (StringView){.len = 0, .str = NULL};
  }

  // Advance past the closing quote
  advance(jp);

  // Add an extra space for the null terminator
  char* buf = allocator_alloc(jp->config->allocator, count + 1, 1);
  memcpy(buf, &(jp->source.str[start]), count);
  buf[count] = '\0';

  return (StringView){.len = count, .str = buf};
}

// Parse a Json Boolean and leave jp->at pointing at the first character after the last letter
// Possibly not used due to bools being simple to parse directly in jp_parseJsonValue
bool jp_parseJsonBoolean(JsonParser* jp);

JsonResult jp_parseJsonValue(JsonParser* jp)
{
  JsonValue value = { 0 };
  consumeWhitespace(jp);
  switch (peek(jp)) {
    case '{': {
                value.type = JSON_OBJECT;
                value.as.object = jp_parseJsonObject(jp);
                break;
              }
    case '[': {
                value.type = JSON_ARRAY;
                value.as.array = jp_parseJsonArray(jp);
                break;
              }
    case '"': {
                value.type = JSON_STRING;
                value.as.string = jp_parseJsonString(jp);
                break;
              }
    case 't': {
                if (jp->at + 4 <= jp->source.len &&
                    strncmp(&(jp->source.str[jp->at]), "true", 4) == 0) {
                  value.type = JSON_BOOL;
                  value.as.boolean = true;
                  advanceBy(jp, 4);
                }
                break;
              }
    case 'f': {
                if (jp->at + 5 <= jp->source.len &&
                    strncmp(&(jp->source.str[jp->at]), "false", 5) == 0) {
                  value.type = JSON_BOOL;
                  value.as.boolean = false;
                  advanceBy(jp, 5);
                }
                break;
              }
    case 'n': {
                if (jp->at + 4 <= jp->source.len &&
                    strncmp(&(jp->source.str[jp->at]), "null", 4) == 0) {
                  value.type = JSON_NULL;
                  // NULL carries no actual value
                  advanceBy(jp, 4);
                }
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
                value.type = JSON_NUMBER;
                value.as.number = jp_parseJsonNumber(jp);
                break;
              }
    default: {
               jp->had_error = true;
               static char err_message[] = "Unexpected character";
               jp->error = (JsonError){.code = JSON_ERROR_UNEXPECTED_TOKEN, .line = jp->line, .column = 0, {.len = strlen(err_message), .str = err_message}};
               break;
             }
  }
  if (jp->had_error) {
    // Handle error
    fprintf(stderr, "Error: %.*s\n", (int)jp->error.message.len, jp->error.message.str);
    return (JsonResult){.root = NULL, .error = jp->error};
  }

  JsonValue* result = allocator_new(jp->config->allocator, JsonValue);
  *result = value;
  return (JsonResult){.root = result, .error = {.code = JSON_OK}};
}

JsonResult jp_parseFile(JsonParserConfig* jpc, StringView file)
{
  JsonParser jp = jp_parserInit(jpc, file);
  JsonResult root_node = jp_parseJsonValue(&jp);
  if (root_node.root == NULL || jp.had_error == true) {
    // Handle error
    fprintf(stderr, "There was an error.\n");
  }
  return root_node;
}

void pretend_main(const char* file_name) {
  profilerInit(&prof);
  profilerBegin(&prof);
    Allocator* arena = arena_list_allocator_create(10 * 1024 * 1024);
    // Allocator* intern = arena_list_allocator_create(10 * 1024);

  ProfileBlock(read, "Read input");
    String file_contents = string_readFile(file_name);
  ProfileBlockEnd(read);

  JsonParserConfig jpc = jp_parserConfigInit(arena, NULL, true);
  ProfileBlock(parse, "Parse file");
    [[maybe_unused]] JsonResult root = jp_parseFile(&jpc, sv_fromString(&file_contents));
  ProfileBlockEnd(parse);

  ProfileBlock(Sum, "Sum");
    JsonValue* pairs = jp_objectGet(root.root, sv_fromLiteral("pairs"));
    usize elements = jp_arrayLength(pairs);
    printf("%ld sets of pairs.\n", elements);

    f64 sum = 0.;
    f64 N   = 0.;
    f64 run = 0.;
    for (usize i = 0; i < elements; ++i) {
      JsonValue* elem = jp_arrayAt(pairs, i);
      f64 x0 = AS_NUMBER(jp_objectGet(elem, (StringView){ .len = 2, .str = "x0"}));
      f64 y0 = AS_NUMBER(jp_objectGet(elem, (StringView){ .len = 2, .str = "y0"}));
      f64 x1 = AS_NUMBER(jp_objectGet(elem, (StringView){ .len = 2, .str = "x1"}));
      f64 y1 = AS_NUMBER(jp_objectGet(elem, (StringView){ .len = 2, .str = "y1"}));
      run = referenceHaversine(x0, y0, x1, y1);
      sum += run;
      N++;
      // printf("Pairs: (%g, %g), (%g, %g) -> %g\n", x0, y0, x1, y1, run);
    }

    printf("Haversine distance = %g/%g = %g\n", sum, N, sum/N);
  ProfileBlockEnd(Sum);

  ProfileBlock(dealloc, "Deallocation");
    allocator_destroy(arena);
    // allocator_destroy(intern);
    string_free(file_contents);
  ProfileBlockEnd(dealloc);

  profilerEndAndPrint(&prof);
}
