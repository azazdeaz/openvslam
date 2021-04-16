extends ImmediateGeometry
const Colors = preload("Colors.gd")

#func _ready():
#	var m = SpatialMaterial.new()
#	m.flags_use_point_size = true
#	m.params_line_width = 3
#	m.vertex_color_use_as_albedo = true # <-- THIS
#	set_material_override(m)
var rng = RandomNumberGenerator.new()

func noise():
	return rng.randf() * 0.015

func _on_Game_current_frame(data):
	rng.seed = 1
	clear()
	begin(Mesh.PRIMITIVE_LINE_STRIP)
	set_color(Colors.CURRENT_FRAME)
	# make lines thicker
	for i in range(6):
		for v in data:
			v = v + Vector3(noise(), noise(), noise())
			add_vertex(v)
	end()
