#include "parser2.h"
#include "types.h"
#include "string8.h"
#include "allocator.h"
#include "arena_list.h"
#include "fixed_buffer.h"
#include "metrics.h"
#include "haversine2.h"
#include "stringset.h"

#include <math.h>

/// 
/// TODO:
///   - [ ] Add tables for timers (and possibly values?)
///   - [x] Add timers
///   - [x] Add functions to retrieve values
///

extern Profiler prof;
extern u64 TimeStampIndex;
extern u64 GlobalProfParent;

[[maybe_unused]] static const char* err_messages[] = {
    "Unexpected token",
    "Unexpected end",
    "Invalid escape",
    "Invalid unicode",
    "Number overflow",
    "Nesting limit",
    "Out of memory"
};

typedef struct JsonFieldNode JsonFieldNode;
typedef struct JsonValueNode JsonValueNode;

struct JsonFieldNode {
    JsonField field;
    JsonFieldNode* next;
};

struct JsonValueNode {
    JsonValue value;
    JsonValueNode* next;
};

////////////////////////////////////////////////////////////////////////////////
/// Function Prototypes
static bool isAtEnd(JsonParser* jp);
void pretend_main(const char* file_name);
static char peekNext(JsonParser* jp);
static char peek(JsonParser* jp);
static void advance(JsonParser* jp);
static void advanceAndConsumeWhitespace(JsonParser* jp);
static char advanceAndPeek(JsonParser* jp);
static char peekAndAdvance(JsonParser* jp);
static void advanceBy(JsonParser* jp, size_t n);
static bool isDigit(char c);
static bool isAlpha(char c);
static void consumeWhitespace(JsonParser* jp);
/// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// Check if we're at the end of the file
static bool isAtEnd(JsonParser* jp)
{
    return (jp->at >= jp->source.len);
}

////////////////////////////////////////////////////////////////////////////////
/// Look at the next character in the file, but don't advance
[[maybe_unused]] static char peekNext(JsonParser* jp) {
    if (jp->at + 1 >= jp->source.len) {
        return EOF;
    }
    return jp->source.str[jp->at + 1];
}

////////////////////////////////////////////////////////////////////////////////
/// Look at the current character in the file, but don't advance
static char peek(JsonParser* jp)
{
    return jp->source.str[jp->at];
}

////////////////////////////////////////////////////////////////////////////////
/// Advance to the next character in the file 
static void advance(JsonParser* jp)
{
    jp->at++;
}

////////////////////////////////////////////////////////////////////////////////
/// Advance to the next character, then consume whitespace
static void advanceAndConsumeWhitespace(JsonParser* jp)
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

////////////////////////////////////////////////////////////////////////////////
/// Advance one character then return that new character
[[maybe_unused]] static char advanceAndPeek(JsonParser* jp)
{
    return jp->source.str[++(jp->at)];
}

////////////////////////////////////////////////////////////////////////////////
/// Return the current character then advance to the next one
[[maybe_unused]] static char peekAndAdvance(JsonParser* jp)
{
    return jp->source.str[(jp->at)++];
}

