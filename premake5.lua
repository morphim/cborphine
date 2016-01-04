newoption {
    trigger     = 'enable-big-endian',
    description = 'Enable big-endian byte order support (default is little-endian)'
}

newoption {
    trigger     = 'disable-int64',
    description = 'Disable 64 bits integers support (enabled by default)'
}

solution "cborphine"
    configurations { "Debug", "Release" }
    platforms { "x64", "x32" }

    if _OPTIONS['enable-big-endian'] then
        defines { "CBOR_BIGENDIAN_PLATFORM" }
    end

    if not _OPTIONS['disable-int64'] then
        defines { "CBOR_INT64_SUPPORT" }
    end

project "cborphine"
    kind "StaticLib"
    language "C"
    targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
    includedirs { "./include" }
    files { "**.h", "src/**.c" }

    filter "configurations:Debug"
        defines { "DEBUG" }

        flags { 
            "Symbols",
            "FatalWarnings",
            "FatalCompileWarnings"
        }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "cborphine-tests"
    kind "ConsoleApp"
    language "C"
    targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
    includedirs { "./include" }
    links { "cborphine" }
    files { "**.h", "test/**.c" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
