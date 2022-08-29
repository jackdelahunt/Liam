require "cmake"

workspace "Liam"
	architecture "x64"
	startproject "liamc" -- this isnt working for some reason
	cppdialect "C++20"

	configurations {
		"Debug",
		"Test",
		"Release",
		"Dist",
	}

distpath = "%{wks.location}/" .. "/%{prj.name}-%{cfg.system}-%{cfg.architecture}"

project "liamc"
	location "src"
	kind "ConsoleApp"
	staticruntime "On"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/%{prj.name}")
	objdir ("obj/%{prj.name}")

	files {
		"./src/**.h",
		"./src/**.cpp",
		"./src/**.cc",
	}

	includedirs {
		"./src",
	}

	filter "configurations:Test"
        defines {
            "TEST",
            symbols "on"
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

	filter "configurations:Dist"
		defines {
			"DIST",
			optimize "on"
		}

		postbuildcommands {
			"{MKDIR} %{distpath}",
			"{COPYFILE} %{cfg.buildtarget.abspath} %{distpath}/",
			"{COPYDIR} %{wks.location}/runtime %{distpath}/runtime",
			"{COPYDIR} %{wks.location}/stdlib %{distpath}/stdlib",
	  	}

	-- windows specific stuffy
	filter "system:windows"
		systemversion "latest"

		defines {
			"WINDOWS",
		}

	-- linux specific stuffy
	filter "system:linux"
		toolset "clang"
		defines {
			"LINUX",
		}

