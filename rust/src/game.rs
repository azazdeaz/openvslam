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
    rx: Option<Receiver<items::VSlamMap>>,
}

use prost::Message;
pub mod items {
    include!(concat!(env!("OUT_DIR"), "/map_segment.rs"));
}
use std::collections::HashMap;

struct Landmark {
    id: u32,
    x: f64,
    y: f64,
    z: f64,
}

struct Values {
    position: Vec<f32>,
    points: Vec<f32>,
    landmarks: HashMap<u32, Landmark>,
    keyframes: HashMap<u32, TypedArray<Vector3>>,
}

use std::sync::mpsc;
use std::sync::mpsc::{Receiver, Sender};
use std::thread::spawn;
use std::{
    sync::{Arc, Mutex},
    thread, time,
};
// use tungstenite::{connect, Error, Message};
use url::Url;
use zmq;

extern crate nalgebra as na;

use ndarray::prelude::*;
use ndarray::{Array, Axis, stack, OwnedRepr};

// #[path = "protos/map_segment.rs"]
// mod map_segment;
// use map_segment::Map;
// use protobuf;


fn keyframe_vertices() -> ArrayBase<OwnedRepr<f64>, ndarray::Dim<[usize; 2]>> {
    let f = 1.0;
    let cx = 2.0;
    let cy = 1.0;
    // let c = na::Vector3::new(0.0,0.0,0.0);
    // let tl = na::Vector3::new(-cx,cy,f);
    // let tr = na::Vector3::new(cx,cy,f);
    // let br = na::Vector3::new(cx,-cy,f);
    // let bl = na::Vector3::new(-cx,-cy,f);
    let c = array![0.0,0.0,0.0];
    let tl = array![-cx,cy,f];
    let tr = array![cx,cy,f];
    let br = array![cx,-cy,f];
    let bl = array![-cx,-cy,f];
    stack![Axis(1), c,tl,tr,c,tr,br,c,br,bl,c,bl,tl]

    // na::Matrix3xX::from_columns(&[c,tl,tr,c,tr,br,c,br,bl,c,bl,tl])  
}

// __One__ `impl` block can have the `#[methods]` attribute, which will generate
// code to automatically bind any exported methods to Godot.
#[methods]
impl Game {
    // Register the builder for methods, properties and/or signals.
    fn register_builder(_builder: &ClassBuilder<Self>) {
        let x = env!("OUT_DIR");
        godot_print!("Game builder is registered!");
    }

