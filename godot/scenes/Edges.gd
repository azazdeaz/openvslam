extends ImmediateGeometry


func _on_Game_edges(data):
	clear()

	# Begin draw.
	begin(Mesh.PRIMITIVE_LINES)
	for edge in data:
		add_vertex(edge[0]);
		add_vertex(edge[1]);
	end()
