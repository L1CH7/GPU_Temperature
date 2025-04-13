#pragma once

#include <JsonHelper.h>
#include <boost/asio.hpp>

class UdpNode {
protected:
    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::endpoint multicast_endpoint_;
    boost::asio::ip::udp::endpoint control_endpoint_;
    std::array<char, 1024> buffer_;

public:
    UdpNode(boost::asio::io_context& io_context, const std::string& address, int data_port, int control_port)
        : io_context_(io_context), socket_(io_context_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) {
        multicast_endpoint_ = boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(address), data_port);
        control_endpoint_ = boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(address), control_port);
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    }

    virtual ~UdpNode() = default;

    void join_multicast(const std::string& interface) {
        auto multicast_address = multicast_endpoint_.address().to_v4(); // Преобразование в address_v4
        auto interface_address = boost::asio::ip::make_address_v4(interface); // Преобразование в address_v4
        socket_.set_option(boost::asio::ip::multicast::join_group(multicast_address, interface_address));
    }

    void leave_multicast(const std::string& interface) {
        auto multicast_address = multicast_endpoint_.address().to_v4(); // Преобразование в address_v4
        auto interface_address = boost::asio::ip::make_address_v4(interface); // Преобразование в address_v4
        socket_.set_option(boost::asio::ip::multicast::leave_group(multicast_address, interface_address));
    }
};
