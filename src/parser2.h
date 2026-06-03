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
typedef struct JsonFieldNode JsonFieldNode;
typedef struct JsonField JsonField;
typedef struct JsonValueNode JsonValueNode;

struct JsonField {
    StringView key;
    JsonValue *value;
};

struct JsonFieldNode {
  JsonField field;
  JsonFieldNode* next;
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

struct JsonValueNode {
  JsonValue value;
  JsonValueNode* next;
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
    size_t        line;
    size_t        column;
    StringView    message;   // points into a static string — no allocation
} JsonError;

typedef struct JsonParser JsonParser;
typedef struct JsonResult JsonResult;
typedef struct JsonParserConfig JsonParserConfig;

struct JsonParser {
  JsonParserConfig* config;
  StringView source;
  usize at;
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

bool isAtEnd(JsonParser* jp);
void pretend_main(const char* file_name);
char peekNext(JsonParser* jp);
char peek(JsonParser* jp);
void advance(JsonParser* jp);
void advanceAndConsumeWhitespace(JsonParser* jp);
char advanceAndPeek(JsonParser* jp);
char peekAndAdvance(JsonParser* jp);
void advanceBy(JsonParser* jp, size_t n);
bool isDigit(char c);
bool isAlpha(char c);
void jp_consumeWhitespace(JsonParser* jp);
JsonParserConfig jp_parserConfigInit(Allocator* allocator, Allocator* intern, bool allow_comments);
JsonParser jp_parserInit(JsonParserConfig* jpc, StringView source);
JsonObject jp_parseJsonObject([[maybe_unused]] JsonParser* jp);
JsonArray jp_parseJsonArray([[maybe_unused]] JsonParser* jp);
f64 jp_parseJsonNumber(JsonParser* jp);
StringView jp_parseJsonString(JsonParser* jp);
bool jp_parseJsonBoolean(JsonParser* jp);
JsonResult jp_parseJsonValue(JsonParser* jp);
JsonResult jp_parseFile(JsonParserConfig* jpc, StringView file);

