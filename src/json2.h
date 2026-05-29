#pragma once

// #include <stddef.h>
// #include "types.h"
// #include "string8.h"
//
// typedef enum {
//     JSON_NULL,
//     JSON_BOOL,
//     JSON_NUMBER,
//     JSON_STRING,
//     JSON_ARRAY,
//     JSON_OBJECT,
// } JsonType;
//
// typedef struct JsonValue JsonValue;
// typedef struct JsonArray JsonArray;
// typedef struct JsonObject JsonObject;
//
// typedef struct {
//     StringView key;
//     JsonValue *value;
// } JsonField;
//
// struct JsonArray {
//   JsonValue** items;
//   size_t count;
// };
//
// struct JsonObject {
//   JsonField* fields;
//   size_t count;
// };
//
// struct JsonValue {
//     JsonType type;
//     union {
//         bool       boolean;
//         f64        number;
//         StringView string;
//         JsonArray  array;
//         JsonObject object;
//     };
// };
//
// typedef enum {
//     JSON_OK,
//     JSON_ERROR_UNEXPECTED_TOKEN,
//     JSON_ERROR_UNEXPECTED_END,
//     JSON_ERROR_INVALID_ESCAPE,
//     JSON_ERROR_INVALID_UNICODE,
//     JSON_ERROR_NUMBER_OVERFLOW,
//     JSON_ERROR_NESTING_LIMIT,
//     JSON_ERROR_OUT_OF_MEMORY,
// } JsonErrorCode;
//
// typedef struct {
//     JsonErrorCode code;
//     size_t        line;
//     size_t        column;
//     StringView    message;   // points into a static string — no allocation
// } JsonError;
