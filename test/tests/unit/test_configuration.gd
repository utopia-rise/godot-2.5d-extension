## IsometricConfiguration applied to a space.
extends TestCase


func test_configuration_defaults() -> void:
	var configuration := IsometricConfiguration.new()
	assert_eq(configuration.diamond_width, 256)
	assert_eq(configuration.angle, 30)
	assert_almost_eq(configuration.margin, 0.1)


func test_configuration_changes_space_projection() -> void:
	var space := IsometricServer.space_create()
	var configuration := IsometricConfiguration.new()
	configuration.diamond_width = 128
	configuration.angle = 45
	IsometricServer.space_update_configuration(space, configuration)

	assert_eq(IsometricServer.space_get_diamond_width(space), 128)
	assert_eq(IsometricServer.space_get_diamond_height(space), roundi(128 * sin(deg_to_rad(45))))
	assert_almost_eq(IsometricServer.space_get_z_length(space), 128 / sqrt(2.0) * cos(deg_to_rad(45)), 0.01)
	IsometricServer.free_rid(space)

