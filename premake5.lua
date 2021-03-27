newoption {
	trigger = "emscripten",
	description = "Build with emscripten"
}

local util = dofile("scripts/util.lua")
local iplug2 = require("thirdparty/iPlug2/lua/iplug2").init()

util.disableFastUpToDateCheck({ "RetroPlug", "configure" })

iplug2.workspace "RetroPlug"
	platforms { "x64" }
	characterset "MBCS"
	cppdialect "C++17"

	defines { "NOMINMAX" }

	filter "system:macosx"
		toolset "clang"
		buildoptions { "-msse -msse2 -msse3 -mavx -mavx2" }
		xcodebuildsettings {
			["MACOSX_DEPLOYMENT_TARGET"] = "10.9"
		};

	configuration { "windows" }
		cppdialect "C++latest"
		defines { "_CRT_SECURE_NO_WARNINGS" }

	configuration {}

project "RetroPlug"
	kind "StaticLib"
	dependson { "SameBoy", "ScriptCompiler" }

	includedirs {
		"config",
		"resources",
		"src",
		"src/retroplug"
	}

	sysincludedirs {
		"thirdparty",
		"thirdparty/gainput/lib/include",
		"thirdparty/simplefilewatcher/include",
		"thirdparty/lua-5.3.5/src",
		"thirdparty/liblsdj/liblsdj/include/lsdj",
		"thirdparty/minizip",
		"thirdparty/spdlog/include",
		"thirdparty/sol",
		"thirdparty/SameBoy/Core"
	}

	files {
		"src/retroplug/microsmsg/**.h",
		"src/retroplug/**.h",
		"src/retroplug/**.c",
		"src/retroplug/**.cpp",
		"src/retroplug/**.lua"
	}

	configuration { "not emscripten" }
		prebuildcommands {
			"%{wks.location}/bin/x64/Debug/ScriptCompiler ../../src/compiler.config.lua"
		}

	filter { "system:windows", "files:src/retroplug/luawrapper/**" }
		buildoptions { "/bigobj" }

	configuration { "Debug" }
		excludes {
			"src/retroplug/luawrapper/generated/CompiledScripts_common.cpp",
			"src/retroplug/luawrapper/generated/CompiledScripts_audio.cpp",
			"src/retroplug/luawrapper/generated/CompiledScripts_ui.cpp",
		}

	configuration { "windows" }
		disablewarnings { "4996", "4250", "4018", "4267", "4068", "4150" }
		defines {
			"NOMINMAX",
			"WIN32",
			"WIN64",
			"_CRT_SECURE_NO_WARNINGS"
		}

local function retroplugProject()
	defines { "GB_INTERNAL", "GB_DISABLE_TIMEKEEPING" }

	sysincludedirs {
		"thirdparty",
		"thirdparty/gainput/lib/include",
		"thirdparty/simplefilewatcher/include",
		"thirdparty/lua-5.3.5/src",
		"thirdparty/liblsdj/liblsdj/include/lsdj",
		"thirdparty/minizip",
		"thirdparty/spdlog/include",
		"thirdparty/sol",
		"thirdparty/SameBoy/Core"
	}

	includedirs {
		".",
		"src",
		"src/retroplug",
		"src/plugin"
	}

	files {
		"src/plugin/**.h",
		"src/plugin/**.cpp",
		"resources/fonts/**",
	}

	links {
		"RetroPlug",
		"lua",
		"simplefilewatcher",
		"minizip",
		"gainput",
		"SameBoy",
		"liblsdj"
	}

	configuration { "Debug" }
		symbols "Full"

	configuration { "windows" }
		links { "xinput" }
end

dofile("scripts/configure.lua")

group "Targets"
	iplug2.project.app(retroplugProject)
	iplug2.project.vst2(retroplugProject)
	iplug2.project.vst3(retroplugProject)
	iplug2.project.wam(retroplugProject)

if _OPTIONS["emscripten"] == nil then
	group "Utils"
		project "ScriptCompiler"
			kind "ConsoleApp"
			sysincludedirs { "thirdparty", "thirdparty/lua-5.3.5/src" }
			includedirs { "src/compiler" }
			files { "src/compiler/**.h", "src/compiler/**.c", "src/compiler/**.cpp" }
			links { "lua" }
end

group "Dependencies"
	dofile("scripts/sameboy.lua")
	dofile("scripts/lua.lua")
	--dofile("scripts/minizip.lua")
	dofile("scripts/liblsdj.lua")

	if _OPTIONS["emscripten"] == nil then
		dofile("scripts/gainput.lua")
		dofile("scripts/simplefilewatcher.lua")
	end
