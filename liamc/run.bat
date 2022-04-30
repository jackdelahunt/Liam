@echo off

if not exist Debug (
  EXIT /B
)

.\Debug\liamc.exe ../main.liam P:\Liam\build\main.cpp 