# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

**Isometric Maps** is a GDExtension for Godot 4.7 that orders 2.5D isometric scenes by their 3D boxes and ships editor
tooling to paint maps. It was an engine module; it is now built against
[godot-cpp](https://github.com/godotengine/godot-cpp) (submodule `godot-cpp/`, tag `10.0.0-stable`, `api_version=4.7`).
The rendering pipeline, step by step, with the tests covering each step, is described in `README.md` ("How rendering
works"): read it before touching sorting.

## Layout

- `src/register_types.cpp` — entry point `isometric_maps_library_init`; registers runtime classes at the scene level and
  editor classes (plugins, commands, inspector panes) at the editor level.
- `src/isometric_server.*` — `IsometricServer` engine singleton: spaces, elements, the sorting thread (see Threading).
- `src/data/` — plain structs the server sorts (`IsometricElement`, `IsometricSpace`, `IsometricParameters`).
- `src/utils/isometric_maths.h` — projection, hexagon overlap and "is in front" tests.
- `src/containers/` — `Grid3D` (map grids) and `CommandQueue` (server thread commands).
- `src/node/` — `IsometricPositionable` (a `Node2D` with a 3D box) and `IsometricMap` (a container instancing a grid).
- `src/resource/` — `PositionableSet` (id → scene) and `IsometricConfiguration`.
- `src/editor/` — editor-only code, compiled in `editor` builds only: `IsometricEditorPlugin` (map editing),
  `IsometricTileEditorPlugin`, `PositionableSetEditorPlugin`, the undo/redo `commands/`, the input `commands/emitters/`
  and the `inspector/` panes.
- `test/` — Godot project the libraries are built into (`test/addons/isometric_maps/`); unit tests in `test/tests/unit/`,
  visual scenes in `test/tests/visual/`, `dojo/` and `maps/`. See `test/README.md`.
- `docs/` — user documentation (mkdocs, published on readthedocs).

## Build

```bash
scons target=editor            # editor library, with the editor tooling
scons target=template_debug    # game library, every check enabled
scons target=template_release  # game library, optimised, internal checks compiled out
```

Libraries land in `test/addons/isometric_maps/libs/<platform>/` (override with `target_path=`). Platforms use
godot-cpp naming: `windows`, `linux`, `macos` (`arch=universal`, framework), `android` (`arch=arm64` / `arch=x86_64`,
needs `ANDROID_HOME` and NDK 28.1.13356709), `ios` (`arch=arm64`, wrapped into an xcframework by SConstruct).
`isometric_maps.gdextension` lists every file name; keep it in sync with SConstruct.

- **Build profile** — `build_profile.json` is an allowlist of the engine classes godot-cpp binds (parents are added
  automatically). A missing class shows up as a missing header, or as a godot-cpp method that does not exist because one
  of its argument types is excluded (e.g. `add_control_to_bottom_panel` needs `Shortcut`). Add the class there; do not
  drop the profile.
- **Targets** — `TOOLS_ENABLED` (editor only) guards editor code and editor-only properties; `DEBUG_ENABLED` (editor and
  template_debug) guards debug features. Build all three targets after touching includes or `#ifdef` code.
- **Release checks** — `ISO_DEBUG_FAIL_COND_MSG` / `ISO_DEBUG_FAIL_COND_V_MSG` (`src/logging.h`) guard API misuse and
  internal invariants that a correct project never triggers (invalid RIDs, moving a static element). They compile to
  nothing in `template_release`, condition included. Use them for such checks on hot paths; keep `ERR_FAIL_*` for errors
  a valid project can hit (bad user input, missing files).

## Threading

`IsometricServer` mirrors the engine servers: sorting runs on a dedicated high-priority `WorkerThreadPool` task fed by
`containers::CommandQueue`.

- Element and space fields are owned by one thread each (comments in `src/data/`). The main thread only allocates RIDs
  and pushes commands; the server thread owns the sorting graph. Never read a server-thread field from the main thread.
- Sort results are handed back under `results_mutex`; the main thread applies them to the `RenderingServer` on
  `frame_pre_draw`, then requests the next sort. Asynchronous mode (default) shows results one frame later;
  `set_synchronous_mode(true)` waits for the sort (`push_and_sync`).
- Lifetime: the thread starts on the first main loop iteration (`hook_into_main_loop`, deferred because server
  singletons are registered after the extension's scene level) and stops when the `SceneTree` root exits, because the
  engine waits for idle `WorkerThreadPool` threads before deinitializing extensions. Outside that window commands run
  inline on the caller's thread.

## Testing

```bash
godot --headless --path test --import
godot --headless --path test -s res://tests/run_tests.gd
```

`test/tests/run_tests.gd` runs every `test_*` method of the `TestCase` scripts in `test/tests/unit/` and exits with 1 on
failure. Use synchronous mode for deterministic sorting assertions; `test_async_sorting.gd` covers the threaded path.
Rendering is checked by eye in `test/tests/visual/sorting_showcase.tscn`. To render frames headlessly for a look:
`godot --path test --write-movie out.png --fixed-fps 30 --quit-after 120 res://tests/visual/sorting_showcase.tscn`.

## CI/CD

`.github/workflows/build.yml` (reusable) builds every platform × target and runs the unit tests on Linux; `ci.yml` calls
it for pull requests and `master`; `release.yml` calls it for `x.y.z` tags, zips `addons/isometric_maps/` and creates
the GitHub release with `gh`. The Godot version is `GODOT_VERSION` in `build.yml`.

## Known issues

- **Crash on exit after a fresh import** — When a project without `.godot/` is imported, the editor loads the
  extension late (during the first scan) and Godot 4.7.2 crashes on exit, on Windows and Linux. Bisecting showed it
  needs at least one registered class (a bare `Resource` is enough) and happens in engine cleanup code; loading the
  extension at startup avoids it. CI writes `.godot/extension_list.cfg` before importing. Not yet investigated further.

## Engineering Principles

### General

These apply to every language in the repository.

- **Code is documentation** — Names and structure carry the meaning, so most code needs no comment at all. Inside a function body, comment only what a reader could not work out quickly from the code itself: a non-obvious invariant, a cross-thread protocol, an engine quirk being worked around. Never restate what the next line does. A short summary on a class is welcome; properties and methods get no comment unless their behavior is genuinely complex.
- **Avoid needless helpers** — Keep trivial single-use code inline. Extract a helper only when it has multiple call sites or its body is substantial enough to make the caller clearer.
- **KISS** — Prefer the simplest design that clearly solves the current problem.
- **YAGNI** — Do not add behavior or abstractions for requirements that do not exist yet.
- **DRY** — Keep each piece of knowledge and behavior in one authoritative place.

### C++

- **Lambdas** — Not banned, but use one only when strictly necessary (a callback API) or when the code would be markedly more complex without it. Prefer a plain loop, a file-local function, or a data table over a local lambda that merely groups a few lines.
- **No anonymous namespaces** — File-local C++ helpers are `static` free functions in the `.cpp` (or private members when they belong to the class); nothing lives in `namespace { }`.
- **Namespaces** — Code lives in the project namespaces (`data`, `utils`, `containers`, `node`, `resource`, `editor` and its sub-namespaces); headers open them with `using namespace godot;` inside the namespace block, and `.cpp` files start with `using namespace godot;` followed by the using-directive of their own namespace, so no `godot::` prefixes appear. Headers outside a project namespace (`isometric_server.h`, `isometric_string_names.h`, `data/`) keep `godot::` qualifiers. Macro bodies stay fully qualified because they expand at the use site.
- **Includes** — Every file includes what it uses directly and nothing it does not; do not rely on a header reaching you through another include. When a header only includes something for its `.cpp`, move the include to the `.cpp`. godot-cpp headers are included without the `godot_cpp/` prefix (`<classes/node2d.hpp>`). Build `editor`, `template_debug` and `template_release` after touching includes: the three targets compile different `#ifdef` branches.
- **clang-format** — Run `clang-format -i` (version 20 or newer, the repo `.clang-format` uses recent options) over every `.cpp` and `.h` under `src/`.
- **Initialization** — Use `=` for scalar values and expressions already of the declared type (`int count = 0;`, `String name = get_name();`). Use direct `()` initialization whenever an object is constructed from something of another type, whether its constructor is explicit or implicit: `Type value(expr);`, never `Type value = Type(expr);`. Use `()` for constructor calls with several arguments (`Vector3 pos(1, 2, 3);`), `{}` only for arrays, aggregates, `std::initializer_list` parameters, and returned/passed temporary aggregates (`return {ptr, size};`). Member initializer lists use `()`, default member initializers use `=`. Never `Type value{expr};`, never `Type value();`, never `Foo f{};` on a class type (write `Foo f;`); zero PODs with `= {}`.

### GDExtension

- **Registration** — Every `GDCLASS` must be registered in `register_types.cpp` (editor classes with `GDREGISTER_INTERNAL_CLASS` at the editor level) and declare its own `static void _bind_methods()`, even empty.
- **Engine virtuals** — godot-cpp binds any method named like an engine virtual (`_ready`, `_enter_tree`, `_physics_process`…) as an override. Nodes here react through `_notification` with differently named private methods (`on_enter_tree`) so nothing runs twice and physics processing is not switched on implicitly.
- **Callables** — Prefer `callable_mp(this, &Class::method)` over string-named `Callable`s; bind a method only when the engine calls it by name (undo/redo `add_do_method`, `call_group`).
- **Engine singletons** — Server singletons (`RenderingServer`…) are registered after the extension's scene level: reach them from deferred calls or later, never from `initialize_isometric_maps_module`.
