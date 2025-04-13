#pragma once

#include <UdpNode.hxx>
#include <iostream>

class UdpClient : public UdpNode 
{
public:
    UdpClient(boost::asio::io_context& io_context, const std::string& address, int data_port, int control_port)
    :   UdpNode(io_context, address, data_port, control_port) 
    {}
        
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(buffer_),
            sender_endpoint_,
            [this](auto...){ handle_receive(); });
    }
    
    void handle_receive() {
        std::string message(buffer_.data(), buffer_.size());
        std::cout << "Received: " << message << std::endl;
        start_receive();
    }
    
    void start_control_listen() {
        boost::asio::ip::udp::socket control_socket_(io_context_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), control_port_));
        control_socket_.async_receive_from(
            boost::asio::buffer(buffer_),
            control_endpoint_,
            [this, &control_socket_](auto...){ handle_control_message(control_socket_); });
    }
    
    void handle_control_message(boost::asio::ip::udp::socket& control_socket_) {
        std::string message(buffer_.data(), buffer_.size());
        if (message == "ALIVE") {
            control_socket_.send_to(boost::asio::buffer("ALIVE_RESPONSE"), control_endpoint_);
        }
        control_socket_.async_receive_from(
            boost::asio::buffer(buffer_),
            control_endpoint_,
            [this, &control_socket_](auto...){ handle_control_message(control_socket_); });
    }
    
    void start() {
        join_multicast("0.0.0.0");
        start_receive();
        start_control_listen();
    }
};