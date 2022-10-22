-- all includes used in a table
all_includes = {}
all_includes["GLFW"] = "libs/GLFW/include" 
all_includes["Glad"] = "libs/Glad/include" 
all_includes["ImGui"] = "libs/imgui" 
all_includes["stb_image"] = "libs/stb_image"
all_includes["glm"] = "libs/glm"

workspace "Liam"
	architecture "x64"
	startproject "liame" -- this isnt working for some reason

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

		
-- include glfw premake file
include "libs/GLFW"
include "libs/Glad"
include "libs/imgui"
include "libs/stb_image"

project "liame"
	location "src"
	kind "ConsoleApp"
	staticruntime "On"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
	    "src/**.h",
	    "src/**.cpp",
	    "src/**.c",
		}

	includedirs {
	    "src",
		"%{all_includes.GLFW}",
		"%{all_includes.Glad}",
		"%{all_includes.ImGui}",
		"%{all_includes.stb_image}",
		"%{all_includes.glm}",
		}

	links {
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
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

	-- windows specific stuffy
	filter "system:windows"
		systemversion "latest"

		defines {
			"WINDOWS",
			"GLFW_INCLUDE_NONE" -- stops glfw from including open gl alnog with glad
		}
