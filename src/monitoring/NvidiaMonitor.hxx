#pragma once

#include <IMonitor.hxx>

#include <string>
#include <vector>
#include <cstdint>
#include <nvml.h>

#include <GpuTemp.hxx>

class NvidiaMonitor : public IMonitor
{
public:

    NvidiaMonitor(){}

    bool initialize() override;

    json operator()() override;

    ~NvidiaMonitor();
};
