extends ImmediateGeometry

const Colors = preload("Colors.gd")

func _ready():
	var m = SpatialMaterial.new()
	m.flags_use_point_size = true
	m.params_point_size = 3
	m.vertex_color_use_as_albedo = true # <-- THIS
	set_material_override(m)
	
