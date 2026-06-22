#pragma once

#include "types.h"
#include "string8.h"
#include "allocator.h"

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
} JsonType;

typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;
typedef struct JsonField JsonField;

struct JsonField {
    StringView key;
    JsonValue *value;
};

struct JsonArray {
  JsonValue* items;
  size_t count;
};

struct JsonObject {
  JsonField* fields;
  size_t count;
};

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

typedef struct {
    JsonErrorCode code;
    u64           line;
    StringView    message;   // points into a static string — no allocation
} JsonError;

typedef struct JsonParser JsonParser;
typedef struct JsonResult JsonResult;
typedef struct JsonParserConfig JsonParserConfig;

struct JsonParser {
  JsonParserConfig* config;
  StringView source;
  usize at;
  u64 line;
  bool had_error;
  JsonError error;
};

struct JsonResult {
  JsonValue* root;
  JsonError  error;
};

struct JsonParserConfig {
  Allocator* allocator;
  Allocator* intern_allocator;
  bool allow_comments;
};

#define JP_DEFINE_RESULT(Name, T) \
    typedef struct { \
        bool ok; \
        union { \
            T         value; \
            JsonError error; \
        }; \
    } Name;

JP_DEFINE_RESULT(JsonValueResult, JsonValue);
JP_DEFINE_RESULT(JsonObjectResult, JsonObject);
JP_DEFINE_RESULT(JsonArrayResult,  JsonArray);
JP_DEFINE_RESULT(JsonNumberResult, f64);
JP_DEFINE_RESULT(JsonStringResult, StringView);
JP_DEFINE_RESULT(JsonBoolResult,   bool);

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

JsonParserConfig jp_parserConfigInit(Allocator* allocator, Allocator* intern, bool allow_comments);
JsonParser jp_parserInit(JsonParserConfig* jpc, StringView source);
JsonObject jp_parseJsonObject(JsonParser* jp);
JsonArray  jp_parseJsonArray(JsonParser* jp);
StringView jp_parseJsonString(JsonParser* jp);
JsonValueResult jp_parseJsonValue(JsonParser* jp);
JsonResult jp_parseFile(JsonParserConfig* jpc, StringView file);
bool jp_parseJsonBoolean(JsonParser* jp);
JsonNumberResult  jp_parseJsonNumber(JsonParser* jp);

JsonValue* jp_arrayAt(const JsonValue* array, usize index);
usize jp_arrayLength(const JsonValue* array);

usize jp_objectCount(const JsonValue* object);
JsonValue* jp_objectGet(const JsonValue* object, StringView key);

JsonError jp_makeError(JsonParser* jp, JsonErrorCode code, const char* detail);


// #define JP_TRY(result_expr, ResultType) \
//   ({ typeof(result_expr) _r = (result_expr); \
//      if (!_r.ok) return (ResultType){ .ok = false, .error = _r.error }; \
//      _r.value; })