////////////////////////////////////////////////////////////////////////////////
/// Advance n characters ahead if we can, otherwise go as far as possible
static void advanceBy(JsonParser* jp, size_t n)
{
    if (jp->at + n > jp->source.len) {
        jp->at += jp->source.len - jp->at;
    } else {
        jp->at += n;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Check if a character is a digit
[[maybe_unused]] static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

////////////////////////////////////////////////////////////////////////////////
/// Check if a character is AlphaNumeric
[[maybe_unused]] static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

////////////////////////////////////////////////////////////////////////////////
/// Skip over whitespace
static void consumeWhitespace(JsonParser* jp)
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

////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
/// Get the number of values in the JSON array
usize jp_arrayLength(const JsonValue* array)
{
    assert(array && "nullptr as array");
    if (!IS_ARRAY(array)) {
        fprintf(stderr, "Error, value is not an array\n");
        return 0;
    }

    return AS_ARRAY(array).count;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the number of fields in the JSON object
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

////////////////////////////////////////////////////////////////////////////////
/// Retrieve the JSON value with the corresponding key
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

////////////////////////////////////////////////////////////////////////////////
/// Retrieve the JSON value with the corresponding key
JsonValue* jp_objectGetCharPtr(const JsonValue* object, char* key)
{
    assert(object && "nullptr as object");
    if (!IS_OBJECT(object)) {
        fprintf(stderr, "Error, value is not object\n");
        fprintf(stderr, "  Value is type: %d\n", object->type);
        return NULL;
    }

    usize len = strlen(key);
    usize count = jp_objectCount(object);
    for (usize i = 0; i < count; ++i) {
        JsonField* field = &AS_OBJECT(object).fields[i];
        if (strncmp(key, field->key.str, len) == 0) {
            return field->value;
        }
    }

    fprintf(stderr, "Couldn't find key %.*s\n", (int)len, key);
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the parser config
JsonParserConfig jp_parserConfigInit(Allocator* allocator, Allocator* intern, bool allow_comments)
{
    return (JsonParserConfig){.allocator = allocator, .intern_allocator = intern, .allow_comments = allow_comments};
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the parser itself
JsonParser jp_parserInit(JsonParserConfig* jpc, StringView source)
{
    i32 status = 0;
    StringSet* set = StringSet_create(509, jpc->intern_allocator, &status);
    return (JsonParser){.config = jpc, .source = source, .at = 0, .line = 1, .had_error = false, .intern = set };
}

////////////////////////////////////////////////////////////////////////////////
/// Parse a JsonObject and leave jp->at pointing at the first character after the closing '}'
JsonValueResult jp_parseJsonObject(JsonParser* jp)
{
    JsonValueResult result = { .ok = true };
    JsonObject obj = { 0 };
    JsonFieldNode* head = NULL;
    JsonFieldNode* tail = NULL;
    usize count = 0;

    // Advance off of the opening '{'
    advanceAndConsumeWhitespace(jp);

    // Check for empty object
    if (peek(jp) == '}') {
        advance(jp);
        result.value.type = JSON_OBJECT;
        result.value.as.object = obj;
        return result;
    } 

    // TODO: Test both with and without a local arena allocator for building the
    // JsonFieldNode linked list:
    u8 buf[4096];
    FixedBufferAllocator fba;
    fixed_buffer_allocator_init(&fba, buf, 4096);
    Allocator* allocator = &fba.base;

    while (!isAtEnd(jp)) {
        // Look for a string to denote the start of a key
        if ((peek(jp) != '"')) {
            // Error the field should have a key
            result.ok = false;
            result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "expected opening '\"'");
            break;
        }

        // We must be at the start of a key so parse it as a string
        StringView key = jp_parseJsonKey(jp);

        // Look for a colon to separate the key and value
        consumeWhitespace(jp);
        if (peek(jp) != ':') {
            result.ok = false;
            result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "expected ':'");
            break;
        }

        // Advance off of the colon and consume whitespace up to the start of the value
        advanceAndConsumeWhitespace(jp);

        JsonValueResult value = { 0 };

        // Parse the field's value
        value = jp_parseJsonValue(jp);
        if (value.ok == false) {
            result = value;
            break;
        }

        // Store the field in a new node
        JsonFieldNode* node = allocator_new(allocator, JsonFieldNode);
        node->field.key = key;
        node->field.value = allocator_new(jp->config->allocator, JsonValue);
        *node->field.value = value.value;
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
            // The object ended, move forward and stop parsing this object
            advance(jp);
            break;
        }
    }

    // Flatten the linked list into an array
    if (result.ok == true) {
        obj.fields = allocator_alloc(jp->config->allocator, count * sizeof(JsonField), alignof(JsonField));
        JsonFieldNode* current = head;
        for (usize i = 0; i < count; i++) {
            obj.fields[i] = current->field;
            current = current->next;
        }
        obj.count = count;
        result.value.type = JSON_OBJECT;
        result.value.as.object = obj;
    }

    // allocator_reset(jp->config->intern_allocator);
    // allocator_destroy(buffer);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Parse a JsonArray and leave jp->at pointing at the first character after the closing ']'
JsonValueResult jp_parseJsonArray(JsonParser* jp)
{
    JsonValueResult result = { .ok = true };
    JsonArray array = { 0 };
    usize count = 0;

    // Advance off of the starting '['
    advanceAndConsumeWhitespace(jp);

    // Check for an empty array
    if (peek(jp) == ']') {
        result.value = (JsonValue){ .type = JSON_ARRAY, .as.array = array};
        return result;
    }

    // Allocate space for the JsonArray values dynamically
    usize capacity = 32;
    JsonValue* temp = malloc(sizeof(JsonValue) * capacity);

    while (!isAtEnd(jp)) {
        JsonValueResult value = jp_parseJsonValue(jp);
        if (value.ok == false) {
            result.ok = false;
            result.error = value.error;
            break;
        }

        if (count >= capacity) {
            capacity *= 2;
            temp = realloc(temp, sizeof(JsonValue) * capacity);
        }

        temp[count++] = value.value;

        consumeWhitespace(jp);
        if (peek(jp) == ']') {
            advance(jp);
            break;
        }
        if (peek(jp) == ',') {
            advanceAndConsumeWhitespace(jp);
            // count++;
            continue;
        } else {
            result.ok = false;
            result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "expected ','");
            break;
        }
    }

    if (result.ok == true) {
        array.count = count;
        array.items = allocator_alloc(jp->config->allocator, count * sizeof(JsonValue), alignof(JsonValue));
        memcpy(array.items, temp, count * sizeof(JsonValue));
        result.value.type = JSON_ARRAY;
        result.value.as.array = array;
    }

    if (temp) free(temp);

    return result;

}

////////////////////////////////////////////////////////////////////////////////
/// Parse a JsonNumber and leave jp->at pointing at the first character after the last digit
JsonValueResult jp_parseJsonNumber(JsonParser* jp)
{
    f64 number;
    JsonValueResult result = { .ok = true };

    const char* start = &jp->source.str[jp->at];
    char* end = NULL;
    number = strtod(start, &end);

    if (end == start) {
        result.ok = false;
        result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "No valid numbers to parse.");
    }

    if (isnan(number)) {
        result.ok = false;
        result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "Trying to parse NAN.");
    }

    // advance by the number of characters strtod consumed.
    if (result.ok == true) {
        jp->at += (end - start);
        result.value.type = JSON_NUMBER;
        result.value.as.number = number;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Similar to jp_parseJsonString, but intern the key in the string set
StringView jp_parseJsonKey(JsonParser* jp)
{
  JsonValueResult result = { .ok = true };
  StringView key = NULL_SV;
  advance(jp); // advance is safe because we know we're at an opening quote
  usize count = 0;
  usize start = jp->at;
  while (!isAtEnd(jp) && peek(jp) != '"') {
    count++; // Count each non-quote character
    advance(jp);
  }
  if (isAtEnd(jp)) {
    result.ok = false;
    result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_END, "expected closing \"");
  }

  // Advance past the closing quote
  advance(jp);
  if (result.ok == true) {
    StringView raw = (StringView){ .len = count, .str = &(jp->source.str[start]) };
    key = StringSet_tryInsert(jp->intern, raw);
  }
  return key;
}

////////////////////////////////////////////////////////////////////////////////
/// Parse a JsonNumber and leave jp->at pointing at the first character after the closing quote
JsonValueResult jp_parseJsonString(JsonParser* jp)
{
    JsonValueResult result = { .ok = true };
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
        result.ok = false;
        result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_END, "expected closing \"");
    }

    // Advance past the closing quote
    advance(jp);

    if (result.ok == true) {
        // Add an extra space for the null terminator
        char* buf = allocator_alloc(jp->config->allocator, count + 1, 1);
        memcpy(buf, &(jp->source.str[start]), count);
        buf[count] = '\0';

        result.value.type = JSON_STRING;
        result.value.as.string = (StringView){ .len = count, .str = buf };
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Parse a Json Boolean and leave jp->at pointing at the first character after the last letter
/// Possibly not used due to bools being simple to parse directly in jp_parseJsonValue
JsonValueResult jp_parseJsonBoolean(JsonParser* jp);

////////////////////////////////////////////////////////////////////////////////
JsonValueResult jp_parseJsonValue(JsonParser* jp)
{
    JsonValueResult result = { .ok = true };
    consumeWhitespace(jp);
    switch (peek(jp)) {
        case '{': {
                      result = jp_parseJsonObject(jp);
                      break;
                  }
        case '[': {
                      result = jp_parseJsonArray(jp);
                      break;
                  }
        case '"': {
                      result = jp_parseJsonString(jp);
                      break;
                  }
        case 't': {
                      if (jp->at + 4 <= jp->source.len &&
                              strncmp(&(jp->source.str[jp->at]), "true", 4) == 0) {
                          result.value.type = JSON_BOOL;
                          result.value.as.boolean = true;
                          advanceBy(jp, 4);
                      } else {
                          result.ok = false;
                          result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "expected 'true'.");
                      }
                      break;
                  }
        case 'f': {
                      if (jp->at + 5 <= jp->source.len &&
                              strncmp(&(jp->source.str[jp->at]), "false", 5) == 0) {
                          result.value.type = JSON_BOOL;
                          result.value.as.boolean = false;
                          advanceBy(jp, 5);
                      } else {
                          result.ok = false;
                          result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "expected 'false'.");
                      }
                      break;
                  }
        case 'n': {
                      if (jp->at + 4 <= jp->source.len &&
                              strncmp(&(jp->source.str[jp->at]), "null", 4) == 0) {
                          result.value.type = JSON_NULL;
                          // NULL carries no actual value
                          advanceBy(jp, 4);
                      } else {
                          result.ok = false;
                          result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "expected 'null'.");
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
                      result = jp_parseJsonNumber(jp);
                      break;
                  }
        default: {
                     result.ok = false;
                     result.error = jp_makeError(jp, JSON_ERROR_UNEXPECTED_TOKEN, "unexpected symbol.");
                     break;
                 }
    }

    return result;

}

