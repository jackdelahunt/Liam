-- all includes used in a table
workspace "Liam"
	architecture "x64"
	startproject "liamc" -- this isnt working for some reason

	configurations {
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
		
project "liamc"
	location "src"
	kind "ConsoleApp"
	staticruntime "On"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
		"./src/**.h",
		"./src/**.cpp",
	}

	includedirs {
		"./src",
	}

	filter "configurations:Debug"
		-- buildoptions "/DEBUG:FULL" -- FULL is for VS to out all debug symbols in the pdb
		defines {
			"DEBUG",
			symbols "on"
		}

	filter "configurations:Release"
		defines {
			"RELEASE",
			optimize "on"
		}

	-- windows specific stuffy
	filter "system:windows"
		systemversion "latest"

		defines {
			"WINDOWS",
		}

	-- windows specific stuffy
	filter "system:linux"
		defines {
			"LINUX",
		}