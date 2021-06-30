
#include <future>
#include <zmq.hpp>
#include <zmq_addon.hpp>    

#include "openvslam/system.h"
#include "openvslam/config.h"
#include "openvslam/publish/map_publisher.h"
#include "openvslam/data/landmark.h"

#include <iostream>
#include <chrono>
#include <fstream>
#include <numeric>

#include <opencv2/core/core.hpp> 
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>
#include <popl.hpp>

#ifdef USE_STACK_TRACE_LOGGER
#include <glog/logging.h>
#endif

#ifdef USE_GOOGLE_PERFTOOLS
#include <gperftools/profiler.h>
#endif

#include "openvslam_api.pb.h"

// TODO try https://forums.developer.nvidia.com/t/optimizing-opencv-gstreamer-with-mipi-and-usb-uvc-cameras/123665/27
std::string gstreamer_pipeline (int capture_width, int capture_height, int framerate, int flip_method) {
    return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(capture_width) + ", height=(int)" +
           std::to_string(capture_height) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(framerate) +
           "/1 ! nvvidconv flip-method=" + std::to_string(flip_method) + " ! video/x-raw, width=(int)" + std::to_string(capture_width) + ", height=(int)" +
           std::to_string(capture_height) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink max-buffers=1 drop=true";
}

std::string usb_pipeline() {
    // return "v4l2src device=/dev/video0 ! video/x-raw,format=BGR,width=640,height=480,framerate=30/1 ! appsink";
    return "v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=30/1 ! videoconvert ! video/x-raw,format=BGR ! appsink max-buffers=1 drop=true";
}


