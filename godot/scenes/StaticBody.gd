extends StaticBody


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
func _input_event(camera, event, click_position, click_normal, shape_idx):
	if event is InputEventMouseButton:
		if event.button_index == BUTTON_LEFT and event.doubleclick:
			get_node("/root/Game/Spatial/Goal").transform.origin = click_position
			print("Mouse Click/Unclick at: ", event.position, " shape:", shape_idx)
			print(get_node("/root/Game").set_target(click_position[0],click_position[1],click_position[2]))
