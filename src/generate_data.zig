const std = @import("std");
// const hs = @import("haversine.zig");
const rnd = std.Random;
const fs = std.fs;
const print = std.debug.print;

const Range = struct {
    x0: f64,
    x1: f64,
    y0: f64,
    y1: f64,
};

pub fn main() !void {
    var out_buffer: [4096]u8 = undefined;
    var cwd = std.fs.cwd();
    var file = try cwd.createFile("../data/data.json", .{});
    var out_writer = file.writer(&out_buffer);
    const out = &out_writer.interface;

    var buf: u64 = undefined;
    try std.posix.getrandom(std.mem.asBytes(&buf));
    var rng = rnd.DefaultPrng.init(buf);
    const rand = rng.random();

    var x0: f64 = undefined;
    var y0: f64 = undefined;
    var x1: f64 = undefined;
    var y1: f64 = undefined;
    
    var sum: f64 = undefined;
    var run: f64 = undefined;
    var avg: f64 = undefined;
    const N: i64 = 1_000_000;

    const n_ranges = 64;
    var ranges: [n_ranges]Range = undefined;

    for (&ranges) |*range| {
        range.x0 = (rand.float(f64) * 360.0) - 180.0;
        range.y0 = (rand.float(f64) * 360.0) - 180.0;
        range.x1 = (rand.float(f64) * 360.0) - 180.0;
        range.y1 = (rand.float(f64) * 360.0) - 180.0;
    }

    // print("Range: ({d}, {d}), ({d}, {d})\n", .{ranges[0].x0, ranges[0].y0, ranges[0].x1, ranges[0].y1});

    try out.print("{{ \"pairs\":[\n", .{});

    var range: usize = undefined;
    var x_max: f64 = undefined;
    var x_min: f64 = undefined;
    var y_max: f64 = undefined;
    var y_min: f64 = undefined;

    for (0..N) |_| {
        range = rand.intRangeLessThan(usize, 0, 64);
        x_max = @max(ranges[range].x0, ranges[range].x1);
        x_min = @min(ranges[range].x0, ranges[range].x1);
        y_max = @max(ranges[range].y0, ranges[range].y1);
        y_min = @min(ranges[range].y0, ranges[range].y1);
        x0 = x_min + rand.float(f64) * (x_max - x_min);
        x1 = x_min + rand.float(f64) * (x_max - x_min);
        y0 = y_min + rand.float(f64) * (y_max - y_min);
        y1 = y_min + rand.float(f64) * (y_max - y_min);

        run = referenceHaversine(x0, y0, x1, y1, 6372.8);
        sum += run;
        try out.print("    {{\"x0\":{d}, \"y0\":{d}, \"x1\":{d}, \"y1\":{d}}},\n", .{x0, y0, x1, y1});
    }

    x0 = (rand.float(f64) * 360.0) - 180.0;
    y0 = (rand.float(f64) * 360.0) - 180.0;
    x1 = (rand.float(f64) * 360.0) - 180.0;
    y1 = (rand.float(f64) * 360.0) - 180.0;
    run = referenceHaversine(x0, y0, x1, y1, 6372.8);
    sum += run;
    avg = sum / N;
    print("{d}\n", .{avg});
    try out.print("    {{\"x0\":{d}, \"y0\":{d}, \"x1\":{d}, \"y1\":{d}}}\n", .{x0, y0, x1, y1});
    try out.print("  ]\n}}", .{});

    try out.flush(); // Don't forget to flush!
    file.close();

}

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
