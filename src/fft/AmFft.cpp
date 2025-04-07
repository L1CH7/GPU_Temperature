#include <GpuFourier.h>

#include <config.h>

AmFft::AmFft( std::shared_ptr< ProgramHandler > handler, FftData & data ) 
:   FftInterface( handler, data )
{
    std::cout<<"AM FFT instance c-tor!\n";
}

TimeResult
AmFft::compute()
{
    const uint32_t sin_arr_len = 1 << 19; // 2^19 = 524288
    uint32_t N = 1 << params_.log2N;

    uint32_t group_log2 = 8;
    if ((params_.log2N - 1) < group_log2)
        group_log2 = params_.log2N - 1;
    uint32_t group_size = 1 << group_log2;

    cl_command_queue_properties queue_properties = CL_QUEUE_PROFILING_ENABLE;

    cl::Buffer sinsBuffer(
        *( handler_->context ), 
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        sin_arr_len * sizeof( cl_float )
    );
        

    cl::Buffer inFFTBuffer(
        *( handler_->context ), 
        CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
        params_.nl * params_.samples_num * sizeof( cl_int2 )
    );


    cl::Buffer midFFTBuffer(
        *( handler_->context ), 
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        params_.nl * params_.kgd * N * sizeof( cl_float2 )
    );


    cl::Buffer outFFTBuffer(
        *( handler_->context ), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
        params_.nl * params_.kgrs * params_.kgd * sizeof( cl_float2 )
    );


    cl::CommandQueue queue(
        *( handler_->context ), *( handler_->device ), queue_properties, NULL 
    );

    cl::Event event_array[6];
    cl_int error = CL_SUCCESS;

    queue.enqueueWriteBuffer(
        inFFTBuffer, CL_TRUE, 0,
        params_.nl * params_.samples_num * sizeof( cl_int2 ),
        data_array_.data(), NULL, &event_array[0]
    );

    auto sins_kernel_functor = cl::KernelFunctor< cl::Buffer >{ *( handler_->program ), "getSinArrayTwoPi_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs sins_eargs{ queue, cl::NullRange, cl::NDRange( sin_arr_len ), cl::NullRange };
    event_array[1] = sins_kernel_functor( sins_eargs, sinsBuffer, error );

    auto fft_prep_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_int, cl_uint, cl_uint, cl_uint >{ 
        *( handler_->program ), "FFTPrep_F" 
    };
    error = CL_SUCCESS;
    cl::EnqueueArgs fft_prep_eargs{ queue, cl::NullRange, cl::NDRange( N, params_.kgd, params_.nl ), cl::NDRange( group_size, 1, 1 ) };
    event_array[2] = fft_prep_kernel_functor( 
        fft_prep_eargs, inFFTBuffer, midFFTBuffer, 
        params_.log2N, 
        params_.samples_num, 
        params_.true_nihs, 
        params_.nfgd_fu, 
        params_.shgd, 
        params_.ndec, 
        params_.dlstr, 
        error 
    );

    auto fft_fft_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_uint >{ *( handler_->program ), "FFT_FFT_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs fft_fft_eargs{ queue, cl::NullRange, cl::NDRange( group_size, params_.kgd, params_.nl ), cl::NDRange( group_size, 1, 1 ) };
    event_array[3] = fft_fft_kernel_functor( 
        fft_fft_eargs, midFFTBuffer, sinsBuffer, params_.log2N, group_log2,
        error 
    );
        
    auto fft_post_kernel_functor = cl::KernelFunctor< cl::Buffer, cl::Buffer, cl_uint, cl_int >{ *( handler_->program ), "FFTPost_F" };
    error = CL_SUCCESS;
    cl::EnqueueArgs fft_post_eargs{ queue, cl::NullRange, cl::NDRange( params_.kgrs, params_.kgd, params_.nl ), cl::NullRange };
    event_array[4] = fft_post_kernel_functor( 
        fft_post_eargs, midFFTBuffer, outFFTBuffer, params_.log2N, params_.n1grs,
        error 
    );

    queue.enqueueReadBuffer(
        outFFTBuffer, CL_TRUE, 0,
        params_.nl * params_.kgd * params_.kgrs * sizeof(cl_float2), 
        out_array_.data(),
        NULL, &event_array[5]);

    queue.finish();

    // RAM cleanup
    std::vector< std::complex< int > >().swap( data_array_ ); 

    const std::scoped_lock< std::mutex > l( m_ );
    TimeResult time = {
        .write_start            = event_array[0].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .write_end              = event_array[0].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .sine_computation_start = event_array[1].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .sine_computation_end   = event_array[1].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .fm_sign_fft_start      = 0,
        .fm_sign_fft_end        = 0,
        .fm_data_fft_start      = 0,
        .fm_data_fft_end        = 0,
        .fft_start              = event_array[2].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .fft_end                = event_array[4].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
        .read_start             = event_array[5].getProfilingInfo<CL_PROFILING_COMMAND_START>(),
        .read_end               = event_array[5].getProfilingInfo<CL_PROFILING_COMMAND_END>(),
    };
    return time;
}
