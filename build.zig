// Used the following as cross-compilation build.zig example:
// https://git.sr.ht/~jamii/focus/tree/master/build.zig

const builtin = @import("builtin");
const std = @import("std");

pub fn build(b: *std.build.Builder) !void {
    // var target = b.standardTargetOptions(.{});

    const windows = b.addStaticLibrary("mqtt-c", null);
    try includeCommon(windows);
    windows.setTarget(std.zig.CrossTarget{
        .cpu_arch = .x86_64,
        .os_tag = .windows,
    });
    windows.install();

    const m1_mac = b.addStaticLibrary("mqtt-c-m1", null);
    try includeCommon(m1_mac);
    m1_mac.setTarget(std.zig.CrossTarget{
        .cpu_arch = .aarch64,
        .os_tag = .macos,
    });
    m1_mac.install();

    const x64_mac = b.addStaticLibrary("mqtt-c-x64", null);
    try includeCommon(x64_mac);
    x64_mac.setTarget(std.zig.CrossTarget{
        .cpu_arch = .x86_64,
        .os_tag = .macos,
    });
    x64_mac.install();

    const windows_step = b.step("windows", "Build for Windows");
    windows_step.dependOn(&windows.step);

    const m1_mac_step = b.step("m1_mac", "Build for M1 Mac");
    m1_mac_step.dependOn(&m1_mac.step);

    const x64_mac_step = b.step("x64_mac", "Build for M1 Mac");
    x64_mac_step.dependOn(&x64_mac.step);
}

fn includeCommon(lib: *std.build.LibExeObjStep) !void {
    lib.addIncludeDir("include");

    lib.addCSourceFile("src/mqtt.c", &[_][]const u8 {});
    lib.addCSourceFile("src/mqtt_pal.c", &[_][]const u8 {});

    lib.setBuildMode(.Debug); // Can be .Debug, .ReleaseSafe, .ReleaseFast, and .ReleaseSmall
    lib.linkLibC();
}
