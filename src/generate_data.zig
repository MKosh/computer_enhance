const std = @import("std");
// const hs = @import("haversine.zig");
const rnd = std.Random;
const fs = std.fs;
const print = std.debug.print;

const radius_km = 6372.8;
const Range = struct {
    x0: f64,
    x1: f64,
    y0: f64,
    y1: f64,
};

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());

    var N: usize = undefined;
    var file_name: []const u8 = undefined;

    if (args.len == 3) {
        N = try std.fmt.parseInt(usize, args[1], 10);
        file_name = args[2];
    } else {
        print("Usage: zig run generate_data.zig -- num_pairs outfile_name\n", .{});
        return;
    }

    const obuff = try init.gpa.alloc(u8, 10 * 1024 * 1024);
    var buf: [1024]u8 = undefined;
    var cwd = std.Io.Dir.cwd();
    var file = try cwd.createFile(init.io, file_name, .{});
    var out_writer = file.writer(init.io, obuff);
    const out = &out_writer.interface;
    const parts = [_][]const u8{ std.fs.path.dirname(file_name).?, "/", std.fs.path.stem(file_name), ".txt" };
    const sum_file_name = try std.mem.concat(init.arena.allocator(), u8, &parts);
    var sum_file = try cwd.createFile(init.io, sum_file_name, .{});
    var sum_writer = sum_file.writer(init.io, &buf);

    // RNG not randomly seeded!
    var prng = std.Random.DefaultPrng.init(0);

    var x0: f64 = undefined;
    var y0: f64 = undefined;
    var x1: f64 = undefined;
    var y1: f64 = undefined;

    var sum: f64 = undefined;
    var run: f64 = undefined;
    var avg: f64 = undefined;

    const n_ranges = 64;
    var ranges: [n_ranges]Range = undefined;

    for (&ranges) |*range| {
        range.x0 = (prng.random().float(f64) * 360.0) - 180.0;
        range.y0 = (prng.random().float(f64) * 360.0) - 180.0;
        range.x1 = (prng.random().float(f64) * 360.0) - 180.0;
        range.y1 = (prng.random().float(f64) * 360.0) - 180.0;
    }

    try out.print("{{ \"pairs\":[\n", .{});

    var range: usize = undefined;
    var x_max: f64 = undefined;
    var x_min: f64 = undefined;
    var y_max: f64 = undefined;
    var y_min: f64 = undefined;

    for (0..N) |_| {
        range = prng.random().intRangeLessThan(usize, 0, 64);
        x_max = @max(ranges[range].x0, ranges[range].x1);
        x_min = @min(ranges[range].x0, ranges[range].x1);
        y_max = @max(ranges[range].y0, ranges[range].y1);
        y_min = @min(ranges[range].y0, ranges[range].y1);
        x0 = x_min + prng.random().float(f64) * (x_max - x_min);
        x1 = x_min + prng.random().float(f64) * (x_max - x_min);
        y0 = y_min + prng.random().float(f64) * (y_max - y_min);
        y1 = y_min + prng.random().float(f64) * (y_max - y_min);

        run = referenceHaversine(x0, y0, x1, y1, radius_km);
        sum += run;
        try out.print("    {{\"x0\":{d}, \"y0\":{d}, \"x1\":{d}, \"y1\":{d}}},\n", .{ x0, y0, x1, y1 });
    }

    x0 = (prng.random().float(f64) * 360.0) - 180.0;
    y0 = (prng.random().float(f64) * 360.0) - 180.0;
    x1 = (prng.random().float(f64) * 360.0) - 180.0;
    y1 = (prng.random().float(f64) * 360.0) - 180.0;
    run = referenceHaversine(x0, y0, x1, y1, 6372.8);
    sum += run;
    avg = sum / @as(f64, @floatFromInt(N));

    print("{d}\n", .{avg});
    try sum_writer.interface.print("{d}\n", .{avg});
    try out.print("    {{\"x0\":{d}, \"y0\":{d}, \"x1\":{d}, \"y1\":{d}}}\n", .{ x0, y0, x1, y1 });
    try out.print("  ]\n}}", .{});

    try sum_writer.interface.flush();
    sum_file.close(init.io);

    try out.flush(); // Don't forget to flush!
    file.close(init.io);

    init.gpa.free(obuff);
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
    const a = square(@sin(d_lat / 2.0)) + @cos(lat1) * @cos(lat2) * square(@sin(d_lon / 2));
    const c = 2.0 * asin(sqrt(a));
    const result = radius * c;
    return result;
}
