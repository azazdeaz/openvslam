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
			print("Mouse Click/Unclick at: ", event.position, " shape:", shape_idx)
			var t_frames = get_node("/root/Game/Spatial/Frames").transform
			print("frame transform:",t_frames)
			click_position = Vector3(click_position)
			print(click_position)
#			click_position = Vector3(click_position).rotated(Vector3.FORWARD, PI)
			click_position = t_frames * click_position
			print(click_position)
			
			get_node("/root/Game/Spatial/Frames/Goal").transform.origin = click_position
			print(get_node("/root/Game").set_target(
				click_position.x,
				click_position.y,
				click_position.z
			))
			
			var mark = CSGCylinder.new()
			mark.radius = 0.3
			mark.height = 0.1
			mark.sides = 20
			mark.translation = click_position
			get_node("/root/Game/Spatial/Frames").add_child(mark)
