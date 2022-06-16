from os import listdir
from os.path import isfile, join
import sys
import subprocess
import os

source_dir = "liam/"
out_path = "bin/"
runtime_path = os.path.abspath("../runtime")

args = sys.argv
if len(args) < 3:
    print("Usage: runner.py <compiler-path> <cpp-compiler-path>")
    exit(1)


compiler_path = args[1]
cpp_compiler_path = args[2]
source_files = [source_dir + f for f in listdir(source_dir) if isfile(join(source_dir, f))]

os.makedirs(out_path, exist_ok=True)

for i, source in enumerate(source_files):
    print("========= " + source + " =========")
    subprocess.run([compiler_path, "--in", source, "--out", out_path + "out.cpp"])
    subprocess.run([cpp_compiler_path, "-I", runtime_path, out_path + "out.cpp", "-o", out_path + "out.exe"])
    subprocess.run(["./" + out_path + "out.exe"])
