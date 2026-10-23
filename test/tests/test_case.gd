## Base class of the unit tests. Every method whose name starts with "test_" is a test; it may await.
## A fresh instance runs each test, with before_each and after_each around it.
class_name TestCase
extends Node

var failures: PackedStringArray = []


func before_each() -> void:
	pass


func after_each() -> void:
	pass


func fail(message: String) -> void:
	failures.append(message)


func assert_true(condition: bool, message: String = "") -> void:
	if not condition:
		fail("expected true. %s" % message)


func assert_false(condition: bool, message: String = "") -> void:
	if condition:
		fail("expected false. %s" % message)


func assert_eq(actual: Variant, expected: Variant, message: String = "") -> void:
	if not is_same_value(actual, expected):
		fail("expected %s, got %s. %s" % [var_to_str(expected), var_to_str(actual), message])


func assert_ne(actual: Variant, unexpected: Variant, message: String = "") -> void:
	if is_same_value(actual, unexpected):
		fail("expected anything but %s. %s" % [var_to_str(unexpected), message])


func assert_almost_eq(actual: Variant, expected: Variant, tolerance: float = 0.001, message: String = "") -> void:
	var difference: Variant = actual - expected
	var is_close: bool
	match typeof(difference):
		TYPE_FLOAT, TYPE_INT:
			is_close = absf(difference) <= tolerance
		_:
			is_close = difference.length() <= tolerance
	if not is_close:
		fail("expected %s ± %s, got %s. %s" % [var_to_str(expected), tolerance, var_to_str(actual), message])


func assert_lt(actual: Variant, bound: Variant, message: String = "") -> void:
	if not actual < bound:
		fail("expected %s < %s. %s" % [var_to_str(actual), var_to_str(bound), message])


func assert_gt(actual: Variant, bound: Variant, message: String = "") -> void:
	if not actual > bound:
		fail("expected %s > %s. %s" % [var_to_str(actual), var_to_str(bound), message])


## Equality that also tells types apart (1 is not 1.0), except that a null object equals null.
func is_same_value(first: Variant, second: Variant) -> bool:
	if first == null or second == null:
		return first == null and second == null
	return typeof(first) == typeof(second) and first == second


## Lets the main loop run, so deferred calls, frame_pre_draw and the IsometricServer thread make progress.
func wait_frames(count: int = 1) -> void:
	for i in count:
		await get_tree().process_frame
