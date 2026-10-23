## Topological sorting through the IsometricServer API, in synchronous mode so results are deterministic.
extends TestCase

var space: RID
var elements: Array[RID] = []


func before_each() -> void:
	IsometricServer.set_synchronous_mode(true)
	space = IsometricServer.space_create()


func after_each() -> void:
	for element in elements:
		IsometricServer.free_rid(element)
	elements.clear()
	IsometricServer.free_rid(space)
	IsometricServer.set_synchronous_mode(false)


func add_element(position: Vector3, is_dynamic := false, size := Vector3.ONE, depth := 1) -> RID:
	var element := IsometricServer.isometric_element_create(is_dynamic, AABB(position, size))
	IsometricServer.isometric_element_set_depth(element, depth)
	IsometricServer.space_attach_isometric_element(space, element)
	elements.append(element)
	return element


func sort() -> void:
	IsometricServer.fetch_data_and_request_ordering()


func z_of(element: RID) -> int:
	return IsometricServer.isometric_element_get_z_order(element)


func test_lone_element_is_at_zero() -> void:
	var element := add_element(Vector3.ZERO)
	sort()
	assert_eq(z_of(element), 0)


func test_element_in_front_on_x_is_drawn_over() -> void:
	var behind := add_element(Vector3(0, 0, 0))
	var front := add_element(Vector3(1, 0, 0))
	sort()
	assert_eq(z_of(behind), 0)
	assert_eq(z_of(front), 1)


func test_element_in_front_on_y_is_drawn_over() -> void:
	var front := add_element(Vector3(0, 1, 0))
	var behind := add_element(Vector3(0, 0, 0))
	sort()
	assert_gt(z_of(front), z_of(behind))


func test_element_above_is_drawn_over() -> void:
	var below := add_element(Vector3(0, 0, 0))
	var above := add_element(Vector3(0, 0, 1))
	sort()
	assert_gt(z_of(above), z_of(below))


func test_attach_order_does_not_change_the_result() -> void:
	var front := add_element(Vector3(1, 0, 0))
	var behind := add_element(Vector3(0, 0, 0))
	sort()
	assert_eq(z_of(behind), 0)
	assert_eq(z_of(front), 1)


func test_chain_accumulates_z_order() -> void:
	var first := add_element(Vector3(0, 0, 0))
	var second := add_element(Vector3(1, 0, 0))
	var third := add_element(Vector3(2, 0, 0))
	sort()
	assert_eq(z_of(first), 0)
	assert_eq(z_of(second), 1)
	assert_eq(z_of(third), 2)


func test_depth_of_element_behind_offsets_z_order() -> void:
	var behind := add_element(Vector3(0, 0, 0), false, Vector3.ONE, 3)
	var front := add_element(Vector3(1, 0, 0))
	sort()
	assert_eq(z_of(front), 3)
	assert_eq(z_of(behind), 0)


func test_distant_elements_do_not_constrain_each_other() -> void:
	var first := add_element(Vector3(0, 0, 0))
	var second := add_element(Vector3(10, -10, 0))
	sort()
	assert_eq(z_of(first), 0)
	assert_eq(z_of(second), 0)


func test_dynamic_element_is_sorted_against_statics() -> void:
	var wall := add_element(Vector3(1, 0, 0))
	var character := add_element(Vector3(2, 0, 0), true)
	sort()
	assert_gt(z_of(character), z_of(wall))

	IsometricServer.isometric_element_set_position(character, Vector3(0, 0, 0))
	sort()
	assert_gt(z_of(wall), z_of(character), "the character walked behind the wall")


func test_dynamic_elements_are_sorted_together() -> void:
	var first := add_element(Vector3(0, 0, 0), true)
	var second := add_element(Vector3(1, 0, 0), true)
	sort()
	assert_gt(z_of(second), z_of(first))

	IsometricServer.isometric_element_set_position(first, Vector3(2, 0, 0))
	sort()
	assert_gt(z_of(first), z_of(second))


