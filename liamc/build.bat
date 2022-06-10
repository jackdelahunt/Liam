@echo off

.\vendor\premake\premake5.exe vs2022

if /I %1 == build-debug goto :build_debug
if /I %1 == build-release goto :build_release
if /I %1 == run-debug goto :run_debug
if /I %1 == run-release goto :run_release

goto :eof

:build_debug
MSBuild.exe /property:Configuration=Debug
goto :eof

:build_release 
MSBuild.exe /property:Configuration=Release
goto :eof

:run_debug
MSBuild.exe /property:Configuration=Debug
.\bin\Debug-windows-x86_64\liamc\liamc.exe rust code/main.liam
goto :eof

:run_release 
MSBuild.exe /property:Configuration=Release
.\bin\Release-windows-x86_64\liamc\liamc.exe rust code/main.liam
goto :eof

:eof