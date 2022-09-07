from os import listdir
from os.path import isfile, join
import sys
import subprocess
import os

source_dir = "liam/"
runtime_path = os.path.abspath("../runtime")
compiler_path = "../cmake-build-debug/liamc"
source_files = [source_dir + f for f in listdir(source_dir) if isfile(join(source_dir, f))]

for i, file_path in enumerate(source_files):

    lines = []
    source = open(file_path).readlines()
    for line in source:
        if line.startswith("#"):
            lines.append(line.strip("\n#"))
        else:
            break

    compile_output = subprocess.run([
        compiler_path,
        "--cpp",
        "--in", file_path,
        "--out", "out.exe",
        "--include", "../runtime",
        "--stdlib", "../stdlib"
    ], capture_output=True)

    if compile_output.stderr != b'':
        print("COMPILE ERROR :: ", file_path,  compile_output.stderr.decode("UTF-8"))
        exit(1)

    if compile_output.returncode != 0:
        print(f"COMPILE ERROR :: {file_path} :: return code was {compile_output.returncode}")
        exit(1)

    running_output = subprocess.run(["./out.exe"], capture_output=True)
    if running_output.stderr != b'':
        print("RUNTIME ERROR :: ", file_path, compile_output.stderr.decode("UTF-8"))
        exit(1)

    output_lines = running_output.stdout.decode("UTF-8").splitlines()
    if lines != output_lines:
        print(f"MATCH ERROR :: non matching output {file_path} expected {lines} got {output_lines}")
    else:
        print(f"({i + 1},{len(source_files)}) TEST PASSED [:")

    if os.path.exists("out.exe"):
        os.remove("out.exe")



