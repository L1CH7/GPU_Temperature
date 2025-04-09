#pragma once 

#include <JsonHelper.h>
#include <string>

struct GpuTemp 
{
    std::string name;
    float temperature;
}; 

static void to_json( json & j, const GpuTemp & obj )
{
    j["name"] = obj.name;
    j["temperature"] = obj.temperature;
}

static void from_json( const json & j, GpuTemp & obj )
{
    obj.name = j["name"];
    obj.temperature = j["temperature"];
}