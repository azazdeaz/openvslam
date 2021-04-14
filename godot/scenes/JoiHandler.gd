extends Spatial

var left = 0.0
var right = 0.0
var left_reversed = false
var right_reversed = false

func report():
	var l = -left if left_reversed else left
	var r = -right if right_reversed else right
	get_node("/root/Game").set_speed(l, r)

var speed_go = 0.4
var speed_turn = 0.6
var step_time = 0.12

func _input(event):
#	print(event.as_text())
	var game = get_node("/root/Game")
	
	if event is InputEventJoypadMotion:
		if event.axis == 6:
			left = event.axis_value
			report()
		if event.axis == 7:
			right = event.axis_value
			report()
			
	elif event is InputEventJoypadButton:
		if event.button_index == 4:
			left_reversed = event.pressed
			report()
		elif event.button_index == 5:
			right_reversed = event.pressed
			report()
		elif event.button_index == 11:
			game.set_follow_target(event.pressed)
		
		elif event.pressed:
			if event.button_index == 12:
				game.set_step(speed_go, speed_go, step_time)
			if event.button_index == 13:
				game.set_step(-speed_go, -speed_go, step_time)
			if event.button_index == 14:
				game.set_step(-speed_turn, speed_turn, step_time)
			if event.button_index == 15:
				game.set_step(speed_turn, -speed_turn, step_time)
			if event.button_index == 1:
				game.set_step(0.0, 0.0, 0.0)
		