    fn register_signals(builder: &ClassBuilder<Self>) {
        builder.add_signal(Signal {
            name: "tick",
            args: &[],
        });

        builder.add_signal(Signal {
            name: "dry_protobuf",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_str(""),
                export_info: ExportInfo::new(VariantType::ByteArray),
                usage: PropertyUsage::DEFAULT,
            }],
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

        builder.add_signal(Signal {
            name: "position",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_vector3(&Vector3::new(0., 0., 0.)),
                export_info: ExportInfo::new(VariantType::Vector3),
                usage: PropertyUsage::DEFAULT,
            }],
        });

        builder.add_signal(Signal {
            name: "points",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_vector3_array(&TypedArray::default()),
                export_info: ExportInfo::new(VariantType::Vector3Array),
                usage: PropertyUsage::DEFAULT,
            }],
        });

        builder.add_signal(Signal {
            name: "keyframe_vertices",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_array(&VariantArray::new_shared()),
                export_info: ExportInfo::new(VariantType::Vector3Array),
                usage: PropertyUsage::DEFAULT,
            }],
        });


        builder.add_signal(Signal {
            name: "edges",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_array(&VariantArray::new_shared()),
                export_info: ExportInfo::new(VariantType::Vector3Array),
                usage: PropertyUsage::DEFAULT,
            }],
        });

        builder.add_signal(Signal {
            name: "message",
            // Argument list used by the editor for GUI and generation of GDScript handlers. It can be omitted if the signal is only used from code.
            args: &[SignalArgument {
                name: "data",
                default: Variant::from_str(""),
                export_info: ExportInfo::new(VariantType::GodotString),
                usage: PropertyUsage::DEFAULT,
            }],
        });
    }

    /// The "constructor" of the class.
    fn new(_owner: &Node) -> Self {
        godot_print!("Game is created!");
        Game {
            name: "".to_string(),
            values: Values {
                position: vec![0., 1., 2.],
                points: vec![1., 2., 3.],
                landmarks: HashMap::new(),
                keyframes: HashMap::new(),
            },
            rx: None,
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
        godot_print!("{} is ready!!!mak", self.name);
        
        let context = zmq::Context::new();


        let owner = Arc::new(Mutex::new(_owner.as_ref()));
        let o = Arc::clone(&owner);

        let (tx, rx): (Sender<items::VSlamMap>, Receiver<items::VSlamMap>) = mpsc::channel();
        self.rx = Some(rx);
        let thread_tx = tx.clone();
        spawn(move || {
            loop {
                println!("Connecting...");
                // let connection = connect(Url::parse("ws://localhost:3012/socket").unwrap());
                let subscriber = context.socket(zmq::SUB).unwrap();
                subscriber
                    .connect("tcp://127.0.0.1:5566")
                    .expect("failed connecting subscriber");
                subscriber.set_subscribe(b"").expect("failed subscribing");

                loop {
                    let envelope = subscriber
                        .recv_string(0)
                        .expect("failed receiving envelope")
                        .unwrap();
                    let message = subscriber
                        .recv_bytes(0)
                        .expect("failed receiving message");
                        // println!("{:?}", message);
                    let message = ::base64::decode(message).unwrap();
                    let msg = items::VSlamMap::decode(&mut std::io::Cursor::new(message)).unwrap();
                    
                    // println!("{:?}", msg);
                    // let m = Map::parse_from_bytes(&message);
                    // m.merge_from(CodedInputStream::from_bytes(&message));
                    // println!("{:?}", m);
                    // let msg = format!("[{}] {}", envelope, message);
                    thread_tx.send(msg).unwrap();
                }

                // if let Ok((mut socket, response)) = connection {
                //     println!("Connected to the server");
                //     println!("Response HTTP code: {}", response.status());
                //     println!("Response contains the following headers:");
                //     for (ref header, _value) in response.headers() {
                //         println!("* {}", header);
                //     }

                //     socket
                //         .write_message(Message::Text("Hello WebSocket".into()))
                //         .unwrap();
                //     loop {
                //         match socket.read_message() {
                //             Ok(msg) => match msg {
                //                 Message::Text(msg) => {
                //                     let msg = json::parse(&msg).unwrap();
                //                     println!("Received: {}", msg["type"]);
                //                     thread_tx.send(msg).unwrap();
                //                 }
                //                 _ => println!("not a text message {}", msg),
                //             },
                //             Err(e) => {
                //                 println!("READ ERR {:?} {}", e, e);
                //                 match e {
                //                     Error::AlreadyClosed
                //                     | Error::ConnectionClosed
                //                     | Error::Io(_)
                //                     | Error::Protocol(_) => {
                //                         println!("Connection closed. Reconnect...");
                //                         break; // reconnect
                //                     }
                //                     _ => {
                //                         println!("Unknown error");
                //                         panic!(e);
                //                     }
                //                 }
                //             }
                //         }
                //     }
                // } else if let Err(err) = connection {
                //     println!("{:?}\nwait...", err);
                //     thread::sleep(time::Duration::from_secs(2));
                // }
            }
        });
    }

    // This function will be called in every frame
    #[export]
    unsafe fn _process(&mut self, _owner: &Node, delta: f64) {

        

        
        // let mut vectors: TypedArray<Vector3> = TypedArray::default();
        // for x in -20..=20 {
        //     for y in -20..=20 {
        //         for z in -20..=20 {
        //             vectors.push(Vector3::new(x as f32, y as f32, z as f32));
        //         }
        //     } 
        // }
        // println!("processed");
        
        // _owner.emit_signal(
        //     "points",
        //     &[Variant::from_vector3_array(&vectors)],
        // );
        
        let mut edges = None;
        if let Some(rx) = &self.rx {
            _owner.emit_signal("tick", &[]);
            
            while let Ok(msg) = rx.try_recv() {
                for landmark in msg.landmarks.iter() {
                    if landmark.coords.len() != 0 {
                        self.values.landmarks.insert(landmark.id, Landmark {
                            id: landmark.id,
                            x: landmark.coords[0],
                            y: landmark.coords[1],
                            z: landmark.coords[2],
                        });
                    }
                    else {
                        self.values.landmarks.remove(&landmark.id);
                    }
                    // println!("landmark {:?}", landmark.color);
                    // vectors.push(landmark.coords);
                }
                

                for message in msg.messages.iter() {
                    let text = format!("[{}]: {}", message.tag, message.txt);
                    println!("{}", text);
                    _owner.emit_signal(
                        "message",
                        &[Variant::from_str(text)],
                    );
                }

                for keyframe in msg.keyframes.iter() {
                    println!("keyframe {} {:?}", keyframe.id, keyframe.pose);
                    if let Some(pose) = &keyframe.pose {
                        let pose = pose.pose.to_vec();
                        let pose = Array::from_vec(pose).into_shape((4,4)).unwrap();
                        let rotation = pose.slice(s![0..3,0..3]);
                        let translation = pose.slice(s![0..3,3..4]);

                        let vertices = rotation.dot(&keyframe_vertices())+translation; 

                        let mut vectors: TypedArray<Vector3> = TypedArray::default();
                        for v in vertices.axis_iter(Axis(1)) {
                            vectors.push(Vector3::new(v[0] as f32,v[1] as f32,v[2] as f32));
                        }

                        self.values.keyframes.insert(keyframe.id, vectors);
                    }
                    else {
                        self.values.keyframes.remove(&keyframe.id);
                    }
                }

                edges = Some(msg.edges);

                // _owner.emit_signal("dry_protobuf", &[msg.to_variant()]);
                // println!("MSG IS {}", msg);
                // if msg["type"] == "position" {
                //     _owner.emit_signal(
                //         "position",
                //         &[Variant::from_vector3(&Vector3::new(
                //             msg["x"].as_f32().unwrap(),
                //             msg["y"].as_f32().unwrap(),
                //             msg["z"].as_f32().unwrap(),
                //         ))],
                //     );
                // }
                // else if msg["type"] == "points" {
                //     let points: Vec<f32> = msg["points"].members().map(|n| n.as_f32().unwrap()).collect();
                //     let mut vectors: TypedArray<Vector3> = TypedArray::default();
                //     let point_count = points.len() / 3;
                //     for i in 0..point_count {
                //         vectors.push(Vector3::new(points[i], points[i+point_count], points[i+point_count*2]))
                //     }
                //     _owner.emit_signal(
                //         "points",
                //         &[Variant::from_vector3_array(&vectors)],
                //     );
                // }
                // _owner.emit_signal("tick_with_data", &[Variant::from_i64(x as i64)]);
            }
        }

        let data = self.values.keyframes.values().cloned().collect::<Vec<TypedArray<Vector3>>>().to_variant();
        _owner.emit_signal(
            "keyframe_vertices",
            &[data],
        );

        let mut vectors: TypedArray<Vector3> = TypedArray::default();
        for landmark in self.values.landmarks.values() {
            vectors.push(Vector3::new(landmark.x as f32, landmark.y as f32, landmark.z as f32));
        }
        
        _owner.emit_signal(
            "points",
            &[Variant::from_vector3_array(&vectors)],
        );
        
        if let Some(edges_) = edges {
            println!("{:?}", edges_);
            let lines = edges_.iter().filter_map(|e| {
                let k0 = self.values.keyframes.get(&e.id0);
                let k1 = self.values.keyframes.get(&e.id1);
                if let (Some(k0), Some(k1)) = (k0, k1) {
                    Some(vec![k0.get(0), k1.get(0)])
                }
                else {
                    None
                }
            }).collect::<Vec<Vec<Vector3>>>();

            _owner.emit_signal(
                "edges",
                &[lines.to_variant()],
            );
        }
        // godot_print!("Inside {} _process(), delta is {} {}", self.name, delta, self.values.position[0]);
    }
}
