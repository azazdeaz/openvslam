extends Button


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func _on_Button_pressed():
	var panel = get_node("../Cam")
	if panel.rect_scale[0] != 1:
		panel.rect_scale = Vector2(1,1)
	else:
		panel.rect_scale = Vector2(.3,.3)
