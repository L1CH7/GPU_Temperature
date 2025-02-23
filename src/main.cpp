#include <tests.h>

int main()
{
    // Choose your platform & device from `$ clinfo -l` (in my case platform_id refers to NVIDIA CUDA). 
    const size_t platform_id = 3;
    const size_t device_id = 0;
    std::string error;

    fs::path root_dir( WORKSPACE );
    ProgramHandler * handler = makeProgramHandler( platform_id, device_id, error );
    if( error != "")
        return 1;

    auto identity = initGpuModule( handler, root_dir / "src/fft/GpuKernels.cl" );

    {   // Single test run example. All datafiles must be in same directory(test_dir)
        fs::path test_dir = root_dir / "testcases/AM" / "000"; // /path/to/test/dir
        RunSingleTest( handler, test_dir );
    }

    {   // All testcases run example. 
        fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
        RunAllTests( handler, testcases );
    }
    delete handler;

    return 0;
}