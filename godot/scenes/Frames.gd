extends Spatial
var rng = RandomNumberGenerator.new()

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
func rp():
	return rng.randf_range(-100.0, 100.0)
	
var pts = [Vector3(0,0,0), Vector3(0,0,0)]
var im = ImmediateGeometry.new()

# Called when the node enters the scene tree for the first time.
func _ready():
	rng.randomize();
	var point_size = 5
	
	add_child(im)
	var m = SpatialMaterial.new()
	m.flags_use_point_size = true
	m.params_point_size = point_size
	im.set_material_override(m)
	im.clear()
	im.begin(Mesh.PRIMITIVE_POINTS, null)
	var pts = [Vector3(0,0,0), Vector3(0,0,0)]
	for p in range(10): #list of Vector3s
		im.add_vertex(Vector3(rp(), rp(), rp()))
	im.end()

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
##	im.clear()
#	im.begin(Mesh.PRIMITIVE_POINTS, null)
#	var pts = [Vector3(0,0,0), Vector3(0,0,0)]
#	for p in range(100): #list of Vector3s
#		im.add_vertex(Vector3(rp(), rp(), rp()))
#	im.end()




func _on_Game_points(points):
	im.clear()
	im.begin(Mesh.PRIMITIVE_POINTS, null)
	for point in points:
		im.add_vertex(point)
	im.end()
