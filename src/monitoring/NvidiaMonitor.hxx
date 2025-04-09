#pragma once

#include <Monitor.hxx>

#include <string>
#include <vector>
#include <cstdint>
#include <nvml.h>

#include <GpuTemp.hxx>

class NvidiaMonitor : public IMonitor< std::vector< GpuTemp > >
{
public:

    NvidiaMonitor(){}

    bool initialize() override;

    std::vector< GpuTemp > toggle() override;

    ~NvidiaMonitor();
};
