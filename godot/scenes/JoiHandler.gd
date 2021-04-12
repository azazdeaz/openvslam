extends Spatial

var left = 0.0
var right = 0.0
var left_reversed = false
var right_reversed = false

func report():
	var l = -left if left_reversed else left
	var r = -right if right_reversed else right
	get_node("/root/Game").set_speed(l, r)


func _input(event):
	
	if event is InputEventJoypadMotion:
		if event.axis == 6:
			left = event.axis_value
		if event.axis == 7:
			right = event.axis_value
		report()
			
	elif event is InputEventJoypadButton:
		if event.button_index == 4:
			left_reversed = event.pressed
		if event.button_index == 5:
			right_reversed = event.pressed
		report()
