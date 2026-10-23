# Isometric Maps test project

A Godot 4.7 project used to test the extension. SCons builds the native libraries straight into
`addons/isometric_maps/libs/`, so a rebuild is all the project needs.

## Unit tests

`tests/unit/` holds the automated tests, run headless by `tests/run_tests.gd`:

```bash
godot --headless --path test --import
godot --headless --path test -s res://tests/run_tests.gd
```

On a fresh checkout (no `test/.godot/`), first write `res://addons/isometric_maps/isometric_maps.gdextension` into
`test/.godot/extension_list.cfg`: otherwise the import loads the extension late and Godot crashes on exit (see
`CLAUDE.md`, Known issues).

The runner exits with code 1 when a test fails or when no test ran. Each test script extends `TestCase`
(`tests/test_case.gd`); every method starting with `test_` is a test and may `await`.

| Suite | Covers |
|-------|--------|
| `test_sorting.gd` | Topological sorting through the `IsometricServer` API, in synchronous mode |
| `test_async_sorting.gd` | The default asynchronous mode, where sorts run on the server thread |
| `test_positionable.gd` | `IsometricPositionable` in the tree: projection, spaces, sorting of moving nodes |
| `test_map.gd` | `IsometricMap` instancing its grid from a `PositionableSet`, resizing, saved maps |
| `test_configuration.gd` | `IsometricConfiguration` applied to a space |

## Visual checks

Rendering is checked by eye. Run the project (or a scene below) from the editor:

- `tests/visual/sorting_showcase.tscn` (main scene): a red dynamic cube loops around a wall, a tower and under a
  bridge. It must always be hidden by what stands in front of it and drawn over what stands behind it. `S` toggles
  synchronous sorting, `D` the debug view (elements caught in a sorting cycle turn red), `Space` pauses.
- `dojo/Dojo.tscn`: a character driven by 3D physics (the `player_*` input actions) walking on maps made in the editor.
- `maps/*.tscn`: maps and nested maps made with the editor tools; open them to try the map editor plugin.

Assets used in this project are provided by [Kenney.nl](https://kenney.nl) under CC0 1.0 Universal license.
