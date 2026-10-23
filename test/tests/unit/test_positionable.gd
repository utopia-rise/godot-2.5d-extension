## IsometricPositionable nodes in the scene tree: 3D to screen projection, spaces and sorting.
extends TestCase

## Default configuration: 256 pixels wide diamonds seen at 30 degrees.
const DIAMOND_WIDTH := 256
const DIAMOND_HEIGHT := 128
const Z_LENGTH := 156.767

var container: Node2D


func before_each() -> void:
	IsometricServer.set_synchronous_mode(true)
	container = Node2D.new()
	add_child(container)


func after_each() -> void:
	container.queue_free()
	IsometricServer.set_synchronous_mode(false)


func add_positionable(position: Vector3, parent: Node = container, is_dynamic := false) -> IsometricPositionable:
	var positionable := IsometricPositionable.new()
	positionable.local_position_3d = position
	positionable.is_dynamic = is_dynamic
	parent.add_child(positionable)
	return positionable


func z_of(positionable: IsometricPositionable) -> int:
	return IsometricServer.isometric_element_get_z_order(positionable.get_isometric_element_rid())


func test_default_configuration_of_a_space() -> void:
	var positionable := add_positionable(Vector3.ZERO)
	var space := positionable.get_space_rid()
	assert_true(space.is_valid())
	assert_eq(IsometricServer.space_get_diamond_width(space), DIAMOND_WIDTH)
	assert_eq(IsometricServer.space_get_diamond_height(space), DIAMOND_HEIGHT)
	assert_almost_eq(IsometricServer.space_get_z_length(space), Z_LENGTH, 0.01)


func test_positions_are_projected_on_screen() -> void:
	var cases := {
		Vector3(0, 0, 0): Vector2(0, 0),
		Vector3(1, 0, 0): Vector2(DIAMOND_WIDTH / 2.0, DIAMOND_HEIGHT / 2.0),
		Vector3(0, 1, 0): Vector2(-DIAMOND_WIDTH / 2.0, DIAMOND_HEIGHT / 2.0),
		Vector3(0, 0, 1): Vector2(0, -Z_LENGTH),
		Vector3(2, 3, 1): Vector2(-DIAMOND_WIDTH / 2.0, 5 * DIAMOND_HEIGHT / 2.0 - Z_LENGTH),
	}
	for position_3d in cases:
		var positionable := add_positionable(position_3d)
		assert_almost_eq(positionable.position, cases[position_3d], 0.01, "projection of %s" % position_3d)


func test_moving_updates_screen_position() -> void:
	var positionable := add_positionable(Vector3.ZERO, container, true)
	positionable.local_position_3d = Vector3(1, 1, 0)
	assert_almost_eq(positionable.position, Vector2(0, DIAMOND_HEIGHT), 0.01)


func test_children_share_the_space_of_their_parent() -> void:
	var parent := add_positionable(Vector3(1, 1, 0))
	var child := add_positionable(Vector3(1, 0, 0), parent)
	assert_eq(child.get_space_rid(), parent.get_space_rid())
	assert_eq(child.get_global_position_3d(), Vector3(2, 1, 0))


func test_siblings_under_a_plain_node_get_their_own_space() -> void:
	var first := add_positionable(Vector3.ZERO)
	var second := add_positionable(Vector3(1, 0, 0))
	assert_ne(first.get_space_rid(), second.get_space_rid())


func test_children_are_sorted_in_their_parent_space() -> void:
	var map := IsometricMap.new()
	map.size = Vector3(4, 4, 1)
	container.add_child(map)
	var behind := add_positionable(Vector3(0, 0, 0), map)
	var front := add_positionable(Vector3(1, 0, 0), map)
	IsometricServer.fetch_data_and_request_ordering()
	assert_eq(z_of(behind), 0)
	assert_eq(z_of(front), 1)


func test_dynamic_positionable_reorders_when_moving() -> void:
	var map := IsometricMap.new()
	map.size = Vector3(4, 4, 1)
	container.add_child(map)
	var wall := add_positionable(Vector3(1, 0, 0), map)
	var character := add_positionable(Vector3(2, 0, 0), map, true)
	IsometricServer.fetch_data_and_request_ordering()
	assert_gt(z_of(character), z_of(wall))

	character.local_position_3d = Vector3(0, 0, 0)
	IsometricServer.fetch_data_and_request_ordering()
	assert_gt(z_of(wall), z_of(character))


func test_switching_to_dynamic_keeps_the_element_sorted() -> void:
	var map := IsometricMap.new()
	map.size = Vector3(4, 4, 1)
	container.add_child(map)
	var behind := add_positionable(Vector3(0, 0, 0), map)
	var front := add_positionable(Vector3(1, 0, 0), map)
	front.is_dynamic = true
	IsometricServer.fetch_data_and_request_ordering()
	assert_gt(z_of(front), z_of(behind))


func test_leaving_the_tree_releases_the_element() -> void:
	var positionable := add_positionable(Vector3.ZERO)
	assert_true(positionable.get_isometric_element_rid().is_valid())
	container.remove_child(positionable)
	assert_false(positionable.get_isometric_element_rid().is_valid())
	assert_false(positionable.get_space_rid().is_valid())
	positionable.free()


func test_depth_property() -> void:
	var map := IsometricMap.new()
	map.size = Vector3(4, 4, 1)
	container.add_child(map)
	var behind := add_positionable(Vector3(0, 0, 0), map, true)
	behind.depth = 4
	var front := add_positionable(Vector3(1, 0, 0), map)
	IsometricServer.fetch_data_and_request_ordering()
	assert_eq(z_of(front), 4)
