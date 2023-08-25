require "cmake"

distpath = "%{wks.location}/" .. "/%{prj.name}"

workspace "Liam"
    -- architecture "x64"
    startproject "liamc" 

    configurations {
        "Debug",
        "Release",
        }

	project "liamc"
		location "src"
		kind "ConsoleApp"
		staticruntime "On"
		language "C++"
		cppdialect "C++20"
		toolset "clang"

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

		-- windows specific stuffy
		filter "system:windows"
			systemversion "latest"

			defines {
				"WINDOWS",
			}

		-- linux specific stuffy
		filter "system:linux"
			defines {
				"LINUX",
			}