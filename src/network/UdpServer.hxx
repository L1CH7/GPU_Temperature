#pragma once

// template< typename DataTp >
// class UdpServer
// {
// public:
//     virtual ~IMonitor() = default;
//     virtual bool initialize() = 0;
//     virtual DataTp toggle() = 0;
// };

// udp_server.h
#include <Monitor.hxx>
#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <memory>
#include <JsonHelper.h>

template< typename MonitorTp >
class UdpServer 
{
public:
    UdpServer( boost::asio::io_context & io_context, size_t port, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 );
    UdpServer( boost::asio::io_context & io_context, const std::string & ipv4_str, size_t port, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 );
    UdpServer( boost::asio::io_context & io_context, const boost::asio::ip::udp::endpoint & server_ep, const boost::asio::ip::udp::endpoint & client_ep, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 );
    void start();
    void stop();

private:
    void send_data( boost::system::error_code & ec );
    void handle_receive( const boost::system::error_code& error, std::size_t bytes_transferred, boost::system::error_code & ec );

    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::shared_ptr< MonitorTp > monitor_;
    boost::asio::ip::udp::socket socket_;
    std::atomic< bool > running_;
    std::thread worker_thread_;
    size_t frequency_; // Частота отправки данных в миллисекундах
};

template< typename MonitorTp >
std::shared_ptr< UdpServer< MonitorTp > > makeUdpServer( boost::asio::io_context & io_context, const boost::asio::ip::udp::endpoint & server_ep, const boost::asio::ip::udp::endpoint & client_ep, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 )
{
    return std::make_shared< UdpServer< MonitorTp > >( io_context, server_ep, client_ep, monitor, frequency );
}

template< typename MonitorTp >
std::shared_ptr< UdpServer< MonitorTp > > makeUdpServer( boost::asio::io_context & io_context, size_t port, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 )
{
    return std::make_shared< UdpServer< MonitorTp > >( io_context, port, monitor, frequency );
}

template< typename MonitorTp >
std::shared_ptr< UdpServer< MonitorTp > > makeUdpServer( boost::asio::io_context & io_context, std::string ipv4_str, size_t port, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 )
{
    return std::make_shared< UdpServer< MonitorTp > >( io_context, ipv4_str, port, monitor, frequency );
}


#include <iostream>
template< typename MonitorTp >
UdpServer< MonitorTp >::UdpServer( boost::asio::io_context & io_context, const boost::asio::ip::udp::endpoint & server_ep, const boost::asio::ip::udp::endpoint & client_ep, std::shared_ptr< MonitorTp > monitor, size_t frequency )
:   remote_endpoint_( client_ep ),
    socket_( io_context, server_ep ),
    running_( false ),
    frequency_( frequency ), 
    monitor_( monitor ) 
{
    // socket_.open( boost::asio::ip::udp::v4() );
}
template< typename MonitorTp >
UdpServer< MonitorTp >::UdpServer( boost::asio::io_context & io_context, const std::string & ipv4_str, size_t port, std::shared_ptr< MonitorTp > monitor, size_t frequency )
:   remote_endpoint_( boost::asio::ip::make_address( ipv4_str ), port ),
    socket_( io_context, remote_endpoint_ ),
    running_( false ),
    frequency_( frequency ), 
    monitor_( monitor ) 
{
    // socket_.open( boost::asio::ip::udp::v4() );
}

template< typename MonitorTp >
UdpServer< MonitorTp >::UdpServer( boost::asio::io_context & io_context, size_t port, std::shared_ptr< MonitorTp > monitor, size_t frequency )
:   socket_( io_context, boost::asio::ip::udp::endpoint( boost::asio::ip::make_address("127.0.0.1"), port ) ),
// :   socket_( io_context, boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), port ) ),
    running_( false ),
    frequency_( frequency ), // По умолчанию 1 секунда
    monitor_( monitor ) 
{
    // socket_.open( boost::asio::ip::udp::v4() );
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::start() 
{
    running_ = true;
    worker_thread_ = std::thread( [this]
        {
            while( running_ ) 
            {
                boost::system::error_code ec;
                send_data( ec );
                if( ec )
                    std::cerr << "Send error: " << ec.message() << std::endl;
                std::this_thread::sleep_for( std::chrono::milliseconds( frequency_ ) );
            }
        }
    );

    socket_.async_receive_from( boost::asio::buffer( new char[1], 1 ), remote_endpoint_,
        [ this ]( const boost::system::error_code & error, std::size_t bytes_transferred )
        {
            boost::system::error_code ec;
            handle_receive( error, bytes_transferred, ec );
            if( ec )
                std::cerr << "Recieve error: " << ec.message() << std::endl;
        }
    );
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::stop() 
{
    running_ = false;
    if( worker_thread_.joinable() )
    {
        worker_thread_.join();
    }
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::send_data( boost::system::error_code & ec ) 
{
    auto data = monitor_->toggle(); // Получение данных
    json j( data );
    std::string msg( j.dump() + '\n' );
    // for( const auto & item : data)
    // {
    //     // Преобразование данных в строку для отправки
    //     std::string message = "Name: " + item.name + ", Temp: " + std::to_string(item.temperature) + " C";
    //     socket_.send_to(boost::asio::buffer(j.dump), remote_endpoint_);
    // }
    socket_.send_to( boost::asio::buffer( msg ), remote_endpoint_, 0, ec );
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::handle_receive( const boost::system::error_code& error, std::size_t bytes_transferred, boost::system::error_code & ec )
{
    if( !error && bytes_transferred > 0 ) 
    {
        char buffer[10];
        socket_.receive_from( boost::asio::buffer( buffer ), remote_endpoint_, 0, ec );
        if (buffer[0] == 'S') { // Signal to start
            start();
        } 
        else if( buffer[0] == 'E' ) 
        { // Signal to stop
            stop();
        } 
        else if( isdigit( buffer[0] ) ) 
        { // Set frequency (in seconds)
            frequency_ = std::stoul( buffer );
        }
    }
}
