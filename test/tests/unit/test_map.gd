## IsometricMap: instancing positionables from its grid and PositionableSet, and resizing.
extends TestCase

const NONE := -1
const BLOCK_ID := 2

var map: IsometricMap


func before_each() -> void:
	IsometricServer.set_synchronous_mode(true)


func after_each() -> void:
	if is_instance_valid(map):
		map.queue_free()
	IsometricServer.set_synchronous_mode(false)


func create_set() -> PositionableSet:
	var positionable_set := PositionableSet.new()
	positionable_set.set("identifier_to_scene_path", {BLOCK_ID: "res://block.tscn"})
	return positionable_set


func create_map(size: Vector3, grid: Array) -> IsometricMap:
	map = IsometricMap.new()
	map.size = size
	map.set("grid_3d", grid)
	map.positionable_set = create_set()
	return map


func test_positionable_set_loads_its_scenes() -> void:
	var positionable_set := create_set()
	assert_false(positionable_set.is_set_loaded())
	positionable_set.preload_scenes()
	assert_true(positionable_set.is_set_loaded())
	assert_ne(positionable_set.get_positionable_scene_for_id(BLOCK_ID), null)
	assert_eq(positionable_set.get_positionable_scene_for_id(42), null)


func test_grid_cells_are_instanced_in_the_tree() -> void:
	create_map(Vector3(2, 2, 1), [BLOCK_ID, NONE, NONE, BLOCK_ID])
	add_child(map)

	var positionables := map.get_children().filter(func(child: Node) -> bool: return child is IsometricPositionable)
	assert_eq(positionables.size(), 2)
	var positions := positionables.map(func(child: IsometricPositionable) -> Vector3: return child.local_position_3d)
	assert_true(positions.has(Vector3(0, 0, 0)))
	assert_true(positions.has(Vector3(1, 1, 0)))
	assert_eq(map.get_positionable_id_for_position(Vector3(0, 0, 0)), BLOCK_ID)
	assert_eq(map.get_positionable_id_for_position(Vector3(1, 0, 0)), NONE)


func test_grid_index_covers_every_dimension() -> void:
	var grid := []
	grid.resize(2 * 3 * 2)
	grid.fill(NONE)
	# Index = x + width * y + width * depth * z
	grid[1 + 2 * 2 + 2 * 3 * 1] = BLOCK_ID
	create_map(Vector3(2, 3, 2), grid)
	add_child(map)

	var positionables := map.get_children().filter(func(child: Node) -> bool: return child is IsometricPositionable)
	assert_eq(positionables.size(), 1)
	assert_eq(positionables[0].local_position_3d, Vector3(1, 2, 1))


func test_instanced_cells_share_the_map_space_and_are_sorted() -> void:
	create_map(Vector3(2, 1, 1), [BLOCK_ID, BLOCK_ID])
	add_child(map)
	IsometricServer.fetch_data_and_request_ordering()

	var z_by_x := {}
	for child in map.get_children():
		if child is IsometricPositionable:
			assert_eq(child.get_space_rid(), map.get_space_rid())
			z_by_x[int(child.local_position_3d.x)] = IsometricServer.isometric_element_get_z_order(child.get_isometric_element_rid())
	assert_gt(z_by_x[1], z_by_x[0])


func test_resizing_keeps_existing_cells() -> void:
	create_map(Vector3(2, 2, 1), [BLOCK_ID, NONE, NONE, BLOCK_ID])
	map.size = Vector3(3, 3, 2)
	var grid: Array = map.get("grid_3d")
	assert_eq(grid.size(), 18)
	assert_eq(grid[0], BLOCK_ID)
	assert_eq(grid[1 + 3 * 1], BLOCK_ID, "cell (1, 1, 0) moved to its new index")
	assert_eq(grid.count(BLOCK_ID), 2)


func test_shrinking_drops_cells_outside_the_map() -> void:
	create_map(Vector3(2, 2, 1), [BLOCK_ID, NONE, NONE, BLOCK_ID])
	map.size = Vector3(1, 1, 1)
	assert_eq(map.get("grid_3d"), [BLOCK_ID])


func test_saved_scene_reloads_its_cells() -> void:
	var scene: PackedScene = load("res://maps/paradox.tscn")
	var root := scene.instantiate()
	add_child(root)
	var positionables := root.find_children("*", "IsometricPositionable", true, false)
	assert_gt(positionables.size(), 1, "paradox.tscn holds positionables")
	root.queue_free()
