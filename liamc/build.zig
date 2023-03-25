const std = @import("std");

pub fn build(b: *std.build.Builder) void {

    const exe = b.addExecutable("liamc", null);
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    // liam
    exe.addCSourceFile("src/main.cpp", &[_][]const u8{"-std=c++20", "-g", "-O0"});
    exe.addCSourceFile("src/args.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/module.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/compiler.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/cpp_backend.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/errors.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/expression.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/file.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/lexer.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/liam.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/parser.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/statement.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/type_checker.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/type_info.cpp", &[_][]const u8{});
    exe.addCSourceFile("src/utils.cpp", &[_][]const u8{});

    // lib fmt  
    exe.addCSourceFile("src/fmt/format.cc", &[_][]const u8{});

    // build
    exe.linkLibCpp();    
    exe.install();

    exe.setTarget(target);
    exe.setBuildMode(mode);
}
