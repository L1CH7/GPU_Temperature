#pragma once
#include <JsonHelper.h>

/**
 * Interface of any monitoring device. To get some info use operator():
 * Monitor m; 
 * nlohmann::json j = m();
 * std::cout << j.dump(); 
 */
class IMonitor
{
public:
    virtual ~IMonitor() = default;
    virtual bool initialize() = 0;
    virtual json operator()() = 0;
};
