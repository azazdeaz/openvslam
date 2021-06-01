use crossbeam_channel::{unbounded, Sender, tick, select};
use nalgebra as na;
use zmq;
use std::thread;
use std::time::{Instant, Duration};

type Iso3 = na::Isometry3<f64>;


fn angle_difference(bearing_from: f64, bearing_to: f64) -> f64 {
    let pi = std::f64::consts::PI;
    let diff = bearing_to - bearing_from;

    if diff > pi {
        diff - pi*2.0
    }
    else if diff < -pi {
        diff + pi*2.0
    }
    else {
        diff
    }
}
// fn test() {
//     let pi = std::f64::consts::PI;
//     println!("{} {}", angle_difference(pi-0.1, -pi+0.1), 0.2);
//     println!("{} {}", angle_difference(-0.1, 0.1), 0.2);
//     println!("{} {}", angle_difference(-pi, pi), 0.0);
//     println!("{} {}", angle_difference(pi, -pi), 0.0);
//     println!("{} {}", angle_difference(-pi, pi-0.1), -0.1);
// }


struct NavState {
    speed: (f64, f64),
    teleop_speed: ((f64, f64), Instant),
    cam_pose: (Iso3, Instant),
    target_pose: Option<Iso3>,
    self_drive_enabled: bool,
}

impl NavState {
    fn new() -> Self {
        NavState {
            speed: (0.0, 0.0),
            teleop_speed: ((0.0, 0.0), Instant::now()),
            cam_pose: (Iso3::identity(), Instant::now()),
            target_pose: None,
            self_drive_enabled: false,
        }
    }

    fn set_teleop_speed(&mut self, speed: (f64, f64)) {
        self.teleop_speed = (speed, Instant::now());
    }

    fn set_cam_pose(&mut self, cam_pose: Iso3) {
        self.cam_pose = (cam_pose, Instant::now());
    }

    fn set_target_pose(&mut self, target_pose: Iso3) {
        self.target_pose = Some(target_pose);
    }

    fn set_self_drive_enabled(&mut self, enable: bool) {
        self.self_drive_enabled = enable;
    }

    fn is_expired(time: Instant) -> bool {
        println!(" {:?} {:?} ", time.checked_add(Duration::from_millis(600)).unwrap() , Instant::now());
        time.checked_add(Duration::from_millis(600)).unwrap() < Instant::now().
    }

    fn step(&mut self) {
        self.speed = if !self.self_drive_enabled {
            if NavState::is_expired(self.teleop_speed.1) {
                (0.0, 0.0)
            }
            else {
                self.teleop_speed.0
            }
        }
        else if NavState::is_expired(self.cam_pose.1) {
            (0.0, 0.0)
        }
        else if let Some(target_pose) = self.target_pose {
            
            let pose = self.cam_pose.0;
            let speed_go = 0.2;
            let speed_turn = 0.2;


            let p = na::Point3::new(0.0, 0.0, 1.0);
            let p = pose.rotation * p;
            let yaw_bot = p.x.atan2(p.z);

            let dx = target_pose.translation.vector.x - pose.translation.vector.x;
            let dz = target_pose.translation.vector.z - pose.translation.vector.z;
            let yaw_target = dx.atan2(dz);
            let yawd = angle_difference(yaw_bot, yaw_target);
            let distance = dx.hypot(dz);

            println!(
                "from {:?} to {:?} is |{},{}|={}; yaw_target={} yaw_bot={} yawd={}",
                pose.translation.vector,
                target_pose.translation.vector,
                dx,dz,distance,
                yaw_target,
                yaw_bot,
                yawd
            );

            if distance < 1.0 {
                (0., 0.)
            } else if yawd.abs() < 0.3 {
                (speed_go, speed_go)
            } else if yawd > 0. {
                (speed_turn, -speed_turn)
            } else {
                (-speed_turn, speed_turn)
            }
        }
        else {
            (0.0, 0.0)
        }
    }
}


pub struct Navigator {
    pub send_cam_pose: Sender<Iso3>,
    pub send_target_pose: Sender<Iso3>,
    pub send_teleop_speed: Sender<(f64, f64)>,
    pub send_self_drive_enabled: Sender<bool>,
}

impl Navigator {
    pub fn new() -> Self {
        let (send_cam_pose, recv_cam_pose) = unbounded();
        let (send_target_pose, recv_target_pose) = unbounded();
        let (send_teleop_speed, recv_teleop_speed) = unbounded();
        let (send_self_drive_enabled, recv_self_drive_enabled) = unbounded();

        let context = zmq::Context::new();
        let publisher = context.socket(zmq::PUB).unwrap();
        publisher
            .bind("tcp://*:5567")
            .expect("failed binding publisher");

        let mut state = NavState::new();

        thread::spawn(move || {
            let ticker = tick(Duration::from_millis(100));
            loop {
                select! {
                    recv(recv_cam_pose) -> msg => if let Ok(msg) = msg { state.set_cam_pose(msg) },
                    recv(recv_target_pose) -> msg => if let Ok(msg) = msg { state.set_target_pose(msg) },
                    recv(recv_teleop_speed) -> msg => if let Ok(msg) = msg { state.set_teleop_speed(msg) },
                    recv(recv_self_drive_enabled) -> msg => if let Ok(msg) = msg { state.set_self_drive_enabled(msg) },
                    recv(ticker) -> _ => {
                        state.step();
                        let cmd = format!("{},{}", state.speed.0, state.speed.1);
                        publisher.send(&cmd, 0).expect("failed to send cmd");
                    },
                }
            }
        });

        Navigator {
            send_cam_pose,
            send_target_pose,
            send_teleop_speed,
            send_self_drive_enabled,
        }
    }
}