////////////////////////////////////////////////////////////////////////////////
/// Make an error value
JsonError
jp_makeError(JsonParser *jp, JsonErrorCode code, const char *detail)
{
    return (JsonError){ code, jp->line, sv_fromLiteral(detail) };
}

////////////////////////////////////////////////////////////////////////////////
JsonValueResult jp_parseFile(JsonParserConfig* jpc, StringView file)
{
    JsonParser jp = jp_parserInit(jpc, file);
    JsonValueResult root = jp_parseJsonValue(&jp);
    if (root.ok == false) {
        // Handle error
        fprintf(stderr, "ERROR at %ld %s: %s.\n", root.error.line, err_messages[root.error.code], root.error.message.str);
    }
    return root;
}

////////////////////////////////////////////////////////////////////////////////
[[maybe_unused]] void pretend_main(const char* file_name) {
    profilerInit(&prof);
    profilerBegin(&prof);
    Allocator* arena = arena_list_allocator_create(MiB(10));
    u8* buffer = malloc(KiB(10));
    Allocator* buf   = fixed_buffer_allocator_create(buffer, KiB(10));
    // Allocator* intern = arena_list_allocator_create(10 * 1024);

    ProfileBlock(read, "Read input");
    String file_contents = String_readFile(file_name, NULL);
    ProfileBlockEnd(read);

    JsonParserConfig jpc = jp_parserConfigInit(arena, buf, true);
    ProfileBlock(parse, "Parse file");
    [[maybe_unused]] JsonValueResult root = jp_parseFile(&jpc, sv_create(&file_contents));
    if (root.ok == false) {
        allocator_destroy(arena);
        // allocator_destroy(intern);
        String_free(&file_contents, NULL);
    }
    ProfileBlockEnd(parse);

    ProfileBlock(Sum, "Sum");
    JsonValue* pairs = jp_objectGet(&root.value, sv_fromLiteral("pairs"));
    usize elements = jp_arrayLength(pairs);
    printf("%ld sets of pairs.\n", elements);

    f64 sum = 0.;
    f64 N   = 0.;
    f64 run = 0.;
    for (usize i = 0; i < elements; ++i) {
        JsonValue* elem = jp_arrayAt(pairs, i);
        f64 x0 = AS_NUMBER(objectGet(elem, "x0"));
        f64 y0 = AS_NUMBER(objectGet(elem, "y0"));
        f64 x1 = AS_NUMBER(objectGet(elem, "x1"));
        f64 y1 = AS_NUMBER(objectGet(elem, "y1"));
        run = referenceHaversine(x0, y0, x1, y1);
        sum += run;
        N++;
    }

    printf("Haversine distance = %g/%g = %g\n", sum, N, sum/N);
    ProfileBlockEnd(Sum);

    ProfileBlock(dealloc, "Deallocation");
    allocator_destroy(arena);
    allocator_destroy(buf);
    free(buffer);
    String_free(&file_contents, NULL);
    ProfileBlockEnd(dealloc);

    profilerEndAndPrint(&prof);
}
