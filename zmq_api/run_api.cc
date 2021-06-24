
#include <future>
#include <zmq.hpp>
#include <zmq_addon.hpp>    

#include "openvslam/system.h"
#include "openvslam/config.h"

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


// void mono_tracking(const std::shared_ptr<openvslam::config>& cfg,
//                    const std::string& vocab_file_path, const std::string& mask_img_path,
//                    const unsigned int frame_skip, const bool no_sleep, const bool auto_term,
//                    const bool eval_log, const std::string& map_db_path_in, const std::string& map_db_path_out) {
//     spdlog::set_level(spdlog::level::trace);
//     zmq::context_t ctx(1);

//     // auto thread1 = std::async(std::launch::async, SubscriberThread, &ctx);
//     // thread1.wait();

//     // load the mask image
//     const cv::Mat mask = mask_img_path.empty() ? cv::Mat{} : cv::imread(mask_img_path, cv::IMREAD_GRAYSCALE);

//     // const image_sequence sequence(image_dir_path, cfg->camera_->fps_);
//     // const auto frames = sequence.get_frames();

//     // build a SLAM system
//     openvslam::system SLAM(cfg, vocab_file_path);

//     // load the prebuilt map
//     if (!map_db_path_in.empty()) {
//         std::cout << "loading map..." << std::endl;
//         SLAM.load_map_database(map_db_path_in);
//     }
//     // startup the SLAM process
//     SLAM.startup(map_db_path_in.empty());
//     SLAM.enable_mapping_module();

//     std::vector<double> track_times;
//     // track_times.reserve(frames.size());

//     // run the SLAM in another thread
//     std::thread thread([&]() {
//         std::cout << "subscriber thread..." << std::endl;
//         //  Prepare subscriber
//         zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
//         subscriber.connect("tcp://192.168.50.234:5560");

//         // subscriber.connect("tcp://192.168.50.111:5560");
//         //  Thread3 opens ALL envelopes
//         subscriber.set(zmq::sockopt::subscribe, "");
//         while (true) {
//             // Receive all parts of the message
//             // std::vector<zmq::message_t> recv_msgs;
//             // zmq::recv_result_t result =
//             //     zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));

//             zmq::message_t message;
//             auto result = subscriber.recv(&message);
//             assert(result && "recv failed");

//             // std::cout << "Got image "<< std::endl;
//             assert(*result == 2);

//             auto msg0 = message.to_string();//recv_msgs[0].to_string();
//             std::vector<char> imdata(msg0.begin(), msg0.end());
//             auto img = cv::imdecode(imdata, cv::IMREAD_ANYCOLOR);

//             // std::cout << "got image" <<std::endl;
//             // std::cout << "Thread2: [" << recv_msgs[0].to_string() << "] "
//             //         << recv_msgs[1].to_string() << std::endl;
//         // }
//         // for (unsigned int i = 0; i < frames.size(); ++i) {
//         //     const auto& frame = frames.at(i);
//         //     const auto img = cv::imread(frame.img_path_, cv::IMREAD_UNCHANGED);

//             const auto tp_1 = std::chrono::steady_clock::now();

//             // if (!img.empty() && (i % frame_skip == 0)) {
//                 // input the current frame and estimate the camera pose
//                 auto current_time = std::chrono::system_clock::now();
//                 auto duration_in_seconds = std::chrono::duration<double>(current_time.time_since_epoch());
                
//                 SLAM.feed_monocular_frame(img, duration_in_seconds.count(), mask);
//             // }

//             const auto tp_2 = std::chrono::steady_clock::now();

//             const auto track_time = std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count();
//             // if (i % frame_skip == 0) {
//             //     track_times.push_back(track_time);
//             // }

//             // // wait until the timestamp of the next frame
//             // if (!no_sleep && i < frames.size() - 1) {
//             //     const auto wait_time = frames.at(i + 1).timestamp_ - (frame.timestamp_ + track_time);
//             //     if (0.0 < wait_time) {
//             //         std::this_thread::sleep_for(std::chrono::microseconds(static_cast<unsigned int>(wait_time * 1e6)));
//             //     }
//             // }

