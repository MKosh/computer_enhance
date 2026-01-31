#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "json.h"
#include "parser.h"

////////////////////////////////////////////////////////////////////////////////
///
JsonObject* parseJsonObject(JsonParser* parser)
{
  JsonObject* object = (JsonObject*)malloc(sizeof(JsonObject));
  JsonMember* last = NULL;

  char buffer[256];
  size_t index = 0;

  while (peek(parser) != '}') {
    consumeWhitespace(parser);

    // Look for a string
    if (peek(parser) == '"') {
      advance(parser);
      
      // Parse member name
      while (peek(parser) != '"') {
        buffer[index++] = advance(parser);
      }
      buffer[index] = '\0';

      // Make sure there's a ':' that seperates the name and element
      consumeWhitespace(parser);
      if (peek(parser) != ':') {
        fprintf(stderr, "Error, expected ':'\n");
        exit(EXIT_FAILURE);
      }

      JsonMember* new_member = (JsonMember*)malloc(sizeof(JsonMember));

      new_member->name = (String){ .data = strdup(buffer), .count = index };
      new_member->element = parseJsonElement(parser);
      new_member->next = NULL;

      if (object->members == NULL) {
        object->members = new_member;
        last = object->members;
      } else {
        last->next = new_member;
      }


    } else if (peek(parser) == '}') {
      // Empty object
      object->members = NULL;
    } else {
      fprintf(stderr, "Error, expected \"\n");
      exit(EXIT_FAILURE);
    }

  }

  return object;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonArray* parseJsonArray(JsonParser* parser)
{
  JsonArray* array = (JsonArray*)malloc(sizeof(JsonArray));

  return array;
}

////////////////////////////////////////////////////////////////////////////////
///
String parseJsonString(JsonParser* parser)
{
  String string;

  return string;
}

////////////////////////////////////////////////////////////////////////////////
///
f64 parseJsonNumber(JsonParser* parser)
{
  f64 number;

  return number;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonValue* parseJsonValue(JsonParser* parser)
{
  JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
  switch (peek(parser)) {
    case '{': {
      value->type = JSON_OBJECT;
      value->as.obj = parseJsonObject(parser);
      break;
    }
    case '[': {
      value->type = JSON_ARRAY;
      value->as.array = parseJsonArray(parser);
      break;
    }
    case '"': {
      value->type = JSON_STRING;
      value->as.string = parseJsonString(parser);
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
      value->as.number = parseJsonNumber(parser);
      break;
    }
    case 't': {
      value->type = JSON_BOOL;
      value->as.Bool = true;
      break;
    }
    case 'f': {
      value->type = JSON_BOOL;
      value->as.Bool = false;
      break;
    }
  }
  return value;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonElement* parseElement(JsonParser* parser)
{
  printf("Starting %s", __FUNCTION__);
  JsonElement* element = malloc(sizeof(JsonElement));

  while (parser->at < parser->source.count - 1) {
    consumeWhitespace(parser);
    element->value = parseJsonValue(parser);

  }
  // If there's an error, set had_error to true and probably return a nullptr
  // parser->had_error = true;
  // element = NULL;
  printf("Finished %s\n", __FUNCTION__);
  return element;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonDocument createJsonDocument()
{
  return (JsonDocument){ .root = NULL};
}

////////////////////////////////////////////////////////////////////////////////
///
void initJsonDocument(JsonDocument* doc)
{
  doc->root = NULL;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonElement* parseJsonRoot(JsonParser* parser)
{
  JsonElement* root = (JsonElement*)malloc(sizeof(JsonElement));

  if (peek(parser) != '\0') {
    root->value = parseJsonValue(parser);
  }

  if (parser->had_error) {
    fprintf(stderr, "Error parsing root element.\n");
    free(root);
    root = NULL;
  }

  return root;
}

////////////////////////////////////////////////////////////////////////////////
///
/// \returns true if parser had an error
bool parseJsonDoc(JsonParser *parser, JsonDocument *doc)
{
  doc->root = parseJsonRoot(parser);
  // Temp test value
  parser->had_error = true;
  return parser->had_error;
}

////////////////////////////////////////////////////////////////////////////////
///
void clearParser(JsonParser* parser)
{
  parser->source = (String){ .data = NULL, .count = 0 };
  parser->at = 0;
  parser->had_error = false;
}

////////////////////////////////////////////////////////////////////////////////
///
void initParser(JsonParser* parser, String source)
{
  clearParser(parser);
  parser->source = source;
}

////////////////////////////////////////////////////////////////////////////////
///
void freeParser(JsonParser* parser)
{
  freeString(&(parser->source));
  clearParser(parser);
}

////////////////////////////////////////////////////////////////////////////////
///
bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

////////////////////////////////////////////////////////////////////////////////
///
bool isAlpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

////////////////////////////////////////////////////////////////////////////////
///
char advance(JsonParser* parser)
{
  if (peekNext(parser) != '\0') {
    parser->at++;
  }
  return parser->source.data[parser->at--];
}

////////////////////////////////////////////////////////////////////////////////
///
char peekNext(JsonParser* parser)
{
  if (parser->at + 1 >= parser->source.count) {
    return EOF;
  }
  return parser->source.data[parser->at + 1];
}

////////////////////////////////////////////////////////////////////////////////
///
char peek(JsonParser* parser)
{
  return parser->source.data[parser->at];
}

////////////////////////////////////////////////////////////////////////////////
///
void consumeWhitespace(JsonParser* parser)
{
  for (;;) {
    switch(peek(parser)) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        advance(parser);
        break;
      default:
        return; 
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
int main(int argc, char* argv[]) {
  printf("Starting %s\n", __FUNCTION__);
  JsonParser parser;
  initParser(&parser, readFileStr(argv[1]));
  JsonDocument doc;
  initJsonDocument(&doc);

  if (parseJsonDoc(&parser, &doc)) {
    fprintf(stderr, "Cannot parse file.\n");
  }


  printf("File contents:\n%s\n", parser.source.data);

  freeParser(&parser);
  printf("Finished %s\n", __FUNCTION__);
  return 0;
}
