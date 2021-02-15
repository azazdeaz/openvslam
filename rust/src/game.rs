use gdnative::api::*;
use gdnative::prelude::*;

/// The Game "class"
#[derive(NativeClass)]
#[inherit(Node)]
#[register_with(Self::register_builder)]
pub struct Game {
    name: String,
}

use std::thread::spawn;
use std::{thread, time};
use tungstenite::{connect, Message, Error};
use url::Url;

// __One__ `impl` block can have the `#[methods]` attribute, which will generate
// code to automatically bind any exported methods to Godot.
#[methods]
impl Game {
    // Register the builder for methods, properties and/or signals.
    fn register_builder(_builder: &ClassBuilder<Self>) {
        godot_print!("Game builder is registered!");
    }

    /// The "constructor" of the class.
    fn new(_owner: &Node) -> Self {
        godot_print!("Game is created!");
        Game {
            name: "".to_string(),
        }
    }

    // In order to make a method known to Godot, the #[export] attribute has to be used.
    // In Godot script-classes do not actually inherit the parent class.
    // Instead they are "attached" to the parent object, called the "owner".
    // The owner is passed to every single exposed method.
    #[export]
    unsafe fn _ready(&mut self, _owner: &Node) {
        // The `godot_print!` macro works like `println!` but prints to the Godot-editor
        // output tab as well.
        self.name = "Game".to_string();
        godot_print!("{} is ready!", self.name);
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
                                println!("Received: {}", msg);
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
                                println!("READ MESSAGE: {:?}", e);
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
        // godot_print!("Inside {} _process(), delta is {}", self.name, delta);
    }
}
