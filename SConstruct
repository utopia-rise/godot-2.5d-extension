#!/usr/bin/env python
import os

# godot-cpp reads these options from the global ARGUMENTS dict; setdefault() keeps a command line value authoritative.
# build_profile.json is an allowlist of the engine classes we use, so godot-cpp only generates and compiles those.
ARGUMENTS.setdefault("build_profile", "build_profile.json")
_is_dev_build = ARGUMENTS.get("dev_build", "no") not in ("no", "false", "0")
ARGUMENTS.setdefault("lto", "none" if _is_dev_build else "full")

target_path = ARGUMENTS.pop("target_path", "test/addons/isometric_maps/libs/")
target_name = ARGUMENTS.pop("target_name", "isometric_maps")

env = SConscript("godot-cpp/SConstruct", {"api_version": "4.7"})

# Skip the godot_cpp directory when including headers: <classes/node2d.hpp> instead of <godot_cpp/classes/node2d.hpp>.
env.Append(
    CPPPATH=[
        env.Dir(os.path.join("godot-cpp", "include", "godot_cpp")),
        env.Dir(os.path.join("godot-cpp", "gen", "include", "godot_cpp")),
    ]
)
env.Append(CPPPATH=["src/"])

sources = [
    Glob("src/*.cpp"),
    Glob("src/data/*.cpp"),
    Glob("src/node/*.cpp"),
    Glob("src/resource/*.cpp"),
]

if env["target"] == "editor":
    sources += [
        Glob("src/editor/*.cpp"),
        Glob("src/editor/commands/*.cpp"),
        Glob("src/editor/commands/emitters/*.cpp"),
        Glob("src/editor/inspector/*.cpp"),
    ]

# File names are spelled out in full, so they match isometric_maps.gdextension on every platform.
env["SHLIBPREFIX"] = ""
platform = env["platform"]
library_base = "{}{}/lib{}{}".format(target_path, platform, target_name, env["suffix"])

if platform == "macos":
    framework = "{}{}/{}.{}.{}.framework".format(target_path, platform, target_name, platform, env["target"])
    library = env.SharedLibrary(
        "{}/{}.{}.{}".format(framework, target_name, platform, env["target"]),
        source=sources,
    )
elif platform == "ios":
    # The iOS exporter embeds extensions shipped as xcframeworks; the intermediate dylib is wrapped, then removed.
    dylib = library_base + env["SHLIBSUFFIX"]
    xcframework = "{}{}/lib{}.{}.{}.xcframework".format(target_path, platform, target_name, platform, env["target"])
    library = env.SharedLibrary(dylib, source=sources)
    env.AddPostAction(
        library,
        [
            Delete(xcframework),
            "xcodebuild -create-xcframework -library {} -output {}".format(dylib, xcframework),
            "/usr/libexec/PlistBuddy -c 'Add :MinimumOSVersion string {}' {}/Info.plist".format(
                env["ios_min_version"], xcframework
            ),
            Delete(dylib),
        ],
    )
else:
    library = env.SharedLibrary(library_base + env["SHLIBSUFFIX"], source=sources)

# godot-cpp's GodotCPP() calls Default() for libgodot-cpp, which replaces the default target list.
Default(library)