func test_resizing_dynamic_element_updates_ordering() -> void:
	var first := add_element(Vector3(0, 0, 0), true)
	var second := add_element(Vector3(3, 0, 0), true)
	sort()
	assert_eq(z_of(second), 0, "too far apart to overlap")

	IsometricServer.isometric_element_set_size(first, Vector3(3, 1, 1))
	sort()
	assert_gt(z_of(second), z_of(first))


func test_detached_element_no_longer_constrains_others() -> void:
	var behind := add_element(Vector3(0, 0, 0), false, Vector3.ONE, 5)
	var front := add_element(Vector3(1, 0, 0))
	sort()
	assert_eq(z_of(front), 5)

	IsometricServer.space_detach_isometric_element(behind)
	sort()
	assert_eq(z_of(front), 0)


func test_freed_element_no_longer_constrains_others() -> void:
	var behind := IsometricServer.isometric_element_create(false, AABB(Vector3.ZERO, Vector3.ONE))
	IsometricServer.isometric_element_set_depth(behind, 4)
	IsometricServer.space_attach_isometric_element(space, behind)
	var front := add_element(Vector3(1, 0, 0))
	sort()
	assert_eq(z_of(front), 4)

	IsometricServer.free_rid(behind)
	sort()
	assert_eq(z_of(front), 0)


func test_spaces_are_sorted_independently() -> void:
	var other_space := IsometricServer.space_create()
	var behind := add_element(Vector3(0, 0, 0))
	var elsewhere := IsometricServer.isometric_element_create(false, AABB(Vector3(1, 0, 0), Vector3.ONE))
	IsometricServer.space_attach_isometric_element(other_space, elsewhere)
	sort()
	assert_eq(z_of(behind), 0)
	assert_eq(z_of(elsewhere), 0, "an element of another space is not ordered against this one")
	IsometricServer.free_rid(elsewhere)
	IsometricServer.free_rid(other_space)


func test_large_grid_is_ordered_consistently() -> void:
	var grid := {}
	for x in 6:
		for y in 6:
			grid[Vector2i(x, y)] = add_element(Vector3(x, y, 0))
	sort()
	for x in 6:
		for y in 6:
			var z := z_of(grid[Vector2i(x, y)])
			if x > 0:
				assert_gt(z, z_of(grid[Vector2i(x - 1, y)]), "(%d, %d) over its x neighbour" % [x, y])
			if y > 0:
				assert_gt(z, z_of(grid[Vector2i(x, y - 1)]), "(%d, %d) over its y neighbour" % [x, y])


# Spaces are split in 10 x 10 chunks; elements are only compared with the ones of neighboring chunks.
func test_neighbors_across_a_chunk_border_are_sorted() -> void:
	var behind := add_element(Vector3(9, 3, 0))
	var front := add_element(Vector3(10, 3, 0))
	sort()
	assert_gt(z_of(front), z_of(behind))


func test_diagonal_neighbors_across_chunk_corners_are_sorted() -> void:
	var behind := add_element(Vector3(9, 9, 0), true)
	var front := add_element(Vector3(10, 10, 0), true)
	sort()
	assert_gt(z_of(front), z_of(behind))


func test_dynamic_element_moving_to_another_chunk_is_sorted_there() -> void:
	var wall := add_element(Vector3(25, 3, 0))
	var character := add_element(Vector3(5, 3, 0), true)
	sort()
	assert_eq(z_of(character), 0, "far from the wall")

	IsometricServer.isometric_element_set_position(character, Vector3(26, 3, 0))
	sort()
	assert_gt(z_of(character), z_of(wall), "in front of the wall")

	IsometricServer.isometric_element_set_position(character, Vector3(24, 3, 0))
	sort()
	assert_gt(z_of(wall), z_of(character), "behind the wall")


func test_elements_at_negative_positions_are_sorted() -> void:
	var behind := add_element(Vector3(-1, 0, 0))
	var front := add_element(Vector3(0, 0, 0))
	sort()
	assert_gt(z_of(front), z_of(behind))
