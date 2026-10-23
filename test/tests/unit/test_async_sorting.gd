## The default asynchronous mode: sorts run on the server thread and land on a later frame.
extends TestCase

const MAX_WAITED_FRAMES := 30

var space: RID
var elements: Array[RID] = []


func before_each() -> void:
	IsometricServer.set_synchronous_mode(false)
	space = IsometricServer.space_create()


func after_each() -> void:
	for element in elements:
		IsometricServer.free_rid(element)
	elements.clear()
	IsometricServer.free_rid(space)


func add_element(position: Vector3, is_dynamic := false) -> RID:
	var element := IsometricServer.isometric_element_create(is_dynamic, AABB(position, Vector3.ONE))
	IsometricServer.space_attach_isometric_element(space, element)
	elements.append(element)
	return element


## Requests orderings frame after frame until the condition holds, as the engine does on frame_pre_draw.
func sort_until(condition: Callable) -> bool:
	for i in MAX_WAITED_FRAMES:
		IsometricServer.fetch_data_and_request_ordering()
		if condition.call():
			return true
		await wait_frames()
	return false


func test_server_is_asynchronous_by_default() -> void:
	assert_false(IsometricServer.is_synchronous_mode())


func test_asynchronous_sort_lands() -> void:
	var behind := add_element(Vector3(0, 0, 0))
	var front := add_element(Vector3(1, 0, 0))
	var landed := await sort_until(func() -> bool: return IsometricServer.isometric_element_get_z_order(front) == 1)
	assert_true(landed, "the sort result reached the main thread")
	assert_eq(IsometricServer.isometric_element_get_z_order(behind), 0)


func test_asynchronous_sort_follows_moving_element() -> void:
	var wall := add_element(Vector3(1, 0, 0))
	var character := add_element(Vector3(2, 0, 0), true)
	assert_true(await sort_until(func() -> bool: return IsometricServer.isometric_element_get_z_order(character) > 0))

	IsometricServer.isometric_element_set_position(character, Vector3(0, 0, 0))
	var landed := await sort_until(
		func() -> bool:
			return IsometricServer.isometric_element_get_z_order(wall) > IsometricServer.isometric_element_get_z_order(character)
	)
	assert_true(landed, "the character went behind the wall")


func test_asynchronous_and_synchronous_modes_agree() -> void:
	var expected := {}
	for x in 4:
		for y in 4:
			expected[add_element(Vector3(x, y, 0))] = 0

	IsometricServer.set_synchronous_mode(true)
	IsometricServer.fetch_data_and_request_ordering()
	for element in expected:
		expected[element] = IsometricServer.isometric_element_get_z_order(element)

	# Moving an element back and forth dirties the space without changing the expected result.
	var mover := add_element(Vector3(10, 10, 0), true)
	IsometricServer.set_synchronous_mode(false)
	IsometricServer.isometric_element_set_position(mover, Vector3(20, 20, 0))
	var all_match := func() -> bool:
		for element in expected:
			if IsometricServer.isometric_element_get_z_order(element) != expected[element]:
				return false
		return true
	assert_true(await sort_until(all_match))


func test_switching_to_synchronous_mode_flushes_pending_sort() -> void:
	var behind := add_element(Vector3(0, 0, 0))
	var front := add_element(Vector3(1, 0, 0))
	IsometricServer.fetch_data_and_request_ordering()

	IsometricServer.set_synchronous_mode(true)
	IsometricServer.fetch_data_and_request_ordering()
	assert_eq(IsometricServer.isometric_element_get_z_order(front), 1)
	assert_eq(IsometricServer.isometric_element_get_z_order(behind), 0)
	IsometricServer.set_synchronous_mode(false)
