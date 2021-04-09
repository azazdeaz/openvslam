extends ImmediateGeometry

#func _process(_delta):
#	# Clean up before drawing.
#	clear()
#
#	# Begin draw.
#	begin(Mesh.PRIMITIVE_LINE_LOOP)
#
#	# Prepare attributes for add_vertex.
#	var f = 1
#	var cx = 2
#	var cy = 1
##	set_normal(Vector3(0, 0, 0))
##	set_uv(Vector2(0, 0))
#	# Call last for each vertex, adds the above attributes.
#	add_vertex(Vector3(0,0,0))
#	add_vertex(Vector3(-cx, cy, f))
#	add_vertex(Vector3(cx, cy, f))
#	add_vertex(Vector3(cx, -cy, f))
#	add_vertex(Vector3(-cx, -cy, f))
#	add_vertex(Vector3(-cx, cy, f))
#
##	set_normal(Vector3(0, 0, 1))
##	set_uv(Vector2(0, 1))
##	add_vertex(Vector3(-1, 1, 0))
#
##	set_normal(Vector3(0, 0, 1))
##	set_uv(Vector2(1, 1))
##	add_vertex(Vector3(1, 1, 0))
#
#	# End drawing.
#	end()



func _on_Game_keyframe_vertices(data):
	clear()

	# Begin draw.
	for frame in data:
		begin(Mesh.PRIMITIVE_LINE_STRIP)
		for v in frame:
			add_vertex(v);
		end()

	# Prepare attributes for add_vertex.
#	var f = 1
#	var cx = 2
#	var cy = 1
##	set_normal(Vector3(0, 0, 0))
##	set_uv(Vector2(0, 0))
#	# Call last for each vertex, adds the above attributes.
#	add_vertex(Vector3(0,0,0))
#	add_vertex(Vector3(-cx, cy, f))
#	add_vertex(Vector3(cx, cy, f))
#	add_vertex(Vector3(cx, -cy, f))
#	add_vertex(Vector3(-cx, -cy, f))
#	add_vertex(Vector3(-cx, cy, f))

#	set_normal(Vector3(0, 0, 1))
#	set_uv(Vector2(0, 1))
#	add_vertex(Vector3(-1, 1, 0))

#	set_normal(Vector3(0, 0, 1))
#	set_uv(Vector2(1, 1))
#	add_vertex(Vector3(1, 1, 0))

	# End drawing.
	
