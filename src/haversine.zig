const std = @import("std");

pub fn square(a: f64) f64 {
    return std.math.pow(f64, a, 2);
}

pub fn radiansFromDegrees(deg: f64) f64 {
  const result = 0.01745329251994329577 * deg;
  return result;
}

pub fn referenceHaversine(x0: f64, y0: f64, x1: f64, y1: f64, radius: f64) f64 {
    const asin = std.math.asin;
    const sqrt = std.math.sqrt;
    var lat1 = y0;
    var lat2 = y1;
    const lon1 = x0;
    const lon2 = x1;
    const d_lat = radiansFromDegrees(lat2 - lat1);
    const d_lon = radiansFromDegrees(lon2 - lon1);
    lat1 = radiansFromDegrees(lat1);
    lat2 = radiansFromDegrees(lat2);
    const a = square(@sin(d_lat/2.0))  + @cos(lat1) * @cos(lat2) * square(@sin(d_lon/2));
    const c = 2.0 * asin(sqrt(a));
    const result = radius * c;
    return result;
}

