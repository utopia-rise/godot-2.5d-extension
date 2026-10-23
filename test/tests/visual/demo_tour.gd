## Scripted tour of the test project, made to be recorded:
## godot --path test --write-movie demo.avi --fixed-fps 30 --resolution 1920x1080 res://tests/visual/demo_tour.tscn
## Each chapter loads a scene, drives it and explains it with subtitles; the tour quits at the end.
extends Node

const SHOWCASE_SCENE := preload("res://tests/visual/sorting_showcase.tscn")
const DOJO_SCENE := preload("res://dojo/Dojo.tscn")
const BIG_DOJO_SCENE := preload("res://dojo/Dojobig100x100x10.tscn")

const DEBUG_VIEW_GROUP := "isometric_debug_view"
# Every duration below is scaled by PACE: raise it to slow the whole tour down.
const PACE := 1.0
const SUBTITLE_GAP := 0.3

var stage: Node
var debug_enabled := false
var ui: CanvasLayer
var subtitle_panel: PanelContainer
var subtitle_label: Label
var chapter_label: Label
var badges_label: Label
var card: ColorRect
var card_title: Label
var card_subtitle: Label
var comparison_backdrop: ColorRect
var comparison: HBoxContainer
var is_measuring := false
var is_walking := false
var live_subtitle_format := ""
var request_cost_ms := 0.0


func _ready() -> void:
	_build_ui()
	_play.call_deferred()


func _play() -> void:
	await _title_card("Isometric Maps", "A Godot 4.7 GDExtension for 2.5D isometric maps", 3.0)

	await _chapter_depth_sorting()
	await _chapter_threading()
	await _chapter_debug_view()
	await _chapter_physics()
	await _chapter_big_map()

	await _title_card("Isometric Maps", "github.com/utopia-rise/godot-2.5d-extension", 3.0)
	get_tree().quit()


func _chapter_depth_sorting() -> void:
	_set_stage(SHOWCASE_SCENE.instantiate())
	stage.info.visible = false
	_set_chapter("1 · Depth sorting")
	await _say("Every block is an IsometricPositionable: a 2D node that also has a 3D position and size.", 5.0)
	await _say("The red cube moves in 3D. Each frame, the IsometricServer orders every box by depth.", 5.0)
	await _say("It goes behind the wall and the tower, and under the bridge, without any manual z-index.", 5.5)


func _chapter_threading() -> void:
	_set_chapter("2 · Asynchronous and synchronous sorting")
	await _say("Sorting runs on the IsometricServer's own thread: a WorkerThreadPool task, like Godot's rendering and physics servers.", 6.0)
	await _say("The game only sends it commands: create, move, resize or free a box. The thread owns the sorting data, so nothing is locked.", 6.5)
	await _say("Asynchronous mode, the default: right before a frame is drawn, the main thread applies the last finished sort and starts the next one.", 7.0)
	await _say("The sort runs while the game computes the next frame, so the main thread never waits... but the order shown is one frame old.", 6.5)

	# The cube jumps from in front of the wall to behind it; the two frames that follow are frozen side by side.
	stage.set_process(false)
	stage.traveller.local_position_3d = Vector3(6, 3.2, 1)
	await _say("Let's freeze time: the red cube is about to jump from in front of the wall to behind it.", 4.5)
	var async_frames := await _capture_jump()
	_show_comparison(async_frames, "Asynchronous: the frame of the jump", "The next frame")
	await _say("Asynchronous, frame of the jump: the cube already stands behind the wall, but is drawn with the previous order: it shows through.", 7.0)
	await _say("One frame later, the new order lands and the wall hides the cube. In motion, this is a one-frame flicker at depth changes.", 7.0)
	_hide_comparison()

	IsometricServer.set_synchronous_mode(true)
	_set_modes()
	stage.traveller.local_position_3d = Vector3(6, 3.2, 1)
	await _say("Synchronous mode: the main thread waits for the sort and applies it within the same frame.", 5.5)
	var sync_frames := await _capture_jump()
	_show_comparison(sync_frames, "Synchronous: the frame of the jump", "The next frame")
	await _say("Same jump, synchronous: the order is right on the very frame of the move. No flicker.", 6.5)
	await _say("The price: the whole sorting time is spent on the main thread, every frame something moves. The large map below shows it.", 7.0)
	_hide_comparison()

	stage.set_process(true)
	await _say("Both modes give the same order in the end. The rest of this tour stays synchronous.", 4.5)