//             // check if the termination of SLAM system is requested or not
//             if (SLAM.terminate_is_requested()) {
//                 break;
//             }
//         }

//         // wait until the loop BA is finished
//         while (SLAM.loop_BA_is_running()) {
//             std::this_thread::sleep_for(std::chrono::microseconds(5000));
//         }

//     });

//     std::thread thread2([&]() {
//         std::cout << "responder thread..." << std::endl;
//         //  Prepare subscriber
//         zmq::socket_t responder(ctx, zmq::socket_type::rep);
// 	    responder.bind("tcp://0.0.0.0:5561");

//         while (true) {
//             // Receive all parts of the message
//             // std::vector<zmq::message_t> recv_msgs;
//             // zmq::recv_result_t result =
//             //     zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
//             zmq::message_t message;
//             auto result = responder.recv(&message);
//             assert(result && "recv failed");
//             assert(*result == 2);

//             auto msg = message.to_string();//recv_msgs[0].to_string();

//             std::cout << "Got command "<< msg<< std::endl;
            
//             if (msg == "disable_mapping_mode") {
//                 SLAM.disable_mapping_module();
//             }
//             else if (msg == "enable_mapping_mode") {
//                 SLAM.enable_mapping_module();
//             }
//             else if (msg == "reset") {
//                 SLAM.request_reset();
//             }
//             else if (msg == "terminate") {
//                 SLAM.request_terminate();
//             }

//             responder.send(zmq::str_buffer(""));

//             // check if the termination of SLAM system is requested or not
//             if (SLAM.terminate_is_requested()) {
//                 break;
//             }
//         }
//     });

//     thread2.join();
    
//     // shutdown the SLAM process
//     SLAM.shutdown();

//     if (eval_log) {
//         // output the trajectories for evaluation
//         SLAM.save_frame_trajectory("frame_trajectory.txt", "TUM");
//         SLAM.save_keyframe_trajectory("keyframe_trajectory.txt", "TUM");
//         // output the tracking times for evaluation
//         std::ofstream ofs("track_times.txt", std::ios::out);
//         if (ofs.is_open()) {
//             for (const auto track_time : track_times) {
//                 ofs << track_time << std::endl;
//             }
//             ofs.close();
//         }
//     }

//     if (!map_db_path_out.empty()) {
//         // output the map database
//         SLAM.save_map_database(map_db_path_out);
//     }

//     std::sort(track_times.begin(), track_times.end());
//     const auto total_track_time = std::accumulate(track_times.begin(), track_times.end(), 0.0);
//     std::cout << "median tracking time: " << track_times.at(track_times.size() / 2) << "[s]" << std::endl;
//     std::cout << "mean tracking time: " << total_track_time / track_times.size() << "[s]" << std::endl;
// }

int main(int argc, char* argv[]) {
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::rep);
    sock.bind("ipc:///tmp/openvslam_wrapper_ipc");
    std::cout << "Listening..." << std::endl;

    #ifdef USE_STACK_TRACE_LOGGER
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    #endif

    // create options
    popl::OptionParser op("Allowed options");
    auto help = op.add<popl::Switch>("h", "help", "produce help message");
    auto vocab_file_path = op.add<popl::Value<std::string>>("v", "vocab", "vocabulary file path");
    auto config_file_path = op.add<popl::Value<std::string>>("c", "config", "config file path");
    auto mask_img_path = op.add<popl::Value<std::string>>("", "mask", "mask image path", "");
    auto debug_mode = op.add<popl::Switch>("", "debug", "debug mode");
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

    openvslam_api::Call call;
    openvslam::system SLAM(cfg, vocab_file_path->value());

    while (true) {
        // Receive all parts of the message
        // std::vector<zmq::message_t> recv_msgs;
        // zmq::recv_result_t result =
        //     zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        zmq::message_t request;
        sock.recv(&request);
        std::cout << "Received" << std::endl;
        std::string msg_str(static_cast<char*>(request.data()), request.size());
        call.ParseFromString(msg_str);
        std::cout << "Got command " << std::endl;
        switch (call.msg_case()) {
            case openvslam_api::Call::MsgCase::kStartSystem:
                auto map_file_path = call.start_system().map_file_path();
                if (!map_file_path.empty()) {
                    std::cout << "loading map..." << std::endl;
                    SLAM.load_map_database(map_file_path);
                }
                // startup the SLAM process
                SLAM.startup(map_file_path.empty());
                if (call.start_system().enable_mapping()) {
                    SLAM.enable_mapping_module();
                }
                else {
                    SLAM.disable_mapping_module();
                }
                break;
        }
        
        call.mutable_ok();
        call.SerializeToString(&msg_str);
        zmq::message_t response (msg_str.size());
        memcpy ((void *) response.data (), msg_str.c_str(), msg_str.size());
        sock.send(response);

    }

    
    
    
