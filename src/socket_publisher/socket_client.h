#ifndef SOCKET_PUBLISHER_SOCKET_CLIENT_H
#define SOCKET_PUBLISHER_SOCKET_CLIENT_H

#include "openvslam/config.h"

#include <sioclient/sio_client.h>
#include <future>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <iostream>
// #include <amqpcpp.h>
// #include <amqpcpp/linux_tcp.h>
// #include "socket_publisher/MyTcpHandler.h"
// #include "socket_publisher/conn_handler.h"

namespace openvslam {
class config;
} // namespace openvslam

namespace socket_publisher {

class socket_client {
public:
    socket_client(const std::string& server_uri);

    void emit(const std::string tag, const std::string buffer) {
        socket_->emit(tag, buffer);
        if (tag == "map_publish") {
            std::cout << "MAP_PUB:";
            publisher_.send(zmq::buffer(buffer), zmq::send_flags::dontwait);
        }
        else if (tag == "frame_publish") {
            
        }
    }

    void set_signal_callback(std::function<void(std::string)> callback) {
        callback_ = callback;
    }

    // MyTcpHandler myHandler;
    // AMQP::TcpConnection connection_;
    // AMQP::TcpChannel channel_;

private:
    void on_close();
    void on_fail();
    void on_open();
    void on_receive(const sio::event& event);

    sio::client client_;
    sio::socket::ptr socket_;

    zmq::context_t ctx_;
    zmq::socket_t publisher_;

    std::function<void(std::string)> callback_;
};

} // namespace socket_publisher

#endif // SOCKET_PUBLISHER_SOCKET_CLIENT_H
