#include "utils.h"

#include <cmath>

////////////////////////////////////////////////////////////////////////////////
///
static f64 Square(f64 a) {
  f64 result = a*a;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
///
static f64 RadiansFromDegrees(f64 degrees) {
  f64 result = 0.01745329251994329577f * degrees;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Reference implementation of the Haversine distance
static f64 ReferenceHaversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 earth_radius = 6372.8) {
  f64 lat1 = y0;
  f64 lat2 = y1;
  f64 lon1 = x0;
  f64 lon2 = x1;

  f64 d_lat= RadiansFromDegrees(lat2 - lat1);
  f64 d_lon= RadiansFromDegrees(lon2 - lon1);
  lat1 = RadiansFromDegrees(lat1);
  lat2 = RadiansFromDegrees(lat2);

  f64 a = Square(sin(d_lat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(d_lon/2));
  f64 c = 2.0*asin(sqrt(a));

  f64 result = earth_radius * c;

  return result;
}

////////////////////////////////////////////////////////////////////////////////
///
int main() {
  
  return 0;
} 