## Keeps the duck moving, so its space has to be sorted again every frame.
func _walk_duck_in_loop() -> void:
	var directions := ["player_backward", "player_right", "player_forward", "player_left"]
	var index := 0
	while is_walking:
		Input.action_press(directions[index])
		await get_tree().create_timer(1.5).timeout
		Input.action_release(directions[index])
		index = (index + 1) % directions.size()


## Moves the showcase cube behind the wall and returns the frame of the move and the next one, without the UI.
func _capture_jump() -> Array[Image]:
	ui.visible = false
	await RenderingServer.frame_post_draw
	stage.traveller.local_position_3d = Vector3(5, 1, 1)
	var frames: Array[Image] = []
	for i in 2:
		await RenderingServer.frame_post_draw
		frames.append(get_viewport().get_texture().get_image())
	ui.visible = true
	return frames


## Shows two captured frames side by side, cropped around the cube.
func _show_comparison(frames: Array[Image], first_caption: String, second_caption: String) -> void:
	var center: Vector2 = stage.traveller.get_global_transform_with_canvas().origin + Vector2(-40, 40)
	var crop_size := Vector2i(870, 640)
	var crop := Rect2i(Vector2i(center) - crop_size / 2, crop_size).intersection(Rect2i(Vector2i.ZERO, frames[0].get_size()))
	var captions := [first_caption, second_caption]
	for i in 2:
		var texture: TextureRect = comparison.get_child(i).get_child(0)
		texture.texture = ImageTexture.create_from_image(frames[i].get_region(crop))
		var caption: Label = comparison.get_child(i).get_child(1)
		caption.text = captions[i]
	comparison_backdrop.visible = true


func _hide_comparison() -> void:
	comparison_backdrop.visible = false


func _chapter_debug_view() -> void:
	_set_chapter("3 · Debug view")
	_set_debug(true)
	await _say("The debug view draws each element as the box the sorting works with.", 5.0)
	_set_debug(false)
	await _say("Debug features only exist in editor and debug builds; release builds drop them.", 4.5)


func _chapter_physics() -> void:
	_set_stage(DOJO_SCENE.instantiate())
	stage.get_node("CanvasLayer").visible = false
	var duck_camera: Camera2D = stage.get_node("IsometricMap/Character/Camera2D")
	duck_camera.zoom = Vector2(0.9, 0.9)
	_set_chapter("4 · 3D physics")
	_say("The duck is a CharacterBody3D under a positionable: 3D physics moves it, the map sorts it against the walls.", 9.0)
	for step in [["player_right", 2.0], ["player_backward", 2.5], ["player_left", 2.0], ["player_forward", 2.5]]:
		Input.action_press(step[0])
		await _wait(step[1])
		Input.action_release(step[0])

	# The dojo holds a map made to be impossible: the camera leaves the duck to look at it.
	var paradox: Node2D = stage.get_node("IsometricMap/IsometricMap")
	var camera := Camera2D.new()
	camera.global_position = duck_camera.get_screen_center_position()
	camera.zoom = duck_camera.zoom
	stage.add_child(camera)
	camera.make_current()
	_set_chapter("5 · Sorting cycles")
	var tween := create_tween().set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	tween.tween_property(camera, "global_position", paradox.global_position + Vector2(0, -60), 2.0 * PACE)
	tween.parallel().tween_property(camera, "zoom", Vector2(1.1, 1.1), 2.0 * PACE)
	await _say("Further in the dojo stands a paradox, made of three boxes that interlock.", 4.5)
	await _say("Box A is in front of box B, B is in front of C, and C is in front of A: the order loops.", 5.5)
	await _say("No drawing order can be right, so one of the boxes is always drawn wrongly.", 4.5)
	_set_debug(true)
	await _say("With debug on, every element is drawn as its sorting box, and the server tints the boxes caught in the cycle red.", 7.0)
	_set_debug(false)
	await _say("Maps like this one are made in the editor: paint, drag and drop, fill, select and move, layers, undo and redo.", 6.0)


