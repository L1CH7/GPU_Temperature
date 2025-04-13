#pragma once

#include <UdpNode.hxx>
#include <IMonitor.hxx>
#include <JsonHelper.h>

#include <memory>
#include <iostream>

class UdpServer : public UdpNode 
{
    std::unique_ptr< IMonitor > monitor_;
    size_t alive_send_freq_;
    size_t data_send_freq_;
    boost::asio::steady_timer data_timer_;
    boost::asio::steady_timer alive_timer_;
    bool is_sending_data_ = false;

public:
    UdpServer(boost::asio::io_context& io_context, const std::string& address, size_t data_port, size_t control_port, size_t alive_send_freq)
    :   UdpNode(io_context, address, data_port, control_port), data_timer_(io_context), alive_send_freq_(alive_send_freq) 
    {}

    void setMonitor( std::unique_ptr< IMonitor > & m )
    {
        monitor_ = std::move( m );
    }
        
    void start_send_alive() {
        socket_.async_send_to(
            boost::asio::buffer("ALIVE"),
            multicast_endpoint_,
            [this](auto...){ schedule_next_alive(); });
    }
    
    void schedule_next_alive() {
        alive_timer_.expires_after(std::chrono::milliseconds(100)); // Отправка alive каждые 100 мс
        alive_timer_.async_wait([this](auto){ start_send_alive(); });
    }
    
    void handle_alive_response() {
        is_sending_data_ = true;
        start_send_data();
    }
    
    void start_send_data() {
        if (is_sending_data_) {
            auto data = monitor_.toggle();
            socket_.async_send_to(
                boost::asio::buffer(nlohmann::json(data).dump()),
                multicast_endpoint_,
                [this](auto...){ schedule_next_data(); });
        }
    }
    
    void schedule_next_data() {
        data_timer_.expires_after(std::chrono::seconds(1)); // Отправка данных каждую секунду
        data_timer_.async_wait([this](auto){ start_send_data(); });
    }
    
    void handle_control_message(const std::string& message) {
        if (message == "ALIVE_RESPONSE") {
            handle_alive_response();
        } else if (message == "START") {
            is_sending_data_ = true;
            start_send_data();
        } else if (message == "END") {
            is_sending_data_ = false;
        }
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
        handle_control_message(message);
        control_socket_.async_receive_from(
            boost::asio::buffer(buffer_),
            control_endpoint_,
            [this, &control_socket_](auto...){ handle_control_message(control_socket_); });
    }
    
    void start() {
        start_send_alive();
        start_control_listen();
    }
};
