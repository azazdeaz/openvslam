extends Spatial

const MyProto = preload("res://MapProto.gd")
# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	pass


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func _on_Game_dry_protobuf(data):
	var m = MyProto.Map.new()
	print(m.from_bytes(data))
