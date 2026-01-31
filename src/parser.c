#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "utils.h"
#include "json.h"
#include "parser.h"

static int DEBUG_ = 0;

////////////////////////////////////////////////////////////////////////////////
///
void printJsonArray(JsonArray* array)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (array == NULL) {
    return;
  }

  printf("[");
  // for (JsonElement* element = array->elements; element != NULL; element = element->next) {
  printJsonElements(array->elements);
  // }

  printf("]");
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  fflush(stdout);
  // exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////////////////////
///
void printJsonObject(JsonObject* obj)
{ 
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (obj == NULL) {
    return;
  }

  bool first = true;

  printf("{");
  for (JsonMember* member = obj->members; member != NULL; member = member->next) {
    printf("\"");
    printString(member->name);
    printf("\"");
    printf(": ");
    printJsonElement(member->element);
    if (member->next != NULL) {
      printf(", ");
    }
  }
  printf("}");
  fflush(stdout);
  // if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  // exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////////////////////
///
void printJsonValue(JsonValue* value)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (value == NULL) {
    return;
  }

  switch (value->type) {
    case JSON_BOOL: {
      if (DEBUG_) printf("Printing JSON_BOOL\n");
      if (value->as.Bool) {
        printf("true");
      } else {
        printf("false");
      }
      break;
    }
    case JSON_NUMBER: {
      if (DEBUG_) printf("Printing JSON_NUMBER\n");
      printf("%g", value->as.number);
      break;
    }
    case JSON_STRING: {
      if (DEBUG_) printf("Printing JSON_STRING\n");
      printf("%s", value->as.string.data);
      break;
    }
    case JSON_OBJECT: {
      if (DEBUG_) printf("Printing JSON_OBJECT\n");
      printJsonObject(value->as.obj);
      break;
    }
    case JSON_ARRAY: {
      if (DEBUG_) printf("Printing JSON_ARRAY\n");
      printJsonArray(value->as.array);
      break;
    }
  }

  fflush(stdout);
  // exit(EXIT_FAILURE);
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}


////////////////////////////////////////////////////////////////////////////////
///
void printJsonElements(JsonElement* element)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (element == NULL) {
    return;
  }

  for (JsonElement* e = element; e != NULL; e = e->next) {
    // if (DEBUG_) printf("Printing element!\n");
    printJsonValue(e->value);
    if (e->next != NULL) printf(","); 
  }
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void printJsonElement(JsonElement* element)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (element == NULL) {
    return;
  }

  for (JsonElement* e = element; e != NULL; e = element->next) {
    // if (DEBUG_) printf("Printing element!\n");
    printJsonValue(e->value);
  }
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void printJsonDoc(JsonDocument* doc)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (doc != NULL && doc->root != NULL) {
    printJsonElement(doc->root);
  }
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
JsonObject* parseJsonObject(JsonParser* parser)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (DEBUG_) printf("Parsing starting on char: %c\n", peek(parser));
  JsonObject* object = (JsonObject*)malloc(sizeof(JsonObject));
  object->members = NULL;
  JsonMember* temp = NULL;

  char buffer[256];
  size_t index = 0;

  // Look for object with no members
  advance(parser);
  if (DEBUG_) printf("After advancing, on char: %c\n", peek(parser));
  consumeWhitespace(parser);
  if (peek(parser) == '}') {
    object->members = NULL;
    return object;
  }

  while (true) {
    consumeWhitespace(parser);

    // Look for a string
    if (peek(parser) == '"') {
      advance(parser);

      index = 0;
      
      // Parse member name
      while (peek(parser) != '"') {
        buffer[index++] = advance(parser);
        // Name missing closing quote.
        if (index >= 255) {
          fprintf(stderr, "Error, expected closing \" in member name.\n");
          parser->had_error = true;
          object = NULL;
          goto end;
        }
      }

      buffer[index] = '\0';
      advance(parser);

      // Make sure there's a ':' that seperates the name and element
      consumeWhitespace(parser);
      if (peek(parser) != ':') {
        fprintf(stderr, "Error, expected ':'\n");
        parser->had_error = true;
        object = NULL;
        break;
      }
      advance(parser);
      consumeWhitespace(parser);

      JsonMember* new_member = (JsonMember*)malloc(sizeof(JsonMember));

      new_member->name = (String){ .data = strdup(buffer), .count = index };
      new_member->element = parseJsonElement(parser);
      new_member->next = NULL;

      // if (object->members == NULL) {
      //   object->members = new_member;
      //   temp = object->members;
      // } else {
      //   temp->next = new_member;
      //   temp = new_member;
      // }
      if (object->members == NULL) {
        object->members = new_member;
      } else {
        temp = object->members;
        while (temp->next != NULL) {
          temp = temp->next;
        }
        temp->next = new_member;
      }

      consumeWhitespace(parser);
      if (peek(parser) == ',') {
        advance(parser);
        continue;
      } else if (peek(parser) == '}') {
        advance(parser);
        break;
      }

    } else {
      fprintf(stderr, "Error, expected '\"'\n");
      fprintf(stderr, "Failed on char: '%c'", peek(parser));
      parser->had_error = true;
      object = NULL;
      break;
      // exit(EXIT_FAILURE);
    }

  }

