# Installing
## Getting the compiler
- Get the latest release from the Github repo [here](https://github.com/jackdelahunt/Liam/releases). The version of the release you need is in the format.
```
liamc-{platform}-{architecture}
```
- Unzip this file into any directory you want as this contains everything the complier needs to run
- Add that folder location to your `PATH` so the compiler can be run from anywhere

## Using the compiler
```bash
touch main.liam
```
```bash
liamc --in=main.liam --out=out.cpp
```
```bash
clang++ out.cpp -I `liamc --runtime`
```