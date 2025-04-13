#include <NvidiaMonitor.hxx>
#include "UdpServer.hxx"
// #include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

int main( int argc, char* argv[] )
{
    boost::asio::io_context io;

    auto monitor = std::make_shared< NvidiaMonitor >();
    if( !monitor->initialize() )
    {
        throw std::runtime_error("Failed to initialize monitor");
    }    
    
    // UdpServer<std::vector<GpuTemp>> server(io, 12345, monitor);
    // auto server = makeUdpServer( io, 9000, monitor, 200 );
    boost::asio::ip::udp::endpoint server_ep( boost::asio::ip::make_address( "127.0.0.1" ), 3210 );
    boost::asio::ip::udp::endpoint data_ep( boost::asio::ip::make_address( "127.0.0.1" ), 3211 );
    auto server = makeUdpServer( io, server_ep, data_ep, monitor, 200 );

    boost::system::error_code ec;
    // server->start( ec );
    if( ec ) 
    {
        std::cerr << "Error starting server: " << ec.message() << std::endl;
    }

    io.run(); // Запуск сервера

    std::cout << '\n';
    return 0;
}


