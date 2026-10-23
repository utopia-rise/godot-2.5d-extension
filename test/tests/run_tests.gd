## Runs every TestCase script under res://tests/unit and exits with code 1 if any test fails.
## godot --headless --path test -s res://tests/run_tests.gd
extends SceneTree

const UNIT_TESTS_DIRECTORY := "res://tests/unit"


func _initialize() -> void:
	_run.call_deferred()


func _run() -> void:
	var test_count := 0
	var failed_tests: PackedStringArray = []

	for file in DirAccess.get_files_at(UNIT_TESTS_DIRECTORY):
		if not file.ends_with(".gd"):
			continue
		var script: GDScript = load(UNIT_TESTS_DIRECTORY.path_join(file))
		for method in script.get_script_method_list():
			var test_name: String = method["name"]
			if not test_name.begins_with("test_"):
				continue
			test_count += 1
			var failures := await _run_test(script, test_name)
			var full_name := "%s::%s" % [file.get_basename(), test_name]
			if failures.is_empty():
				print("  PASS %s" % full_name)
			else:
				failed_tests.append(full_name)
				print("  FAIL %s" % full_name)
				for failure in failures:
					print("       %s" % failure)

	print("\n%d tests, %d passed, %d failed" % [test_count, test_count - failed_tests.size(), failed_tests.size()])
	quit(1 if test_count == 0 or not failed_tests.is_empty() else 0)


func _run_test(script: GDScript, test_name: String) -> PackedStringArray:
	var test: TestCase = script.new()
	root.add_child(test)
	await test.before_each()
	await test.call(test_name)
	await test.after_each()
	var failures := test.failures
	test.queue_free()
	await process_frame
	return failures
