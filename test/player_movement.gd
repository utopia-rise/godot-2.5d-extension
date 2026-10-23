extends CharacterBody3D


var linear_velocity: Vector3

@export var gravity: float = ProjectSettings.get_setting("physics/2d/default_gravity")
@export var speed: int = 1


func _physics_process(delta):
	linear_velocity += Vector3(0, -gravity * delta, 0)
	var horizontal_velocity: Vector3 = Vector3()
	var vertical_velocity: Vector3 = Vector3(0, linear_velocity.y, 0)

	if Input.is_action_pressed("player_forward"):
		horizontal_velocity += Vector3(-1, 0, -1).normalized() * speed
	if Input.is_action_pressed("player_backward"):
		horizontal_velocity += Vector3(1, 0, 1).normalized() * speed
	if Input.is_action_pressed("player_left"):
		horizontal_velocity += Vector3(-1, 0, 1).normalized() * speed
	if Input.is_action_pressed("player_right"):
		horizontal_velocity += Vector3(1, 0, -1).normalized() * speed


	linear_velocity = horizontal_velocity + vertical_velocity
	set_velocity(linear_velocity)
	set_up_direction(Vector3(0, 1, 0))
	move_and_slide()
	linear_velocity = velocity
