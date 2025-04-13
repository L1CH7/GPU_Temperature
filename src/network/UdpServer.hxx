#pragma once

#include <boost/asio.hpp>
// #include <boost/asio/ip/address_v4.hpp>
#include <iostream>
#include <memory>
#include <map>

#include <Monitor.hxx>
#include <JsonHelper.h>

template< typename MonitorTp >
class UdpServer 
{
public:
    UdpServer( boost::asio::io_context & io, const boost::asio::ip::udp::endpoint & server_ep, const boost::asio::ip::udp::endpoint & data_ep, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 );
    void start( boost::system::error_code & ec );
    void stop( boost::system::error_code & ec );

private:
    void start_recieve( boost::system::error_code & ec );
    void handle_receive( const boost::system::error_code & ec, std::size_t bytes, boost::system::error_code & outer_ec );
    void send_data( boost::system::error_code & ec );

    boost::asio::ip::udp::socket socket_;
    std::array< char, 1024 > recv_buffer_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::shared_ptr< MonitorTp > monitor_;
    std::atomic< bool > running_;
    std::thread worker_thread_;
    // std::map< boost::asio::ip::udp::endpoint, bool > clients_;
    size_t frequency_; // Частота отправки данных в миллисекундах
};

template< typename MonitorTp >
std::shared_ptr< UdpServer< MonitorTp > > 
makeUdpServer( boost::asio::io_context & io, const boost::asio::ip::udp::endpoint & server_ep, const boost::asio::ip::udp::endpoint & data_ep, std::shared_ptr< MonitorTp > monitor, size_t frequency = 200 )
{
    return std::make_shared< UdpServer< MonitorTp > >( io, server_ep, data_ep, monitor, frequency );
}

template< typename MonitorTp >
UdpServer< MonitorTp >::UdpServer( boost::asio::io_context & io, const boost::asio::ip::udp::endpoint & ep, const boost::asio::ip::udp::endpoint & data_ep, std::shared_ptr< MonitorTp > monitor, size_t frequency )
:   socket_( io, ep ),
    remote_endpoint_( data_ep ),
    monitor_( monitor ),
    running_( false ),
    frequency_( frequency )
    // :   remote_endpoint_( client_ep ),
{
    // socket_.open( boost::asio::ip::udp::v4() );
    boost::system::error_code ec;
    start_recieve( ec );
    if( ec )
        std::cerr << ec.what() << '\n';
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::start( boost::system::error_code & ec ) 
{
    running_ = true;
    worker_thread_ = std::thread( [this, &ec]
        {
            while( running_ ) 
            {
                // if( !clients_.empty() ) 
                // {
                    // auto data = monitor_->toggle();
                    // send_data( data, ec );
                    send_data( ec );
                // }
                std::this_thread::sleep_for( std::chrono::milliseconds( frequency_ ) );
            }
        }
    );
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::stop( boost::system::error_code & ec ) 
{
    running_ = false;
    if( worker_thread_.joinable() )
    {
        worker_thread_.join();
    }
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::start_recieve( boost::system::error_code & ec ) 
{
    socket_.async_receive_from(
        boost::asio::buffer( recv_buffer_ ),
        remote_endpoint_,
        [this, &ec]( const boost::system::error_code & inner_ec, size_t bytes ) 
        {
            std::cout << "start recieving..." << remote_endpoint_ << std::endl;
            handle_receive( inner_ec, bytes, ec );
        }
    );
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::handle_receive( const boost::system::error_code & ec, std::size_t bytes, boost::system::error_code & outer_ec )
{
    if( ec ) 
    {
        outer_ec = ec;
        return;
    }

    std::cout << "recieved data!" << std::endl;
    std::string cmd( recv_buffer_.data(), bytes );

    if( cmd[0] == 'S' ) 
    {
        // clients_[remote_endpoint_] = true;
        std::cout << "Client connected: " << remote_endpoint_ << std::endl;
        std::cout << "Start: " << std::endl;
        start( outer_ec );
    } 
    else if( cmd[0] == 'E' ) 
    {
        // clients_.erase( remote_endpoint_ );
        std::cout << "Client disconnected: " << remote_endpoint_ << std::endl;
        std::cout << "Stopped: " << std::endl;
        stop( outer_ec );
    }

    start_recieve( outer_ec );
}

template< typename MonitorTp >
void UdpServer< MonitorTp >::send_data( boost::system::error_code & ec ) 
{
    auto data = monitor_->toggle();
    json j( data );
    std::string msg( j.dump() + '\n' );
    std::cout << "sending data to.." << remote_endpoint_ << std::endl;

    socket_.send_to( boost::asio::buffer( msg ), remote_endpoint_, 0, ec );
    
    if( ec ) 
    {
        std::cerr << "Error sending to " << remote_endpoint_ << ": " << ec.message() << std::endl;
    }
    // for( const auto & [endpoint, active] : clients_ ) 
    // {
    //     if( active ) 
    //     {
    //         socket_.send_to( boost::asio::buffer( msg ), endpoint, 0, ec );
    //         if( ec ) 
    //         {
    //             std::cerr << "Error sending to " << endpoint << ": " << ec.message() << std::endl;
    //             clients_.erase( endpoint );
    //         }
    //     }
    // }
}
