## Building on Linux

### Requirements
- Clang++
- Make
- Bash

``` bash
cd liamc
```

``` bash
bash scripts/setup_dev.sh
```

``` bash
bash scripts/build_run.sh
```

## Building on Windows
### Requirements
- Visual Studio 2022

```powershell
cd liamc
```
```powershell
.\vendor\premake\premake5.exe vs2022
```
Creates Visual Studio project to use as normal
