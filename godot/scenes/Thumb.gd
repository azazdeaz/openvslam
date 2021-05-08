extends Panel

func _input_event(camera, event, click_position, click_normal, shape_idx):
	print(event)
	if event is InputEventMouseButton:
		if event.button_index == BUTTON_LEFT and event.doubleclick:
			print(rect_scale)
