./bin/linux-x86_64/liamc/liamc --out="selfhost/out.cpp" --in="selfhost/main.liam" 

g++ -I runtime selfhost/out.cpp -o selfhost/a.out

./selfhost/a.out

rm selfhost/a.out
