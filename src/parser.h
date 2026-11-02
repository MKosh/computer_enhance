#pragma once

#include "types.h"
#include "string8.h"
#include "allocator.h"
#include "stringset.h"

////////////////////////////////////////////////////////////////////////////////
/// Forward struct declarations
////////////////////////////////////////////////////////////////////////////////
typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;
typedef struct JsonField JsonField;
typedef struct JsonParser JsonParser;
typedef struct JsonResult JsonResult;
typedef struct JsonParserConfig JsonParserConfig;

////////////////////////////////////////////////////////////////////////////////
/// Type definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
} JsonType;

////////////////////////////////////////////////////////////////////////////////
///
struct JsonField {
    StringView key;
    JsonValue *value;
};

////////////////////////////////////////////////////////////////////////////////
///
struct JsonArray {
  JsonValue* items;
  size_t count;
};

////////////////////////////////////////////////////////////////////////////////
///
struct JsonObject {
  JsonField* fields;
  size_t count;
};

////////////////////////////////////////////////////////////////////////////////
///
struct JsonValue {
    JsonType type;
    union {
        bool       boolean;
        f64        number;
        StringView string;
        JsonArray  array;
        JsonObject object;
    } as;
};

////////////////////////////////////////////////////////////////////////////////
///
typedef enum {
    JSON_OK,
    JSON_ERROR_UNEXPECTED_TOKEN,
    JSON_ERROR_UNEXPECTED_END,
    JSON_ERROR_INVALID_ESCAPE,
    JSON_ERROR_INVALID_UNICODE,
    JSON_ERROR_NUMBER_OVERFLOW,
    JSON_ERROR_NESTING_LIMIT,
    JSON_ERROR_OUT_OF_MEMORY,
} JsonErrorCode;

////////////////////////////////////////////////////////////////////////////////
///
typedef struct {
    JsonErrorCode code;
    u64           line;
    StringView    message;   // points into a static string — no allocation
} JsonError;


////////////////////////////////////////////////////////////////////////////////
///
struct JsonParser {
  JsonParserConfig* config;
  StringView source;
  usize at;
  u64 line;
  bool had_error;
  JsonError error;
  StringSet* intern;
};

////////////////////////////////////////////////////////////////////////////////
///
struct JsonResult {
  JsonValue* root;
  JsonError  error;
};

////////////////////////////////////////////////////////////////////////////////
///
struct JsonParserConfig {
  Allocator* allocator;
  Allocator* intern_allocator;
  bool allow_comments;
};

////////////////////////////////////////////////////////////////////////////////
///
typedef struct JsonValueResult {
  bool ok;
  union {
    JsonValue value;
    JsonError error;
  };
} JsonValueResult;

////////////////////////////////////////////////////////////////////////////////
/// Function macros
////////////////////////////////////////////////////////////////////////////////
#define IS_NULL(value)   ((value)->type == JSON_NULL)
#define IS_BOOL(value)   ((value)->type == JSON_BOOL)
#define IS_NUMBER(value) ((value)->type == JSON_NUMBER)
#define IS_STRING(value) ((value)->type == JSON_STRING)
#define IS_ARRAY(value)  ((value)->type == JSON_ARRAY)
#define IS_OBJECT(value) ((value)->type == JSON_OBJECT)

#define AS_BOOL(value)   ((value)->as.boolean)
#define AS_NUMBER(value) ((value)->as.number)
#define AS_STRING(value) ((value)->as.string)
#define AS_ARRAY(value)  ((value)->as.array)
#define AS_OBJECT(value) ((value)->as.object)

////////////////////////////////////////////////////////////////////////////////
/// Function Declarations
////////////////////////////////////////////////////////////////////////////////
JsonParserConfig jp_parserConfigInit(Allocator* allocator, Allocator* intern, bool allow_comments);
JsonParser jp_parserInit(JsonParserConfig* jpc, StringView source);
JsonValueResult jp_parseJsonObject(JsonParser* jp);
JsonValueResult  jp_parseJsonArray(JsonParser* jp);
JsonValueResult jp_parseJsonString(JsonParser* jp);
StringView jp_parseJsonKey(JsonParser* jp);
JsonValueResult jp_parseJsonValue(JsonParser* jp);
JsonValueResult jp_parseFile(JsonParserConfig* jpc, StringView file);
JsonValueResult jp_parseJsonBoolean(JsonParser* jp);
JsonValueResult  jp_parseJsonNumber(JsonParser* jp);

JsonValue* jp_arrayAt(const JsonValue* array, usize index);
usize jp_arrayLength(const JsonValue* array);

usize jp_objectCount(const JsonValue* object);

// Retrieve object elements
JsonValue* jp_objectGet(const JsonValue* object, StringView key);
JsonValue* jp_objectGetSV(const JsonValue* object, StringView key);
JsonValue* jp_objectGetString(const JsonValue* object, String key);
JsonValue* jp_objectGetCharPtr(const JsonValue* object, char* key);
#define objectGet(o, k) _Generic((k), \
                          StringView: jp_objectGet, \
                          String: jp_objectGetString, \
                          char*: jp_objectGetCharPtr \
                          )(o, k)

JsonError jp_makeError(JsonParser* jp, JsonErrorCode code, const char* detail);

[[maybe_unused]] void pretend_main(const char* file_name);
