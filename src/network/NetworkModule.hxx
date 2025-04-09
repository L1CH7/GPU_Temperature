#pragma once 

#include <boost/asio.hpp>
#include <memory>

class NetworkModule 
{
public:
    virtual ~NetworkModule() = default;
    virtual void run() = 0;
};