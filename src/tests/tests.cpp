#include "tests.h"
#include <JsonReadDataQueue.h>
#include <JsonWriteDataQueue.h>
#include <ExecutionPool.h>
#include <error.h>


void testExecutionPool( std::shared_ptr< ProgramHandler > handler, fs::path root_dir, size_t read_thread_num, size_t computing_thread_num, size_t write_thread_num )
{
    fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
    ReadPathsTemplate data_paths_template{
        .params_path = "in_args.json",
        .mseq_path = "tfpMSeqSigns.json",
        .data_path = "out.json",
        .ftps_path = "ftps.json"
    };
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t( now );

    std::stringstream ss;
    ss << "report_" << std::put_time( std::localtime( &in_time_t ), "%Y-%m-%d_%X" );
    fs::path report_root_dir = root_dir / "reports" / ss.str();
    fs::path report_dir = report_root_dir / "times";
    if( !fs::exists( report_dir ) ) fs::create_directories( report_dir );
    fs::path result_dir = report_root_dir / "data";
    if( !fs::exists( result_dir ) ) fs::create_directories( result_dir );
    FftReport reports_template{
        .report_dir = report_dir,
        .result_dir = result_dir,
    };
    // auto rdq = std::make_shared< JsonReadDataQueue >( read_thread_num, testcases, data_paths_template );
    std::vector< fs::path > testcases_v;
    // testcases_v.push_back( testcases / "021");
    // testcases_v.push_back( testcases / "022");
    // testcases_v.push_back( testcases / "023");
    // testcases_v.push_back( testcases / "006");
    testcases_v.push_back( testcases / "005");
    // testcases_v.push_back( testcases / "000");
    auto rdq = std::make_shared< JsonReadDataQueue >( read_thread_num, testcases_v, data_paths_template );
    auto wdq = std::make_shared< JsonWriteDataQueue >( write_thread_num );
    ExecutionPool exec_pool{ handler, rdq, wdq, reports_template, computing_thread_num };

    rdq->startReading();
    // rdq->startReadingSplitKGRS( 14 );
    rdq->wait();
    exec_pool.execute();
}

// void RunAllTests( std::shared_ptr< ProgramHandler > handler, fs::path root_dir, size_t read_thread_num = 1, size_t computing_thread_num = 1, size_t write_thread_num = 1 )
void RunAllTests( std::shared_ptr< ProgramHandler > handler, fs::path root_dir, size_t read_thread_num, size_t computing_thread_num, size_t write_thread_num )
{
    // fs::path testcases = root_dir / "testcases/FM_copies"; // /path/to/all/testcases/dir
    fs::path testcases = root_dir / "testcases/FM"; // /path/to/all/testcases/dir
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t( now );

    std::stringstream ss;
    ss << "report_" << std::put_time( std::localtime( &in_time_t ), "%Y-%m-%d_%X" );
    fs::path report_root_dir = root_dir / "reports" / ss.str();
    fs::path report_dir = report_root_dir / "times";
    if( !fs::exists( report_dir ) )
        fs::create_directories( report_dir );
    fs::path result_dir = report_root_dir / "data";
    if( !fs::exists( result_dir ) )
        fs::create_directories( result_dir );


    ReadPathsTemplate data_paths{
        .params_path = "in_args.json",
        .mseq_path = "tfpMSeqSigns.json",
        .data_path = "out.json"
    };

    JsonReadDataQueue rdq( read_thread_num, testcases, data_paths );
    // std::vector< fs::path > testcases_v;
    // testcases_v.push_back( testcases / "005");
    // testcases_v.push_back( testcases / "006");
    // // testcases_v.push_back( testcases / "003");
    // JsonReadDataQueue rdq( read_thread_num, testcases_v, data_paths );
    rdq.startReading();
    rdq.wait(); // wait for all tasks to be readed!
    std::cout << warn_str("end reading!\n");

    JsonWriteDataQueue wdq( read_thread_num );

    std::vector< std::shared_ptr < FftCreator > > fft_instances( computing_thread_num, nullptr );
    BS::light_thread_pool pool( 
        computing_thread_num,
        [handler, &fft_instances]( std::size_t thread_id ){ 
            fft_instances.at( thread_id ) = std::make_shared< FftCreator >( handler ); 
        }
    );

    // for( size_t i = 0; i < rdq.size(); ++i )
    // for( size_t i = 0; !rdq.finish(); ++i )
    size_t task_index = 0;
    // can read data if:
    //  rdq is stopped but not empty
    //  rdq empty but not stopped (case of not waiting for ackquiring data)
    // resume: read if true: stop xor empty (or stop != empty)
    while( rdq.stopped() != rdq.empty() )
    {
        FftReport report{
            .report_dir = report_dir,
            .result_dir = result_dir,
            .task_index = task_index
        };
        // report.time.cpu_start_point = in_time_t;
        auto task = [&rdq, &wdq, &fft_instances, report ]() mutable
        {
            // size_t thread_id = BS::this_thread::get_index().value_or(0);
            size_t thread_id = BS::this_thread::get_index().value_or(-1);
            if( thread_id > fft_instances.size() )
            {
                std::cerr << error_str("Invalid thread_id!\n");
                return;
            }
            try
            {
                auto data = rdq.pop();
                if( !data )
                    return; // acquired empty data!

                FftParams params = data->params;
                uint8_t polar = data->polar;
                fs::path data_path = data->data_path;
                fft_instances[thread_id]->load( *data );
                auto time_start = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
                auto time = fft_instances[thread_id]->compute();

                auto time_end = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

                time.cpu_start_point = time_start / 1000.;
                time.cpu_end_point = time_end / 1000.;
                auto out_array = fft_instances[thread_id]->getFftResult();

                report.data_path = data_path;
                report.out_array = std::move( out_array );
                report.polar = polar;
                report.params = params;
                report.time = time;
                wdq.writeData( std::move( report ) );
            }
            catch( const cl::Error & e )
            {
                std::stringstream ss;
                ss << error_str( e.what() ) << std::endl;
                ss << error_str( getErrorString( e.err() ) ) << std::endl;
                ss << error_str("test#") << report.params.test_name << std::endl;
                std::cerr << ss.str();
                throw e;
            }
            catch( const std::exception & e )
            {
                std::cerr << error_str(e.what()) <<std::endl;
                throw e;
            }
        };
        pool.detach_task( task );
        ++task_index;
    }
    if( rdq.stopped() && rdq.empty() )
        pool.purge();
    pool.wait();
    std::cout << focus_str("end computing\n");

}
