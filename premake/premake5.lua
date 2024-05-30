-- Note: loaded scripts will change the current directory due to the code below:
-- https://github.com/premake/premake-core/blob/5b6289407d24fd48d5d8066119295a9c89947c4c/src/host/lua_auxlib.c#L153
-- for this reason we have to change directory manually back at the beginning on each script to avoid major pain!
os.chdir(_MAIN_SCRIPT_DIR)

require("premake/utils")

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

function copytodir(subdir)
	if _OPTIONS["copy-to"] then
		postbuildcommands {"copy /y \"$(TargetPath)\" \"" .. path.join(_OPTIONS["copy-to"], subdir) .. "\""}
	end
end

newoption {
	trigger = "ci-build",
	description = "Enable CI builds of the client."
}

newaction {
	trigger = "generate-buildinfo",
	description = "Sets up build information file like version.h.",
	onWorkspace = function(wks)
		-- get old version if any
		local oldVersion = "(none)"
		local oldVersionFile = io.open(wks.location .. "/version.txt", "r")
		if oldVersionFile ~= nil then
			oldVersion = assert(oldVersionFile:read('*line')):gsub("%s+", "")
			oldVersionFile:close()
		end

		-- get current version
		local bmeVersionFile = assert(io.open("bme_version.txt", "r"))
		local bmeVersion = assert(bmeVersionFile:read('*line')):gsub("%s+", "")
		bmeVersionFile:close()

		if not _OPTIONS["ci-build"] then
			bmeVersion = bmeVersion .. "-DEV"
		end
		
		local proc = assert(io.popen("git describe --always" .. (_OPTIONS["ci-build"] and "" or " --dirty --broken"), "r"))
		local gitDescribeOutput = assert(proc:read('*all')):gsub("%s+", "")
		proc:close()
		
		if _OPTIONS["ci-build"] ~= "release" then
			bmeVersion = bmeVersion .. "+" .. gitDescribeOutput
		end

		if oldVersion ~= bmeVersion then
			print("Update " .. oldVersion .. " -> " .. bmeVersion)

			-- write to version.txt
			local finalVersionFile = assert(io.open(wks.location .. "/version.txt", "w"))
			finalVersionFile:write(bmeVersion)
			finalVersionFile:close()

			-- write version header
			local versionHeader = assert(io.open(wks.location .. "/src/version.h", "w"))
			versionHeader:write("/*\n")
			versionHeader:write(" * Automatically generated by premake5.\n")
			versionHeader:write(" * Do not touch!\n")
			versionHeader:write(" */\n")
			versionHeader:write("\n")
			versionHeader:write("#define GIT_DESCRIBE " .. cstrquote(gitDescribeOutput) .. "\n")
			versionHeader:write("#define BME_VERSION " .. cstrquote(bmeVersion) .. "\n")
			versionHeader:write("\n")
			versionHeader:close()
		end
	end
}

dependencies = {
	basePath = path.join(_MAIN_SCRIPT_DIR, "thirdparty"),
	depsScriptsPath = path.join(_MAIN_SCRIPT_DIR, "premake/deps"),
}

deps = {} -- actual projects are here

function dependencies.load()
	local dir = path.join(dependencies.depsScriptsPath, "*.lua")
	local deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		local dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for name, proj in pairs(deps) do
		proj.import()
	end
end

function dependencies.projects()
	for name, proj in pairs(deps) do
		proj.project()
	end
end

dependencies.load()

workspace "bme"
startproject "bmedll"
location "./build"
objdir "%{wks.location}/obj"
targetdir "%{wks.location}/bin/%{cfg.platform}-%{cfg.buildcfg}"
libdirs {"%{wks.location}/bin/%{cfg.platform}-%{cfg.buildcfg}"}

solutionitems {
	"./README.md",
	"./bme_version.txt",
	"./.gitignore",
	"./.github/workflows/build.yaml",
	"./.github/dependabot.yaml",
	["Premake"] = {
		"./premake/*.lua",
		["Deps"] = {
			"./premake/deps/**.lua"
		}
	},
}

configurations {"Release", "Staging", "Debug"}

language "C++"
cppdialect "C++20"
toolset "msc"

architecture "x86_64"
platforms "x64"

systemversion "latest"
symbols "On"
staticruntime "Off"
editandcontinue "Off"
--warnings "Extra"
characterset "ASCII"

flags {"MultiProcessorCompile", "No64BitChecks"}

defines {"_WINDOWS", "WIN32", "NOMINMAX"}

disablewarnings {"26812"} -- prefer `enum class` over `enum`
disablewarnings {"6255"} -- _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead

buildoptions {"/utf-8"}
linkoptions {}

filter "configurations:Release or configurations:Staging"
	optimize "Speed"
	buildoptions {
		--"/GL", -- Whole Program Optimization (requires LTCG)
		--"/guard:cf", -- Control Flow Guard (commented out because it was failing on us calling function pointer from GetProcAddress)
		--"/guard:ehcont", -- Enable EH Continuation Metadata (commented out because requires all .lib's to be recompiled)
	}
	linkoptions {
		--"/LTCG", -- Link Time Code Generation (required for whole program optimization)
		--"/guard:cf", -- Control Flow Guard
		--"/guard:ehcont", -- Enable EH Continuation Metadata
	}
	defines {"NDEBUG"}
	flags {--[["FatalCompileWarnings",]] "LinkTimeOptimization"}
filter {}

filter "configurations:Staging"
	defines {"STAGING"}
filter {}

filter "configurations:Debug"
	optimize "Debug"
	defines {"DEBUG", "_DEBUG"}
filter {}

if _OPTIONS["ci-build"] then
	defines {"CI_BUILD"}
end

files {"shared.manifest"}
includedirs {"./thirdparty"}
libdirs {"./thirdparty"}

require("premake/bmedll")
require("premake/loader_launcher_exe")
require("premake/loader_launcher_proxy")

group "Dependencies"
dependencies.projects()
