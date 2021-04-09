extends ImmediateGeometry

func _on_Game_points(data):
	clear()

	# Begin draw.
	begin(Mesh.PRIMITIVE_POINTS)
	for point in data:
		add_vertex(point);
	end()
