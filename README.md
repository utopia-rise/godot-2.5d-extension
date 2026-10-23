# Isometric Maps

[![GitHub](https://img.shields.io/github/license/utopia-rise/godot-2.5D-isometric-map-editor?style=flat-square)](LICENSE)

A Godot 4.7 GDExtension to build complex 2.5D isometric maps.

Classic tiled maps cannot express isometric maps with real 3D logic. Here every basic node is an
`IsometricPositionable`: a 2D node with a 3D position and a 3D size (an `AABB`). The extension orders them by their 3D
boxes, so tiles, walls, bridges and moving characters are drawn in the right order. A positionable can follow 3D physics
through a `CollisionObject3D` child.

Positionables are ordinary nodes and scenes: a tile or a whole map can be reused in many maps, and fixing it once fixes
every map that uses it.

**Disclaimer:** This project is still work in progress and not ready for production.

User documentation: [isometricmapmodule.readthedocs.io](https://isometricmapmodule.readthedocs.io/en/latest/).

## Installation

Download `isometric_maps-x.y.z.zip` from the [releases](https://github.com/utopia-rise/godot-2.5d-extension/releases) and
unzip it at the root of your Godot 4.7 project: it adds `addons/isometric_maps/` with the libraries for Windows, Linux,
macOS, Android and iOS.

## How rendering works

The whole pipeline turns 3D boxes into one integer per node, the canvas z-index. Each step below names the tests that
pin it down (`test/tests/unit/`) and the visual check that shows it (`test/tests/visual/sorting_showcase.tscn`).

### 1. Every node is a 3D box projected on screen

An `IsometricPositionable` has a `local_position_3d` and a `size`; its global 3D position adds up the positions of its
positionable parents. The node's 2D position is the projection of its local 3D position:

```
screen.x = (x - y) * diamond_width / 2
screen.y = (x + y) * diamond_height / 2 - z * z_length
```

`diamond_width`, `diamond_height` and `z_length` come from the space configuration (`IsometricConfiguration`: a tile
width and a camera angle; 256 px at 30° by default). Moving a node in 3D is therefore just moving it in 2D: the sprite
lands where its box is.

Checks: `test_positionable.gd` (`test_positions_are_projected_on_screen`, `test_moving_updates_screen_position`),
`test_configuration.gd`.

### 2. Boxes are grouped in spaces

Only boxes of the same space are ordered against each other. The topmost positionable of a hierarchy creates a space,
and every positionable below it joins that space. An `IsometricMap` is a container: it owns the space of its cells but
is not sorted itself.

Checks: `test_children_share_the_space_of_their_parent`, `test_siblings_under_a_plain_node_get_their_own_space`,
`test_spaces_are_sorted_independently`.

### 3. Each box is an element of the IsometricServer

Entering the tree, a positionable registers an *element* in the `IsometricServer`: its box, its canvas item, a `depth`
(how many z levels it takes) and whether it is static or dynamic. Static elements are expected never to move (map
tiles); dynamic elements can (characters, moving platforms). Leaving the tree frees the element.

Checks: `test_leaving_the_tree_releases_the_element`, `test_freed_element_no_longer_constrains_others`,
`test_switching_to_dynamic_keeps_the_element_sorted`.

### 4. Two boxes are only ordered if they can hide each other

On screen, a box looks like a hexagon. Two boxes only need an order when their hexagons overlap, which is tested on the
three isometric directions (x - z, y - z and x - y), with the configuration's topological margin as tolerance.
Far apart boxes stay unconstrained, which keeps the graph small.

Checks: `test_distant_elements_do_not_constrain_each_other`.

### 5. Overlapping boxes are compared: which one is in front

For two overlapping boxes, the first axis on which they are separated decides: the box further along x, then y, then
z (higher) is in front. Boxes interpenetrating on all three axes fall back on the distance of their centers along the
camera direction.

Checks: `test_element_in_front_on_x_is_drawn_over`, `test_element_in_front_on_y_is_drawn_over`,
`test_element_above_is_drawn_over`, `test_attach_order_does_not_change_the_result`.

### 6. The comparisons form a "behind" graph

Each element keeps the list of elements behind it. Static/static edges are computed once, when a static element joins
the space. Edges involving dynamic elements are rebuilt on every sort, since those elements move.

Checks: `test_dynamic_element_is_sorted_against_statics`, `test_dynamic_elements_are_sorted_together`,
`test_resizing_dynamic_element_updates_ordering`, `test_detached_element_no_longer_constrains_others`.

### 7. A depth-first walk turns the graph into z-indices

The z-order of an element is the largest `z-order + depth` among the elements behind it (0 when nothing is behind).
The walk keeps a stack: meeting an element already on the stack means a cycle (A behind B behind A), which no order can
satisfy; the elements of the cycle are flagged invalid and drawn red in debug mode.

Checks: `test_chain_accumulates_z_order`, `test_depth_of_element_behind_offsets_z_order`,
`test_large_grid_is_ordered_consistently`. Visual: debug mode (`D`) in the showcase.

### 8. z-indices reach the RenderingServer, once per frame, from another thread

Sorting runs on the server's own thread, like the engine's rendering and physics servers: a high-priority
`WorkerThreadPool` task consuming a command queue. The main thread never touches the graph; it only pushes commands
(create, move, resize, free).

Right before each frame is drawn (`RenderingServer.frame_pre_draw`), the main thread:

1. applies the z-indices of the last finished sort to the canvas items (`canvas_item_set_z_index`), and
2. requests the next sort, which runs while the game computes the following frame.

So by default a move shows its new order one frame later. `IsometricServer.set_synchronous_mode(true)` sorts and
applies within the same frame instead, at the cost of waiting for the sort.

Checks: `test_async_sorting.gd` (results land, follow moving elements, and match the synchronous mode).
Visual: `S` toggles the mode in the showcase; the red cube must never pop in front of the wall it walks behind.

### 9. Maps instance their cells from a positionable set

An `IsometricMap` stores a 3D grid of ids. A `PositionableSet` maps each id to a positionable scene. Entering the tree,
the map instances one scene per filled cell, as a child at that cell, so every cell goes through steps 1 to 8.

Checks: `test_map.gd`.

### Known limits

- Boxes that partially interpenetrate (a character half under a bridge) get one z-index each: the painter's order
  cannot split a box, so part of the overlap is drawn wrongly.
- Canvas z-indices are capped by Godot (±4096): a single chain of more stacked elements than that saturates.

## Building

Requirements: Python 3, SCons 4, and the C++ toolchain Godot requires on your platform. Clone with submodules
(`git clone --recursive`, or `git submodule update --init`).

```bash
scons target=editor
scons target=template_debug
scons target=template_release
```

Libraries are written straight into `test/addons/isometric_maps/libs/`. Cross-platform builds use godot-cpp's
options: `platform=android arch=arm64`, `platform=ios arch=arm64`, `platform=macos arch=universal`.

- `editor` contains the editor plugins (map editor, positionable set editor, layers) and the debug views.
- `template_debug` is the game build with every check enabled.
- `template_release` is optimised for speed: checks that a correct project never triggers (invalid RIDs, moving a
  static element) are compiled out.

Only the engine classes listed in `build_profile.json` are bound, which keeps the libraries small.

## Testing

See [test/README.md](test/README.md): unit tests run headless with
`godot --headless --path test -s res://tests/run_tests.gd`, and visual scenes are checked by eye.

## CI and releases

Every pull request builds Windows, Linux, macOS, Android and iOS in editor, debug and release, then runs the unit
tests. Pushing a tag `x.y.z` does the same and publishes `isometric_maps-x.y.z.zip` as a GitHub release.