// #ifdef USE_STACK_TRACE_LOGGER
//     google::InitGoogleLogging(argv[0]);
//     google::InstallFailureSignalHandler();
// #endif

//     // create options
//     popl::OptionParser op("Allowed options");
//     auto help = op.add<popl::Switch>("h", "help", "produce help message");
//     auto vocab_file_path = op.add<popl::Value<std::string>>("v", "vocab", "vocabulary file path");
//     auto config_file_path = op.add<popl::Value<std::string>>("c", "config", "config file path");
//     auto mask_img_path = op.add<popl::Value<std::string>>("", "mask", "mask image path", "");
//     auto frame_skip = op.add<popl::Value<unsigned int>>("", "frame-skip", "interval of frame skip", 1);
//     auto no_sleep = op.add<popl::Switch>("", "no-sleep", "not wait for next frame in real time");
//     auto auto_term = op.add<popl::Switch>("", "auto-term", "automatically terminate the viewer");
//     auto debug_mode = op.add<popl::Switch>("", "debug", "debug mode");
//     auto eval_log = op.add<popl::Switch>("", "eval-log", "store trajectory and tracking times for evaluation");
//     auto map_db_path_in = op.add<popl::Value<std::string>>("", "map-db-in", "load map database from this path before SLAM", "");
//     auto map_db_path_out = op.add<popl::Value<std::string>>("", "map-db-out", "store a map database at this path after SLAM", "");
//     try {
//         op.parse(argc, argv);
//     }
//     catch (const std::exception& e) {
//         std::cerr << e.what() << std::endl;
//         std::cerr << std::endl;
//         std::cerr << op << std::endl;
//         return EXIT_FAILURE;
//     }

//     // check validness of options
//     if (help->is_set()) {
//         std::cerr << op << std::endl;
//         return EXIT_FAILURE;
//     }
//     if (!vocab_file_path->is_set() || !config_file_path->is_set()) {
//         std::cerr << "invalid arguments" << std::endl;
//         std::cerr << std::endl;
//         std::cerr << op << std::endl;
//         return EXIT_FAILURE;
//     }

//     // setup logger
//     spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%L] %v%$");
//     if (debug_mode->is_set()) {
//         spdlog::set_level(spdlog::level::debug);
//     }
//     else {
//         spdlog::set_level(spdlog::level::info);
//     }

//     // load configuration
//     std::shared_ptr<openvslam::config> cfg;
//     try {
//         cfg = std::make_shared<openvslam::config>(config_file_path->value());
//     }
//     catch (const std::exception& e) {
//         std::cerr << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }

// #ifdef USE_GOOGLE_PERFTOOLS
//     ProfilerStart("slam.prof");
// #endif

//     // run tracking
//     if (cfg->camera_->setup_type_ == openvslam::camera::setup_type_t::Monocular) {
//         mono_tracking(cfg, vocab_file_path->value(), mask_img_path->value(),
//                       frame_skip->value(), no_sleep->is_set(), auto_term->is_set(),
//                       eval_log->is_set(), map_db_path_in->value(), map_db_path_out->value());
//     }
//     else {
//         throw std::runtime_error("Invalid setup type: " + cfg->camera_->get_setup_type_string());
//     }

// #ifdef USE_GOOGLE_PERFTOOLS
//     ProfilerStop();
// #endif

    return EXIT_SUCCESS;
}