func _chapter_big_map() -> void:
	var dojo: Node2D = BIG_DOJO_SCENE.instantiate()
	dojo.get_node("CanvasLayer").visible = false
	var camera := Camera2D.new()
	camera.position = Vector2(0, 600)
	camera.zoom = Vector2(0.6, 0.6)
	dojo.add_child(camera)
	_set_stage(dojo)
	camera.make_current()
	_set_chapter("6 · Large maps")
	is_walking = true
	_walk_duck_in_loop()
	var tween := create_tween().set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
	tween.tween_property(camera, "zoom", Vector2(0.05, 0.05), 8.0 * PACE)
	tween.parallel().tween_property(camera, "position", Vector2(0, 5800), 8.0 * PACE)
	await _say("A 100 × 100 × 10 map, with the duck walking on it: every frame, the space has to be sorted again.", 5.5)
	is_measuring = true
	await _say("Spaces are split into 10 × 10 chunks, so each box is only compared with its neighbors.", 5.0)
	await _say_live("Synchronous: the main thread waits for the whole sort of this map, every frame: %.2f ms per request.", 7.0)
	IsometricServer.set_synchronous_mode(false)
	_set_modes()
	await _say_live("Asynchronous: it only hands the work over and applies what changed: %.2f ms. The price is the one-frame delay.", 7.0)
	IsometricServer.set_synchronous_mode(true)
	_set_modes()
	await _say("Pick per game: asynchronous for large busy maps, synchronous when depth changes must never flicker.", 5.5)
	is_measuring = false
	is_walking = false


func _set_stage(new_stage: Node) -> void:
	if stage:
		stage.queue_free()
	_set_debug(false)
	stage = new_stage
	add_child(stage)
	move_child(stage, 0)
	_set_modes()


## Debug mode tints sorting cycles; the debug view also draws elements as their boxes.
func _set_debug(enabled: bool, with_box_view := true) -> void:
	debug_enabled = enabled
	IsometricServer.set_debug(enabled)
	get_tree().call_group(DEBUG_VIEW_GROUP, "set_debug_view", enabled and with_box_view)
	_set_modes()


func _set_modes() -> void:
	var lines := PackedStringArray(["%s sorting" % ("Synchronous" if IsometricServer.is_synchronous_mode() else "Asynchronous")])
	if debug_enabled:
		lines.append("Debug")
	if is_measuring:
		lines.append("Main thread per ordering request: %.2f ms" % request_cost_ms)
	badges_label.text = "
".join(lines)


## While measuring, each frame's ordering request is made here, timed, instead of only by the engine's frame hook.
func _process(_delta: float) -> void:
	if not is_measuring:
		return
	var start := Time.get_ticks_usec()
	IsometricServer.fetch_data_and_request_ordering()
	request_cost_ms = lerpf(request_cost_ms, (Time.get_ticks_usec() - start) / 1000.0, 0.1)
	_set_modes()
	if not live_subtitle_format.is_empty():
		subtitle_label.text = live_subtitle_format % request_cost_ms


func _set_chapter(text: String) -> void:
	chapter_label.text = text


func _wait(seconds: float) -> void:
	await get_tree().create_timer(seconds * PACE).timeout


## Like _say, with the measured cost of an ordering request kept up to date in the %.2f of the text.
func _say_live(text_format: String, seconds: float) -> void:
	live_subtitle_format = text_format
	await _say(text_format % request_cost_ms, seconds)
	live_subtitle_format = ""


## Shows a subtitle for a while, then leaves a short gap; await it to wait until it ends.
func _say(text: String, seconds: float) -> void:
	subtitle_label.text = text
	subtitle_panel.visible = true
	await _wait(seconds)
	subtitle_panel.visible = false
	await _wait(SUBTITLE_GAP)


