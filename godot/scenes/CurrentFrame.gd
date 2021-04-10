extends ImmediateGeometry
const Colors = preload("Colors.gd")

#func _ready():
#	var m = SpatialMaterial.new()
#	m.flags_use_point_size = true
#	m.params_line_width = 3
#	m.vertex_color_use_as_albedo = true # <-- THIS
#	set_material_override(m)

func _on_Game_current_frame(data):
	clear()
	print("CF", data)
	begin(Mesh.PRIMITIVE_LINE_STRIP)
	set_color(Colors.CURRENT_FRAME)
	for v in data:
		add_vertex(v);
	end()
