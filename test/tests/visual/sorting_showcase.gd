## Visual check of the depth sorting: a red dynamic cube travels through static structures, and must always be hidden by
## what stands in front of it and drawn over what stands behind it.
## Keys: S toggles synchronous / asynchronous sorting, D toggles the debug view (cycles drawn in red), Space pauses.
extends Node2D

const DIAMOND_WIDTH := 256.0
const DIAMOND_HEIGHT := 128.0
const Z_LENGTH := 156.767
const MAP_SIZE := Vector3(8, 8, 4)
const PATH_PERIOD := 12.0

var map: IsometricMap
var traveller: IsometricPositionable
var info: Label
var elapsed := 0.0
var paused := false
var debug := false


func _ready() -> void:
    var camera := Camera2D.new()
    camera.position = Vector2(0, 450)
    camera.zoom = Vector2(0.55, 0.55)
    add_child(camera)

    map = IsometricMap.new()
    map.size = MAP_SIZE
    add_child(map)

    for x in 8:
        for y in 8:
            add_cube(Vector3(x, y, 0), Vector3.ONE, Color(0.35, 0.55, 0.35))
    # A wall along x, a tower, a bridge and a pillar the traveller crosses around.
    for x in range(1, 7):
        add_cube(Vector3(x, 2, 1), Vector3.ONE, Color(0.7, 0.6, 0.45))
    add_cube(Vector3(5, 5, 1), Vector3(1, 1, 2), Color(0.45, 0.5, 0.75))
    add_cube(Vector3(2, 5, 2), Vector3(3, 1, 1), Color(0.75, 0.75, 0.75))
    add_cube(Vector3(2, 5, 1), Vector3.ONE, Color(0.6, 0.6, 0.6))
    add_cube(Vector3(4, 5, 1), Vector3.ONE, Color(0.6, 0.6, 0.6))

    traveller = add_cube(Vector3(0, 0, 1), Vector3.ONE, Color(0.9, 0.2, 0.2), true)

    var hud := CanvasLayer.new()
    add_child(hud)
    info = Label.new()
    info.position = Vector2(16, 16)
    hud.add_child(info)


func _process(delta: float) -> void:
    if not paused:
        elapsed += delta
    traveller.local_position_3d = path_position(fmod(elapsed, PATH_PERIOD) / PATH_PERIOD)
    info.text = (
		"Sorting: %s (S)   Debug: %s (D)   Pause: Space\nTraveller at %s, z-index %d"
        % [
            "synchronous" if IsometricServer.is_synchronous_mode() else "asynchronous",
            "on" if debug else "off",
            traveller.local_position_3d.snappedf(0.01),
            IsometricServer.isometric_element_get_z_order(traveller.get_isometric_element_rid()),
        ]
    )


func _unhandled_key_input(event: InputEvent) -> void:
    if not event.is_pressed():
        return
    match event.keycode:
        KEY_S:
            IsometricServer.set_synchronous_mode(not IsometricServer.is_synchronous_mode())
        KEY_D:
            debug = not debug
            IsometricServer.set_debug(debug)
        KEY_SPACE:
            paused = not paused


## A loop around the map: in front of the wall, behind the tower, under the bridge and back.
func path_position(progress: float) -> Vector3:
    var corners: Array[Vector3] = [
        Vector3(0, 0, 1), Vector3(7, 0, 1), Vector3(7, 7, 1), Vector3(3, 7, 1), Vector3(3, 4, 1), Vector3(0, 4, 1)
    ]
    var scaled := progress * corners.size()
    var index := int(scaled) % corners.size()
    return corners[index].lerp(corners[(index + 1) % corners.size()], scaled - int(scaled))


func add_cube(position: Vector3, size: Vector3, color: Color, is_dynamic := false) -> IsometricPositionable:
    var cube := IsometricPositionable.new()
    cube.size = size
    cube.local_position_3d = position
    cube.is_dynamic = is_dynamic
    add_faces(cube, size, color)
    map.add_child(cube)
    return cube


## The three visible faces of a box, built from the same bounding box the extension uses for its outlines.
func add_faces(cube: Node2D, size: Vector3, color: Color) -> void:
    var offset := Vector2(0, -DIAMOND_HEIGHT * 0.5)
    var lower: Array[Vector2] = [
        offset,
        Vector2(DIAMOND_WIDTH * 0.5 * size.x, DIAMOND_HEIGHT * 0.5 * size.x) + offset,
        Vector2(DIAMOND_WIDTH * 0.5 * (size.x - size.y), DIAMOND_HEIGHT * 0.5 * (size.x + size.y)) + offset,
        Vector2(-DIAMOND_WIDTH * 0.5 * size.y, DIAMOND_HEIGHT * 0.5 * size.y) + offset,
    ]
    var height := Vector2(0, -Z_LENGTH * size.z)
    var upper: Array[Vector2] = []
    for point in lower:
        upper.append(point + height)

    add_face(cube, [upper[0], upper[1], upper[2], upper[3]], color)
    add_face(cube, [upper[1], lower[1], lower[2], upper[2]], color.darkened(0.25))
    add_face(cube, [upper[2], lower[2], lower[3], upper[3]], color.darkened(0.45))


func add_face(cube: Node2D, points: Array[Vector2], color: Color) -> void:
    var face := Polygon2D.new()
    face.polygon = PackedVector2Array(points)
    face.color = color
    var outline := Line2D.new()
    outline.points = PackedVector2Array(points + [points[0]])
    outline.width = 2
    outline.default_color = color.darkened(0.6)
    face.add_child(outline)
    cube.add_child(face)
