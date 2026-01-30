#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "json.h"
#include "parser.h"

////////////////////////////////////////////////////////////////////////////////
///
void initJsonDocument(JsonDocument* doc)
{
  doc->root = NULL;
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

  printf("%s\n", parser.source.data);

  freeParser(&parser);
  printf("Finished %s\n", __FUNCTION__);
  return 0;
}
