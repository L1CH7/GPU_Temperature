#include <NvidiaMonitor.hxx>

bool 
NvidiaMonitor::initialize() 
{
    return nvmlInit() == NVML_SUCCESS;
}

json NvidiaMonitor::operator()() 
{
    std::vector< GpuTemp > result{};
    uint32_t count = 0;
    
    if( nvmlDeviceGetCount( &count ) != NVML_SUCCESS )
        return result;

    for( uint32_t i = 0; i < count; ++i )
    {
        nvmlDevice_t device{};
        GpuTemp data{};
        
        if( nvmlDeviceGetHandleByIndex( i, &device ) == NVML_SUCCESS )
        {
            char name[NVML_DEVICE_NAME_BUFFER_SIZE];
            nvmlDeviceGetName( device, name, sizeof( name ) );
            data.name = name;

            uint32_t temp = 0;
            nvmlDeviceGetTemperature( device, NVML_TEMPERATURE_GPU, &temp );
            data.temperature = static_cast< float >( temp );
            
            result.push_back( data );
        }
    }
    return json{ result };
}

NvidiaMonitor::~NvidiaMonitor() 
{
    nvmlShutdown();
}
