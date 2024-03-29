from os import listdir
from os.path import isdir, join, isfile, basename
import sys
import subprocess
import os

source_dir = os.path.dirname(__file__) + "/liam/"
compiler_path = os.path.dirname(__file__) + "/../build/debug/liamc"
stdlib_path = os.path.dirname(__file__) + "/../stdlib"
core_path = os.path.dirname(__file__) + "/../core"

source_files = []

for f in listdir(source_dir):
    if isfile(join(source_dir, f)) and f.endswith(".liam"):
        source_files.append(join(source_dir, f))


failed_tests_count = 0
tests_count = len(source_files)

for i, file_path in enumerate(source_files):
    file_name_for_output = os.path.basename(file_path)

    lines = []
    source = open(file_path).readlines()
    for line in source:
        if line.startswith("//"):
            lines.append(line.strip("\n/"))
        else:
            break  

    compile_output = subprocess.run([
        compiler_path,
        file_path,
    ], capture_output=True)

    if compile_output.stderr != b'' or compile_output.returncode != 0:
        print(f"({i + 1},{len(source_files)}) TEST FAILED ]: {file_name_for_output} liamc compile error")
        failed_tests_count += 1

        print(compile_output.stderr.decode("UTF-8"))

        continue

    clang_output = subprocess.run([
        "clang++",
        "-I", f"{stdlib_path}/include",
        "-I", core_path,
        "-std=c++17",
        "-o", "out.exe",
        "out.cpp"
    ], capture_output=True)

    if clang_output.returncode != 0:
        print(f"({i + 1},{len(source_files)}) TEST FAILED ]: {file_name_for_output} clang++ compile error")
        failed_tests_count += 1
        continue

    running_output = subprocess.run(["./out.exe"], capture_output=True)

    if running_output.stderr != b'' or running_output.returncode != 0:
        print(f"({i + 1},{len(source_files)}) TEST FAILED ]: runtime error")
        failed_tests_count += 1
        continue

    output_lines = running_output.stdout.decode("UTF-8").splitlines()
    if lines != output_lines:
        print(f"({i + 1},{len(source_files)}) TEST FAILED ]: {file_name_for_output  } expected {lines} got {output_lines}")
        failed_tests_count += 1
    else:
        print(f"({i + 1},{len(source_files)}) TEST PASSED [:")

    if os.path.exists("out.exe"):
        os.remove("out.exe")

print(f"\nPassing {tests_count - failed_tests_count}/{tests_count} tests")
print(f"Failing {failed_tests_count}/{tests_count} tests")

