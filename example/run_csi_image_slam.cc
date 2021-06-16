#include "util/image_util.h"

#include <future>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#ifdef USE_PANGOLIN_VIEWER
#include "pangolin_viewer/viewer.h"
#elif USE_SOCKET_PUBLISHER
#include "socket_publisher/publisher.h"
#endif

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

std::string gstreamer_pipeline (int capture_width, int capture_height, int display_width, int display_height, int framerate, int flip_method) {
    return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(capture_width) + ", height=(int)" +
           std::to_string(capture_height) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(framerate) +
           "/1 ! nvvidconv flip-method=" + std::to_string(flip_method) + " ! video/x-raw, width=(int)" + std::to_string(display_width) + ", height=(int)" +
           std::to_string(display_height) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

void mono_tracking(const std::shared_ptr<openvslam::config>& cfg,
                   const std::string& vocab_file_path, const std::string& mask_img_path,
                   const unsigned int frame_skip, const bool no_sleep, const bool auto_term,
                   const bool eval_log, const std::string& map_db_path_in, const std::string& map_db_path_out) {
    spdlog::set_level(spdlog::level::trace);
    zmq::context_t ctx(2);

    int capture_width = 640 ;
    int capture_height = 480 ;
    int display_width = 640 ;
    int display_height = 480 ;
    int framerate = 4 ;
    int flip_method = 2 ;

    std::string pipeline = gstreamer_pipeline(capture_width,
	capture_height,
	display_width,
	display_height,
	framerate,
	flip_method);
    std::cout << "Using pipeline: \n\t" << pipeline << "\n";
 
    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened()) {
        std::cout<<"Failed to open camera."<<std::endl;
        return;
    }

    // load the mask image
    const cv::Mat mask = mask_img_path.empty() ? cv::Mat{} : cv::imread(mask_img_path, cv::IMREAD_GRAYSCALE);

    // const image_sequence sequence(image_dir_path, cfg->camera_->fps_);
    // const auto frames = sequence.get_frames();

    // build a SLAM system
    openvslam::system SLAM(cfg, vocab_file_path);

    // load the prebuilt map
    if (!map_db_path_in.empty()) {
        std::cout << "loading map..." << std::endl;
        SLAM.load_map_database(map_db_path_in);
    }
    // startup the SLAM process
    SLAM.startup(map_db_path_in.empty());
    SLAM.enable_mapping_module();

#ifdef USE_PANGOLIN_VIEWER
    pangolin_viewer::viewer viewer(cfg, &SLAM, SLAM.get_frame_publisher(), SLAM.get_map_publisher());
#elif USE_SOCKET_PUBLISHER
    socket_publisher::publisher publisher(cfg, &SLAM, SLAM.get_frame_publisher(), SLAM.get_map_publisher());
#endif

    std::vector<double> track_times;
    // track_times.reserve(frames.size());

    // run the SLAM in another thread
    std::thread thread([&]() {
        cv::Mat img;

        std::cout << "Hit ESC to exit" << "\n" ;
        while(true)
        {
            if (!cap.read(img)) {
                std::cout<<"Capture read error"<<std::endl;
                cap.release();
                break;
            }
            // cv::imwrite("test.jpg",img);
            
            // cv::imshow("CSI Camera",img);
            const auto tp_1 = std::chrono::steady_clock::now();

            // if (!img.empty() && (i % frame_skip == 0)) {
                // input the current frame and estimate the camera pose
                auto current_time = std::chrono::system_clock::now();
                auto duration_in_seconds = std::chrono::duration<double>(current_time.time_since_epoch());
                
            std::cout << "process image..." << std::endl;
                SLAM.feed_monocular_frame(img, duration_in_seconds.count(), mask);
            // }

            const auto tp_2 = std::chrono::steady_clock::now();

            const auto track_time = std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count();

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

        // automatically close the viewer
#ifdef USE_PANGOLIN_VIEWER
        if (auto_term) {
            viewer.request_terminate();
        }
#elif USE_SOCKET_PUBLISHER
        if (auto_term) {
            publisher.request_terminate();
        }
#endif
    });

    std::thread thread2([&]() {
        std::cout << "responder thread..." << std::endl;
        //  Prepare subscriber
        zmq::socket_t responder(ctx, zmq::socket_type::rep);
	    responder.bind("tcp://0.0.0.0:5561");

        while (true) {
            // Receive all parts of the message
            // std::vector<zmq::message_t> recv_msgs;
            // zmq::recv_result_t result =
            //     zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
            zmq::message_t message;
            auto result = responder.recv(&message);
            assert(result && "recv failed");
            assert(*result == 2);

            auto msg = message.to_string();//recv_msgs[0].to_string();

            std::cout << "Got command "<< msg<< std::endl;
            
            if (msg == "disable_mapping_mode") {
                SLAM.disable_mapping_module();
            }
            else if (msg == "enable_mapping_mode") {
                SLAM.enable_mapping_module();
            }
            else if (msg == "reset") {
                SLAM.request_reset();
            }
            else if (msg == "terminate") {
                SLAM.request_terminate();
                #ifdef USE_SOCKET_PUBLISHER
                    publisher.request_terminate();
                #endif

                break;
            }

            responder.send(zmq::str_buffer(""));

            // check if the termination of SLAM system is requested or not
            if (SLAM.terminate_is_requested()) {
                break;
            }
        }
    });

    // run the viewer in the current thread
#ifdef USE_PANGOLIN_VIEWER
    viewer.run();
#elif USE_SOCKET_PUBLISHER
    publisher.run();
#endif

    thread2.join();
    
    // shutdown the SLAM process
    SLAM.shutdown();

    if (eval_log) {
        // output the trajectories for evaluation
        SLAM.save_frame_trajectory("frame_trajectory.txt", "TUM");
        SLAM.save_keyframe_trajectory("keyframe_trajectory.txt", "TUM");
        // output the tracking times for evaluation
        std::ofstream ofs("track_times.txt", std::ios::out);
        if (ofs.is_open()) {
            for (const auto track_time : track_times) {
                ofs << track_time << std::endl;
            }
            ofs.close();
        }
    }

    if (!map_db_path_out.empty()) {
        // output the map database
        SLAM.save_map_database(map_db_path_out);
    }

    std::sort(track_times.begin(), track_times.end());
    const auto total_track_time = std::accumulate(track_times.begin(), track_times.end(), 0.0);
    std::cout << "median tracking time: " << track_times.at(track_times.size() / 2) << "[s]" << std::endl;
    std::cout << "mean tracking time: " << total_track_time / track_times.size() << "[s]" << std::endl;
}

int main(int argc, char* argv[]) {
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);
    sock.bind("inproc://test");
    sock.send(zmq::str_buffer("Hello, world"), zmq::send_flags::dontwait);
    
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
    auto frame_skip = op.add<popl::Value<unsigned int>>("", "frame-skip", "interval of frame skip", 1);
    auto no_sleep = op.add<popl::Switch>("", "no-sleep", "not wait for next frame in real time");
    auto auto_term = op.add<popl::Switch>("", "auto-term", "automatically terminate the viewer");
    auto debug_mode = op.add<popl::Switch>("", "debug", "debug mode");
    auto eval_log = op.add<popl::Switch>("", "eval-log", "store trajectory and tracking times for evaluation");
    auto map_db_path_in = op.add<popl::Value<std::string>>("", "map-db-in", "load map database from this path before SLAM", "");
    auto map_db_path_out = op.add<popl::Value<std::string>>("", "map-db-out", "store a map database at this path after SLAM", "");
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

#ifdef USE_GOOGLE_PERFTOOLS
    ProfilerStart("slam.prof");
#endif

    // run tracking
    if (cfg->camera_->setup_type_ == openvslam::camera::setup_type_t::Monocular) {
        mono_tracking(cfg, vocab_file_path->value(), mask_img_path->value(),
                      frame_skip->value(), no_sleep->is_set(), auto_term->is_set(),
                      eval_log->is_set(), map_db_path_in->value(), map_db_path_out->value());
    }
    else {
        throw std::runtime_error("Invalid setup type: " + cfg->camera_->get_setup_type_string());
    }

#ifdef USE_GOOGLE_PERFTOOLS
    ProfilerStop();
#endif

    return EXIT_SUCCESS;
}
