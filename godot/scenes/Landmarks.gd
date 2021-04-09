extends ImmediateGeometry


func _ready():
	var m = SpatialMaterial.new()
	m.flags_use_point_size = true
#	m.params_point_size = point_size
	m.vertex_color_use_as_albedo = true # <-- THIS
	set_material_override(m)
	
func _on_Game_points(data):
	clear()

	# Begin draw.
	begin(Mesh.PRIMITIVE_POINTS)
	
	set_color(Color.tomato)
	for point in data:
		add_vertex(point);
	end()
