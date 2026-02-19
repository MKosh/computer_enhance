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
f64 radiansFromDegrees(f64 deg)
{
  return 0.01745329251994329577 * deg;
}

////////////////////////////////////////////////////////////////////////////////
///
/// x0 = lon1  y0 = lat1
/// x1 = lon2  y1 = lat2
f64 referenceHaversine(f64 x0, f64 y0, f64 x1, f64 y1)
{
  f64 radius = 6372.8;
  f64 lat1 = radiansFromDegrees(y0);
  f64 lat2 = radiansFromDegrees(y1);

  f64 d_lat = radiansFromDegrees(y1 - y0);
  f64 d_lon = radiansFromDegrees(x1 - x0);
  
  f64 a = pow(sin(d_lat/2.0), 2.0) + cos(lat1) * cos(lat2) * pow(sin(d_lon/2.0), 2.0);
  f64 c = 2.0 * asin(sqrt(a));
  return radius * c;
}

////////////////////////////////////////////////////////////////////////////////
///
int main(int argc, char* argv[]) {
  // int debug_arg = 0;
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

  if (DEBUG_) printf("File contents:\n%s\n", parser.source.data);

  if (parseJsonDoc(&parser, &doc)) {
    fprintf(stderr, "Cannot parse file.\n");
  } else {
    printf("Done parsing doc.\n");
    if (DEBUG_) printJsonDoc(&doc);
    if (DEBUG_) printf("JsonDoc contents:\n");
    if (DEBUG_) printf("\n");
  }


  JsonMember* pairs_node = getMember(doc.root, "pairs");
  if (pairs_node != NULL) {
    if (DEBUG_) printJsonElements(pairs_node->element);
    if (DEBUG_) printf("\nFound: %s\n", pairs_node->name.data);
    if (DEBUG_) printf("\n\n");

    JsonArray* values = getJsonValue(pairs_node->element);
    f64 sum = 0;
    f64 N = 0;
    f64 run = 0;
    for (JsonElement* e = values->elements; e != NULL; e = e->next) {
      f64 x0 = getJsonNumber(e, "x0");
      f64 y0 = getJsonNumber(e, "y0");
      f64 x1 = getJsonNumber(e, "x1");
      f64 y1 = getJsonNumber(e, "y1");
      
      run = referenceHaversine(x0, y0, x1, y1);
      sum += run;
      N++;
      if (DEBUG_) printf("x0 = %g, y0 = %g, x0 = %g, x1 = %g | Haversine distance = %g\n", x0, y0, x1, y1, run);
    }
    printf("Haversine distance = %g/%g = %g\n", sum, N, sum/N);
  } else {
    fprintf(stderr, "Error, can't find pairs.\n");
  }

  freeParser(&parser);
  freeJsonDoc(&doc);
  if (DEBUG_) printf("Finished %s\n", __FUNCTION__);
  return 0;
}

