extends CanvasLayer

var debug: bool = false;
var sync_mode: bool = false;

@onready var fps_label : Node = get_node("%FPS")
@onready var debug_label : Node = get_node("%Debug")
@onready var sync_label : Node = get_node("%Sync")
@onready var player_label : Node = get_node("%PlayerPosition")

func _ready():
	debug_label.text = "Debug: " + str(debug)
	sync_label.text = "Sync mode: " + str(sync_mode)

func _process(_delta: float):
	fps_label.text = "FPS: " + str(Engine.get_frames_per_second())
	player_label.text = "Player Position: " + str(get_node("%Character").get_local_position_3d())
	if Input.is_action_just_released("set_debug"):
		debug = !debug
		IsometricServer.set_debug(debug)
		debug_label.text = "Debug: " + str(debug)
	if Input.is_action_just_released("set_sync"):
		sync_mode = !sync_mode
		IsometricServer.set_synchronous_mode(sync_mode)
		sync_label.text = "Sync mode: " + str(sync_mode)

