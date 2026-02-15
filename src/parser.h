#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils.h"
#include "json.h"

////////////////////////////////////////////////////////////////////////////////
///
typedef struct {
  String source;
  u64 at;
  bool had_error;
} JsonParser;

////////////////////////////////////////////////////////////////////////////////
///
void consumeWhitespace(JsonParser* parser);

////////////////////////////////////////////////////////////////////////////////
///
char peek(JsonParser* parser);

////////////////////////////////////////////////////////////////////////////////
///
char peekNext(JsonParser* parser);

////////////////////////////////////////////////////////////////////////////////
///
char advance(JsonParser* parser);
bool isDigit(char c);

void initParser(JsonParser* parser, String source);
void clearParser(JsonParser* parser);
void freeParser(JsonParser* parser);

bool parseJsonDoc(JsonParser* parser, JsonDocument* doc);
JsonElement* parse(JsonParser* parser);
JsonElement* parseJsonElement(JsonParser* parser);
JsonObject* parseJsonObject(JsonParser* parser);
JsonArray* parseJsonArray(JsonParser* parser);
String parseJsonString(JsonParser* parser);
f64 parseJsonNumber(JsonParser* parser);
bool parseJsonBool(JsonParser* parser);

void printJsonDoc(JsonDocument* doc);
void printJsonElement(JsonElement* element);
void printJsonElements(JsonElement* element);
void printJsonValue(JsonValue* value);
void printJsonObject(JsonObject* obj);
void printJsonArray(JsonArray* array);

void freeJsonDoc(JsonDocument* doc);
void freeJsonElement(JsonElement* element);
void freeJsonElements(JsonElement* elements);
void freeJsonValue(JsonValue* value);
void freeJsonArray(JsonArray* array);
void freeJsonObject(JsonObject* object);
void freeJsonMember(JsonMember* member);

JsonMember* getMember(JsonElement* root, const char* name);
JsonMember* getJsonMember(JsonObject* obj);
JsonObject* getJsonObject(JsonValue* value);
JsonValue* getJsonValue(JsonElement* element);


// JsonElement getElement(String name);
