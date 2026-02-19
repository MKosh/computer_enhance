#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "utils.h"
#include "json.h"
#include "parser.h"


////////////////////////////////////////////////////////////////////////////////
///
JsonObject* getJsonObject(JsonValue* value)
{
  if (value->type != JSON_OBJECT) {
    return NULL;
  }
  return value->as.obj;
}

////////////////////////////////////////////////////////////////////////////////
///
// JsonMember* getJsonMember(JsonObject* obj);

////////////////////////////////////////////////////////////////////////////////
///
JsonMember* getMember(JsonElement* root, const char* name)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (root == NULL) {
    return NULL;
  }

  JsonMember* ret = NULL;
  JsonMember* member;
  JsonObject* obj;
  for (JsonElement* e = root; e != NULL; e = e->next) {
    obj = getJsonObject(e->value);
    if (obj == NULL) return NULL;
    member = obj->members;
    while (member != NULL) {
      if (strcmp(name, member->name.data) == 0) {
        ret = member;
        goto found;
      } else {
        member = member->next;
      }
    }
  }

found:
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
///
f64 getJsonNumber(JsonElement* element, const char* name)
{
  if (element == NULL) {
    fprintf(stderr, "NULL json element to json number function.\n");
    return NAN;
  }

  if (element->value->type != JSON_OBJECT) {
    fprintf(stderr, "Invalid value type\n");
    return NAN;
  }

  JsonObject* obj = element->value->as.obj;
  for (JsonMember* member = obj->members; member != NULL; member = member->next) {
    if (strcmp(name, member->name.data) == 0) {
      if (member->element->value->type == JSON_NUMBER) {
        return member->element->value->as.number;
      } else {
        fprintf(stderr, "Found member named '%s', but it was not a number as expected.\n", member->name.data);
        return NAN;
      }
    }
  }

  fprintf(stderr, "No member named '%s' found.\n", name);
  return NAN;
}

////////////////////////////////////////////////////////////////////////////////
///
void* getJsonValue(JsonElement* element)
{
  if (element == NULL) {
    return NULL;
  }
  void* ret;

  switch (element->value->type) {
    case JSON_BOOL: {
      ret = &element->value->as.Bool;
      break;
    }
    case JSON_NUMBER: {
      ret = &element->value->as.number;
      break;
    }
    case JSON_STRING: {
      ret = &element->value->as.string;
      break;
    }
    case JSON_ARRAY: {
      ret = element->value->as.array;
      break;
    }
    case JSON_OBJECT: {
      ret = element->value->as.obj;
      break;
    }
  }

  if (ret == NULL) {
    fprintf(stderr, "Error, returning NULL json value.\n");
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
///
void freeJsonDoc(JsonDocument* doc)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (doc == NULL) {
    fprintf(stderr, "Error, trying to free a NULL document.\n");
    return;
  }

  freeJsonElements(doc->root);
  
  // free(doc);
  // doc = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void freeJsonElements(JsonElement* elements)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (elements == NULL) {
    fprintf(stderr, "Error, freeing NULL element.\n");
    return;
  }

  JsonElement* temp;
  for (JsonElement* e = elements; e != NULL;) {
    temp = e->next;
    freeJsonValue(e->value);
    free(e);
    e = temp;
    // e = e->next;
    // free(last);
  }

  // elements = NULL;

  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void freeJsonValue(JsonValue* value)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (value == NULL) {
    fprintf(stderr, "Error, freeing NULL value.\n");
    return;
  }

  switch (value->type) {
    case JSON_BOOL:
    case JSON_NUMBER:
      if (DEBUG_) printf("Freeing number: %g\n", value->as.number);
      break;
    case JSON_STRING: {
      if (DEBUG_) printf("Freeing string: %s\n", value->as.string.data);
      freeString(&(value->as.string));
      break;
    }
    case JSON_ARRAY: {
      freeJsonArray(value->as.array);
      break;
    }
    case JSON_OBJECT: {
      freeJsonObject(value->as.obj);
      break;
    }
  }

  free(value);
  value = NULL;

  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void freeJsonArray(JsonArray* array)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (array == NULL) {
    return;
  }
  freeJsonElements(array->elements);
  free(array);
  array = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void freeJsonMember(JsonMember* member)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (member == NULL) {
    return;
  }

  if (DEBUG_) printf("Freeing member: %s\n", member->name.data);
  freeString(&member->name);
  freeJsonElements(member->element);
  member->next = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
///
void freeJsonObject(JsonObject* object)
{
  if (DEBUG_) printf("Starting %s\n", __FUNCTION__);
  if (object == NULL) {
    return;
  }

  JsonMember* temp;
  for (JsonMember* mem = object->members; mem != NULL;) {
    temp = mem->next;
    freeJsonMember(mem);
    free(mem);
    mem = temp;
  }
  free(object);
  object = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
}

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

  // bool first = true;

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
    if (e->next != NULL) printf(",\n"); 
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

  // JsonElement* temp = (JsonElement*)malloc(sizeof(JsonElement));
  JsonElement* temp;

  if (array == NULL) {
    fprintf(stderr, "Error allocating memory for array element.\n");
    parser->had_error = true;
    return NULL;
  }

  for (;;) {
    advance(parser);
    consumeWhitespace(parser);
    if (DEBUG_) printf("Current char: parser[%ld] = %c\n", parser->at, peek(parser));
    if (peek(parser) == ']') {
      break;
      // return array;
    }
    
    JsonElement* new_element = parseJsonElement(parser);

    if (array->elements == NULL) {
      array->elements = new_element;
      temp = array->elements;
    } else {
      // temp = array->elements;
      while (temp->next != NULL) { // Hottest path?
        temp = temp->next; //hottest path?
      }
      temp->next = new_element;
      // temp->next = new_element;
      temp = new_element;
    }

    consumeWhitespace(parser);
    if (peek(parser) == ',') {
      continue;
    } else if (peek(parser) == ']') {
      advance(parser);
      break;
    }

   }

  // free(temp);
  // temp = NULL;
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return array;
}

////////////////////////////////////////////////////////////////////////////////
///
String parseJsonString(JsonParser* parser)
{
  String string = { .data = NULL, .count = 0};
  advance(parser); // Advance off of the starting quotation mark.

  // Keep track of where the string starts to fill the data buffer later.
  u64 start = parser->at;
  while(peek(parser) != '"') {
    string.count++;
    advance(parser);
  }
  string.count++; // Don't forget space for the null terminator.

  string.data = malloc(sizeof(char)*string.count);

  parser->at = start;

  for (u64 i = 0; i < string.count - 1; ++i) {
    string.data[i] = advance(parser);
  }
  string.data[string.count - 1] = '\0';

  consumeWhitespace(parser);
  advance(parser);

  return string;
}

////////////////////////////////////////////////////////////////////////////////
///
f64 parseJsonNumber(JsonParser* parser)
{
  f64 number;
  char buffer[256];
  size_t index = 0;
  
  while (isDigit(peek(parser)) || peek(parser) == '.' || peek(parser) == '-') {
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

