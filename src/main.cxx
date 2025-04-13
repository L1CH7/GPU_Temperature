#include <NvidiaMonitor.hxx>
#include <UdpServer.hxx>
#include <UdpClient.hxx>
// #include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <fstream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
    // po::options_description desc("Allowed options");
    // desc.add_options()
    //     ("help", "produce help message")
    //     ("server", "launch app in server mode")
    //     ("client", "launch app in client mode")
    // ;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " --server|--client" << std::endl;
        return 1;
    }

    boost::asio::io_context io_context;

    nlohmann::json config;
    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return 1;
    }
    config_file >> config;

    if (std::string(argv[1]) == "--server" || std::string(argv[1]) == "-s") {
        std::unique_ptr< IMonitor > monitor( new NvidiaMonitor() );
        if( !monitor->initialize() )
        {
            std::cerr << "Error initialising monitor!\n";
            return 1;
        }
        UdpServer server(io_context, config["server_address"], config["data_port"], config["control_port"], config["timeout_sec"]);
        server.setMonitor( monitor );
        server.start();
        io_context.run();
    } else if (std::string(argv[1]) == "--client" || std::string(argv[1]) == "-c") {
        UdpClient client(io_context, config["server_address"], config["data_port"], config["control_port"]);
        client.join_multicast("0.0.0.0");
        client.start();
        io_context.run();
    }

    return 0;
}


