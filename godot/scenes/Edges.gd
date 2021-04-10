extends ImmediateGeometry
const Colors = preload("Colors.gd")

func _ready():
	var m = SpatialMaterial.new()
	m.flags_use_point_size = true
#	m.params_point_size = point_size
	m.vertex_color_use_as_albedo = true # <-- THIS
	set_material_override(m)


func _on_Game_edges(data):
	clear()

	# Begin draw.
	begin(Mesh.PRIMITIVE_LINES)
	
	set_color(Colors.EDGE)
	for edge in data:
		add_vertex(edge[0]);
		add_vertex(edge[1]);
	end()
