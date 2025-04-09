#pragma once

template< typename DataTp >
class IMonitor
{
public:
    virtual ~IMonitor() = default;
    virtual bool initialize() = 0;
    virtual DataTp toggle() = 0;
};