end:
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return object;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonArray* parseJsonArray(JsonParser* parser)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  JsonArray* array = (JsonArray*)malloc(sizeof(JsonArray));
  array->elements = NULL;

  JsonElement* temp = (JsonElement*)malloc(sizeof(JsonElement));

  if (array == NULL || temp == NULL) {
    fprintf(stderr, "Error allocating memory for array element.\n");
    parser->had_error = true;
    return NULL;
  }

  for (;;) {
    advance(parser);
    consumeWhitespace(parser);
    printf("Current char: parser[%ld] = %c\n", parser->at, peek(parser));
    if (peek(parser) == ']') {
      break;
      // return array;
    }
    
    JsonElement* new_element = parseJsonElement(parser);

    if (array->elements == NULL) {
      array->elements = new_element;
      // temp = array->elements;
    } else {
      temp = array->elements;
      while (temp->next != NULL) {
        temp = temp->next;
      }
      temp->next = new_element;
      // temp->next = new_element;
      // temp = new_element;
    }

    consumeWhitespace(parser);
    if (peek(parser) == ',') {
      continue;
    } else if (peek(parser) == ']') {
      advance(parser);
      break;
    }

   }

  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
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
  char buffer[256];
  size_t index = 0;
  
  while (isDigit(peek(parser)) || peek(parser) == '.') {
    buffer[index++] = advance(parser);
  }
  buffer[index] = '\0';
  number = atof(buffer);
  if (isnan(number)) {
    fprintf(stderr, "Error parsing number, got NAN.\n");
    parser->had_error = true;
  }

  return number;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonValue* parseJsonValue(JsonParser* parser)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (DEBUG_) printf("Source at: parse[%ld] = %c\n", parser->at, peek(parser));
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
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return value;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonElement* parseJsonElement(JsonParser* parser)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  JsonElement* element = malloc(sizeof(JsonElement));

  consumeWhitespace(parser);
  element->value = parseJsonValue(parser);
  element->next = NULL;
    
  // If there's an error, set had_error to true and probably return a nullptr
  // parser->had_error = true;
  // element = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return element;
}

////////////////////////////////////////////////////////////////////////////////
///
JsonElement* parseElement(JsonParser* parser)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  JsonElement* element = malloc(sizeof(JsonElement));

  while (parser->at < parser->source.count - 1) {
    consumeWhitespace(parser);
    element->value = parseJsonValue(parser);
  }
  // If there's an error, set had_error to true and probably return a nullptr
  // parser->had_error = true;
  // element = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
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
    root->next = NULL;
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
  // parser->had_error = false;
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
  if (DEBUG_) printf("Current char: parser[%ld] = %c\n", parser->at, parser->source.data[parser->at]);
  if (peekNext(parser) != '\0') {
    parser->at++;
  }
  if (DEBUG_) printf("After step: parser[%ld] = %c\n", parser->at, parser->source.data[parser->at]);
  return parser->source.data[parser->at - 1];
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
  int debug_arg = 0;
  char* file_arg;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-debug") == 0) {
      DEBUG_ = 1;
    } else {
      file_arg = argv[i];
    }
  }

  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);

  JsonParser parser;
  initParser(&parser, readFileStr(file_arg));
  JsonDocument doc;
  initJsonDocument(&doc);

  printf("File contents:\n%s\n", parser.source.data);

  if (parseJsonDoc(&parser, &doc)) {
    fprintf(stderr, "Cannot parse file.\n");
  } else {
    printf("JsonDoc contents:\n");
    printJsonDoc(&doc);
  }

  freeParser(&parser);
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return 0;
}
