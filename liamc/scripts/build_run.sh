vendor/premake/premake5 gmake 
make config=debug
./bin/linux-x86_64/liamc/liamc --out="Code/out.cpp" --in="Code/main.liam" --time 
clang++ Code/out.cpp -I runtime -std=c++20 -o Code/a.out 
./Code/a.out