func _title_card(title: String, subtitle: String, seconds: float) -> void:
	card_title.text = title
	card_subtitle.text = subtitle
	subtitle_panel.visible = false
	chapter_label.text = ""
	badges_label.text = ""
	card.modulate.a = 0.0
	card.visible = true
	var tween := create_tween()
	tween.tween_property(card, "modulate:a", 1.0, 0.8)
	tween.tween_interval(seconds * PACE - 1.6)
	tween.tween_property(card, "modulate:a", 0.0, 0.8)
	await tween.finished
	card.visible = false


func _build_ui() -> void:
	ui = CanvasLayer.new()
	ui.layer = 10
	add_child(ui)

	subtitle_panel = PanelContainer.new()
	var panel_style := StyleBoxFlat.new()
	panel_style.bg_color = Color(0, 0, 0, 0.72)
	panel_style.set_corner_radius_all(10)
	panel_style.set_content_margin_all(18)
	subtitle_panel.add_theme_stylebox_override("panel", panel_style)
	subtitle_panel.set_anchors_and_offsets_preset(Control.PRESET_CENTER_BOTTOM)
	subtitle_panel.anchor_left = 0.1
	subtitle_panel.anchor_right = 0.9
	subtitle_panel.offset_left = 0
	subtitle_panel.offset_right = 0
	subtitle_panel.offset_top = -140
	subtitle_panel.offset_bottom = -40
	subtitle_panel.visible = false
	ui.add_child(subtitle_panel)

	subtitle_label = _label(30, HORIZONTAL_ALIGNMENT_CENTER)
	subtitle_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	subtitle_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	subtitle_panel.add_child(subtitle_label)

	chapter_label = _label(34, HORIZONTAL_ALIGNMENT_LEFT)
	chapter_label.position = Vector2(40, 28)
	ui.add_child(chapter_label)

	badges_label = _label(24, HORIZONTAL_ALIGNMENT_RIGHT)
	badges_label.set_anchors_and_offsets_preset(Control.PRESET_TOP_RIGHT)
	badges_label.offset_left = -500
	badges_label.offset_right = -40
	badges_label.offset_top = 32
	badges_label.add_theme_color_override("font_color", Color(1, 0.85, 0.35))
	ui.add_child(badges_label)

	card = ColorRect.new()
	card.color = Color(0.09, 0.1, 0.13)
	card.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	card.visible = false
	ui.add_child(card)

	comparison_backdrop = ColorRect.new()
	comparison_backdrop.color = Color(0.05, 0.06, 0.08, 0.92)
	comparison_backdrop.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	comparison_backdrop.visible = false
	ui.add_child(comparison_backdrop)
	ui.move_child(comparison_backdrop, 0)
	comparison = HBoxContainer.new()
	comparison.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	comparison.offset_top = 110
	comparison.offset_bottom = -170
	comparison.alignment = BoxContainer.ALIGNMENT_CENTER
	comparison.add_theme_constant_override("separation", 48)
	comparison_backdrop.add_child(comparison)
	for i in 2:
		var column := VBoxContainer.new()
		column.alignment = BoxContainer.ALIGNMENT_CENTER
		var texture := TextureRect.new()
		texture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		texture.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
		texture.custom_minimum_size = Vector2(760, 560)
		column.add_child(texture)
		column.add_child(_label(28, HORIZONTAL_ALIGNMENT_CENTER))
		comparison.add_child(column)

	var card_content := VBoxContainer.new()
	card_content.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	card_content.alignment = BoxContainer.ALIGNMENT_CENTER
	card.add_child(card_content)
	card_title = _label(72, HORIZONTAL_ALIGNMENT_CENTER)
	card_content.add_child(card_title)
	card_subtitle = _label(32, HORIZONTAL_ALIGNMENT_CENTER)
	card_subtitle.add_theme_color_override("font_color", Color(0.75, 0.8, 0.9))
	card_content.add_child(card_subtitle)


func _label(font_size: int, alignment: HorizontalAlignment) -> Label:
	var label := Label.new()
	label.horizontal_alignment = alignment
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_constant_override("outline_size", 6)
	label.add_theme_color_override("font_outline_color", Color(0, 0, 0, 0.85))
	return label
