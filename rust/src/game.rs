use gdnative::api::*;
use gdnative::prelude::*;

/// The Game "class"
#[derive(NativeClass)]
#[inherit(Node)]
// #[register_with(Self::register_builder)]
#[register_with(Self::register_signals)]
pub struct Game {
    name: String,
    values: Values,
}

struct Values {
    position: Vec<f32>,
    points: Vec<f32>,
}

use std::thread::spawn;
use std::{thread, time, sync::{Arc, Mutex}};
use tungstenite::{connect, Message, Error};
use url::Url;
use ::json;

// __One__ `impl` block can have the `#[methods]` attribute, which will generate
// code to automatically bind any exported methods to Godot.
#[methods]
impl Game {
    // Register the builder for methods, properties and/or signals.
    fn register_builder(_builder: &ClassBuilder<Self>) {
        godot_print!("Game builder is registered!");
    }

    fn register_signals(builder: &ClassBuilder<Self>) {
        builder.add_signal(Signal {
            name: "tick",
            args: &[],
        });

        builder.add_signal(Signal {
            name: "tick_with_data",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_i64(100),
                export_info: ExportInfo::new(VariantType::I64),
                usage: PropertyUsage::DEFAULT,
            }],
        });
    }


    /// The "constructor" of the class.
    fn new(_owner: &Node) -> Self {
        godot_print!("Game is created!");
        Game {
            name: "".to_string(),
            values: Values { position: vec![0.,1.,2.], points: vec![1.,2.,3.]}
        }
    }

    // In order to make a method known to Godot, the #[export] attribute has to be used.
    // In Godot script-classes do not actually inherit the parent class.
    // Instead they are "attached" to the parent object, called the "owner".
    // The owner is passed to every single exposed method.
    #[export]
    unsafe fn _ready(&mut self, _owner: TRef<Node>) {
        // The `godot_print!` macro works like `println!` but prints to the Godot-editor
        // output tab as well.
        self.name = "Game".to_string();
        godot_print!("{} is ready!", self.name);
        // let ref_im = ImmediateGeometry::new();
        // _owner.add_child(ref_im, true);
        // let ref_m = SpatialMaterial::new();
        // ref_m.as_ref().set_flag(SpatialMaterial::FLAG_USE_POINT_SIZE, true);
        // ref_m.as_ref().set_point_size(5.);
        // ref_im.as_ref().set_material_override(ref_m);
        // ref_im.as_ref().clear();
        // ref_im.as_ref().begin(Mesh::PRIMITIVE_POINTS, AsArg);
        let owner = Arc::new(Mutex::new(_owner.as_ref()));
        let o = Arc::clone(&owner);
        
        spawn(move || {
            loop {
                println!("Connecting...");
                let connection = connect(Url::parse("ws://localhost:3012/socket").unwrap());
                if let Ok((mut socket, response)) = connection {
                    println!("Connected to the server");
                    println!("Response HTTP code: {}", response.status());
                    println!("Response contains the following headers:");
                    for (ref header, _value) in response.headers() {
                        println!("* {}", header);
                    }
                    
                    socket.write_message(Message::Text("Hello WebSocket".into())).unwrap();
                    loop {
                        match socket.read_message() {
                            Ok(msg) => {
                                match msg {
                                    Message::Text(msg) => {
                                        let msg = json::parse(&msg).unwrap();
                                        println!("Received: {}", msg["type"]);
                                        if msg["type"] == "position" {
                                            let x: f32 = msg["x"].as_f32().unwrap();
                                            let y: f32 = msg["y"].as_f32().unwrap();
                                            let z: f32 = msg["z"].as_f32().unwrap();
                                            let mut o2 = o.lock().unwrap();
                                            // println!("set x to {}", x);
                                            // o2.values.position = vec![x,y,z];
                                            
                                            // _owner.as_ref().emit_signal("tick", &[]);
                                            // _owner.emit_signal("tick_with_data", &[Variant::from_i64(x as i64)]);
                                        }
                                    }
                                    _ => println!("not a text message {}", msg)
                                }
                            },
                            Err(e) => {
                                println!("READ ERR {:?} {}", e, e);
                                match e {
                                    Error::AlreadyClosed | Error::ConnectionClosed | Error::Io(_) | Error::Protocol(_) => {
                                        println!("Connection closed. Reconnect...");
                                        break; // reconnect
                                    }
                                    _ => {
                                        println!("Unknown error");
                                        panic!(e);
                                    }
                                }
                            }
                        }
                    }
                }
                else if let Err(err) = connection {
                    println!("{:?}\nwait...", err);
                    thread::sleep(time::Duration::from_secs(2));
                }
            }
        });
    }

    // This function will be called in every frame
    #[export]
    unsafe fn _process(&self, _owner: &Node, delta: f64) {
        _owner.emit_signal("tick", &[]);
        _owner.emit_signal("tick_with_data", &[Variant::from_i64(self.values.position[0] as i64)]);
        godot_print!("Inside {} _process(), delta is {} {}", self.name, delta, self.values.position[0]);
    }
}
