rm -rf liamc/release/macos
mkdir liamc/release/macos
cp -r liamc/stdlib liamc/release/macos
(cd liamc && ./vendor/premake5-mac gmake && make config=release)
cp liamc/bin/liamc/liamc liamc/release/macos