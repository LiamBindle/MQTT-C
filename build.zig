const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const lib = b.addStaticLibrary("MQTT-C", null);

    lib.addIncludeDir("include");

    lib.addCSourceFile("src/mqtt.c", &[_][]const u8 {});
    lib.addCSourceFile("src/mqtt_pal.c", &[_][]const u8 {});

    lib.linkLibC();

    lib.setBuildMode(mode);
    lib.install();
}
