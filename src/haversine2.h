#pragma once

#include <math.h>

#include "types.h"

static inline f64 radiansFromDegrees(f64 deg);
static inline f64 referenceHaversine(f64 x0, f64 y0, f64 x1, f64 y1);


static inline f64 radiansFromDegrees(f64 deg)
{
  return 0.01745329251994329577 * deg;
}

static inline f64 referenceHaversine(f64 x0, f64 y0, f64 x1, f64 y1)
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

