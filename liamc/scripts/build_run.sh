vendor/premake/premake5 gmake 
make config=debug
./bin/linux-x86_64/liamc/liamc --out="Code/out.cpp" --in="Code/main.liam" --time 
g++ -I runtime Code/out.cpp -o Code/a.out 
./Code/a.out
