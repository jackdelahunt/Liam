cls
mkdir build
pushd build
cmake -B build/ ../
popd

msbuild liamc.sln