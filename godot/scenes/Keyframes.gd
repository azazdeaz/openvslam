extends ImmediateGeometry
const Colors = preload("Colors.gd")

func _ready():
	var m = SpatialMaterial.new()
	m.flags_use_point_size = true
#	m.params_point_size = point_size
	m.vertex_color_use_as_albedo = true # <-- THIS
	set_material_override(m)

func _on_Game_keyframe_vertices(data):
	clear()

	# Begin draw.
	for frame in data:
		begin(Mesh.PRIMITIVE_LINE_STRIP)
		set_color(Colors.FRAME)
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
	
