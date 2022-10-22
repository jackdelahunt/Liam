outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

workspace "Liam"
    architecture "x64"
    startproject "liamc" -- this isnt working for some reason

    configurations {
        "Debug",
        "Release",
        "Dist"
        }

include "liamc"
include "liame"