int main(int argc, char* argv[]) {
    zmq::context_t ctx;

    zmq::socket_t sock_rep(ctx, zmq::socket_type::rep);
    // TODO pass adresses as parameters
    sock_rep.bind("ipc:///tmp/openvslam_wrapper_ipc_request");

    zmq::socket_t sock_stream(ctx, zmq::socket_type::push);
    int confl = 1;
    sock_stream.setsockopt(ZMQ_CONFLATE, &confl, sizeof(confl));
    sock_stream.bind("ipc:///tmp/openvslam_wrapper_ipc_stream");
    
    std::cout << "Listening..." << std::endl;

    // create options
    popl::OptionParser op("Allowed options");
    auto help = op.add<popl::Switch>("h", "help", "produce help message");
    auto vocab_file_path = op.add<popl::Value<std::string>>("v", "vocab", "vocabulary file path");
    auto config_file_path = op.add<popl::Value<std::string>>("c", "config", "config file path");
    auto mask_img_path = op.add<popl::Value<std::string>>("", "mask", "mask image path", "");
    auto map = op.add<popl::Value<std::string>>("", "map", "db map path", "");
    auto debug_mode = op.add<popl::Switch>("", "debug", "debug mode");
    auto video_file_path = op.add<popl::Value<std::string>>("m", "video", "video file path");
    auto eval_log = op.add<popl::Switch>("", "eval-log", "store trajectory and tracking times for evaluation");
    try {
        op.parse(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << std::endl;
        std::cerr << op << std::endl;
        return EXIT_FAILURE;
    }

    // check validness of options
    if (help->is_set()) {
        std::cerr << op << std::endl;
        return EXIT_FAILURE;
    }
    if (!vocab_file_path->is_set() || !config_file_path->is_set()) {
        std::cerr << "invalid arguments" << std::endl;
        std::cerr << std::endl;
        std::cerr << op << std::endl;
        return EXIT_FAILURE;
    }

    // setup logger
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%L] %v%$");
    if (debug_mode->is_set()) {
        spdlog::set_level(spdlog::level::debug);
    }
    else {
        spdlog::set_level(spdlog::level::info);
    }

    // load configuration
    std::shared_ptr<openvslam::config> cfg;
    try {
        cfg = std::make_shared<openvslam::config>(config_file_path->value());
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    openvslam::system SLAM(cfg, vocab_file_path->value());
    SLAM.startup();
    const cv::Mat mask = mask_img_path->value().empty() ? cv::Mat{} : cv::imread(mask_img_path->value(), cv::IMREAD_GRAYSCALE);

    if (!map->value().empty()) {
        std::cout << "loading map..." << std::endl;
        SLAM.load_map_database(map->value());
    }


    int capture_width = 640 ;
    int capture_height = 480 ;
    int framerate = 4 ;
    int flip_method = 0 ;

    bool stream_pose = true;
    bool stream_landmarks = true;
    bool stream_keyframes = true;

    std::string pipeline = gstreamer_pipeline(capture_width, capture_height, framerate, flip_method);
    pipeline = usb_pipeline();
    std::cout << "Using pipeline: \n\t" << pipeline << "\n";
    // std::cout<<cv::getBuildInformation()<<std::endl;
    cv::VideoCapture cap;
    bool no_sleep = true;
    if (!video_file_path->value().empty()) {
        cap = cv::VideoCapture(video_file_path->value(), cv::CAP_FFMPEG);
        no_sleep = false;
    }
    else {
        cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);
        
        if(!cap.isOpened()) {
            std::cout<<"Failed to open camera. Try USB camera..."<<std::endl;
            cap.release();
            cap = cv::VideoCapture(0);
            // TODO remove this
            if(!cap.isOpened()) {
                std::cout<<"Failed to USB camera."<<std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    
    

    std::thread thread([&]() {
        cv::Mat img;
        std::shared_ptr<openvslam::publish::map_publisher> map_publisher(SLAM.get_map_publisher());
        std::cout << "Hit ESC to exit" << "\n" ;

        auto landmarks_sent_at = std::chrono::steady_clock::now();

        while(true)
        {   
            // HACK! skip three frames in video
            if (!no_sleep) {
                cap.read(img);
                cap.read(img);
                cap.read(img);
            }
            if (!cap.read(img)) {
                std::cout<<"Capture read error"<<std::endl;
                cap.release();
                break;
            }

            const auto tp_1 = std::chrono::steady_clock::now();

            auto current_time = std::chrono::system_clock::now();
            auto duration_in_seconds = std::chrono::duration<double>(current_time.time_since_epoch());
            
            openvslam::Mat44_t cam_pose_cw;
            if (!img.empty()) {
            // std::cout << "process image..." << std::endl;
            cam_pose_cw = SLAM.feed_monocular_frame(img, duration_in_seconds.count(), mask);
            }
            else {
                
            std::cout << "frame was empty..." << std::endl;
            }
            // std::cout << "image processed..." << std::endl;
            const auto tp_2 = std::chrono::steady_clock::now();

            const auto track_time = std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count();

            if (stream_pose) {
                auto cam_pose = map_publisher->get_current_cam_pose_wc();
                // std::cout << "sending camera pose..." << std::endl;
                openvslam_api::Stream stream_msg;
                // auto mat = stream_msg.mutable_camera_position();
                auto mat = stream_msg.mutable_camera_position();
                for (int i = 0; i < 16; i++) {
                    int ir = i / 4;
                    int il = i % 4;
                    mat->add_pose(cam_pose(ir, il));
                }
                
                std::string msg_str;
                stream_msg.SerializeToString(&msg_str);
                zmq::message_t response (msg_str.size());
                memcpy ((void *) response.data (), msg_str.c_str(), msg_str.size());
                sock_stream.send(response, zmq::send_flags::dontwait);
            }

            if (stream_landmarks && std::chrono::seconds(1) < std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - landmarks_sent_at)) {
                std::cout << "sending landmarks..." << std::endl;
                landmarks_sent_at = std::chrono::steady_clock::now();

                openvslam_api::Stream stream_msg;
                auto landmarks = stream_msg.mutable_landmarks();
                
                std::vector<openvslam::data::landmark*> all_landmarks;
                std::set<openvslam::data::landmark*> local_landmarks;
                map_publisher->get_landmarks(all_landmarks, local_landmarks);
                for (const auto landmark : all_landmarks) {
                    if (landmark->will_be_erased()) {
                        continue;
                    }
                    const auto pos = landmark->get_pos_in_world();
                    auto lm_pb = landmarks->add_landmarks();
                    lm_pb->set_id(landmark->id_);
                    lm_pb->set_x(pos[0]);
                    lm_pb->set_y(pos[1]);
                    lm_pb->set_z(pos[2]);
                    lm_pb->set_num_observations(landmark->num_observations());
                }

                std::string msg_str;
                stream_msg.SerializeToString(&msg_str);
                zmq::message_t response (msg_str.size());
                memcpy ((void *) response.data (), msg_str.c_str(), msg_str.size());
                sock_stream.send(response, zmq::send_flags::dontwait);
            }

            if (stream_keyframes) {
                std::vector<openvslam::data::keyframe*> all_keyframes;
                map_publisher->get_keyframes(all_keyframes);

                openvslam_api::Stream stream_msg;
                auto pb_keyframes = stream_msg.mutable_keyframes();
                
                for (const auto keyframe : all_keyframes) {
                    if (keyframe->will_be_erased()) {
                        continue;
                    }
                    const auto cam_pose = keyframe->get_cam_pose_inv();
                    auto pb_keyframe = pb_keyframes->add_keyframes();

                    pb_keyframe->set_id(keyframe->id_);
                    auto mat = pb_keyframe->mutable_pose();
                    for (int i = 0; i < 16; i++) {
                        int ir = i / 4;
                        int il = i % 4;
                        mat->add_pose(cam_pose(ir, il));
                    }
                }

                std::string msg_str;
                stream_msg.SerializeToString(&msg_str);
                zmq::message_t response (msg_str.size());
                memcpy ((void *) response.data (), msg_str.c_str(), msg_str.size());
                sock_stream.send(response, zmq::send_flags::dontwait);
            }

            if (!no_sleep) {
                const auto wait_time = 1.0 / cfg->camera_->fps_ - track_time;
                if (0.0 < wait_time) {
                    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<unsigned int>(wait_time * 1e6)));
                }
            }

            // check if the termination of SLAM system is requested or not
            if (SLAM.terminate_is_requested()) {
                cap.release();
                break;
            }
        }

        // wait until the loop BA is finished
        while (SLAM.loop_BA_is_running()) {
            std::this_thread::sleep_for(std::chrono::microseconds(5000));
        }
    });

    openvslam_api::Request request_msg;
    openvslam_api::Response response_msg;
    std::cout << "Waiting for command" << std::endl;
    while (!SLAM.terminate_is_requested()) {
        // stream_pose = true;
        // Receive all parts of the message
        // std::vector<zmq::message_t> recv_msgs;
        // zmq::recv_result_t result =
        //     zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        zmq::message_t request;
        std::cout << "Waaaiiiitinng..." << std::endl;
        sock_rep.recv(&request);
        std::cout << "Received" << std::endl;
        std::string msg_str(static_cast<char*>(request.data()), request.size());
        request_msg.ParseFromString(msg_str);
        std::cout << "Got command " << std::endl;
        switch (request_msg.msg_case()) {
            case openvslam_api::Request::MsgCase::kTerminate:
                std::cout << "Terminating down..." << std::endl;
                SLAM.request_terminate();
                break;
        }
        
        response_msg.mutable_ok();
        response_msg.SerializeToString(&msg_str);
        zmq::message_t response (msg_str.size());
        memcpy ((void *) response.data (), msg_str.c_str(), msg_str.size());
        sock_rep.send(response);

    }
    thread.join();

    return EXIT_SUCCESS;
}
