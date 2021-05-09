#include "socket_publisher/socket_client.h"

#include <spdlog/spdlog.h>



namespace socket_publisher {

socket_client::socket_client(const std::string& server_uri)
    : client_(), callback_() {
    // register socket callbacks
    spdlog::info(server_uri);
    client_.set_open_listener(std::bind(&socket_client::on_open, this));
    client_.set_close_listener(std::bind(&socket_client::on_close, this));
    client_.set_fail_listener(std::bind(&socket_client::on_fail, this));

    // // create an instance of your own tcp handler
    // MyTcpHandler myHandler;

    // // address of the server
    // AMQP::Address address("amqp://localhost/");

    // // create a AMQP connection object
    // AMQP::TcpConnection connection(&myHandler, address);

    // // and create a channel
    // AMQP::TcpChannel channel(&connection);

    // // // use the channel object to call the AMQP method you like
    // channel.declareExchange("my-exchange", AMQP::fanout);
    // channel.declareQueue("my-queue");
    // channel.bindQueue("my-exchange", "my-queue", "my-routing-key");

    // channel.startTransaction();

    // int pubVal = channel.publish("my-exchange", "my-routing-key", "masssage");

    // start connection
    client_.connect(server_uri);
    // get socket
    socket_ = client_.socket();

    ctx_ = zmq::context_t(2);
    publisher_ = zmq::socket_t(ctx_, zmq::socket_type::pub);
    int confl = 1;
    publisher_.setsockopt(ZMQ_CONFLATE, &confl, sizeof(confl));
    publisher_.bind("tcp://*:5566");


    // // subscribe to control messages
    // std::thread thread([&]() {
    //     //  Prepare subscriber
    //     zmq::socket_t subscriber(ctx_, zmq::socket_type::sub);
    //     subscriber.connect("tcp://192.168.50.234:5561");
    //     //  Thread3 opens ALL envelopes
    //     subscriber.set(zmq::sockopt::subscribe, "");
    //     while (true) {
    //         // Receive all parts of the message
    //         // std::vector<zmq::message_t> recv_msgs;
    //         // zmq::recv_result_t result =
    //         //     zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));

    //         zmq::message_t message;
    //         auto result = subscriber.recv(&message);
    //         assert(result && "recv failed");

    //         // std::cout << "Got "<< std::endl;
    //         assert(*result == 2);

    //         auto msg0 = message.to_string();//recv_msgs[0].to_string();
    //         spdlog::info("got message {}", msg0);
    //         // if (callback_) {
    //         //     callback_(msg0);
    //         // }
    //     }
    // });

    socket_->on("signal", std::bind(&socket_client::on_receive, this, std::placeholders::_1));
}

void socket_client::on_close() {
    spdlog::info("connection closed correctly");
}

void socket_client::on_fail() {
    spdlog::info("connection closed incorrectly");
}

void socket_client::on_open() {
    spdlog::info("connected to server");
}

void socket_client::on_receive(const sio::event& event) {
    try {
        const std::string message = event.get_message()->get_string();
        if (callback_) {
            callback_(message);
        }
    }
    catch (std::exception& ex) {
        spdlog::error(ex.what());
    }
}

} // namespace socket_publisher
