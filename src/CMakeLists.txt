include_directories(
    monitoring
    network
    # io_json
    # gpu
    # fft
    # data_queue
    # execution_pool
    # helper
)

add_subdirectory(monitoring)

# add_subdirectory(gpu)
# add_subdirectory(fft)
# add_subdirectory(tests)
# add_subdirectory(execution_pool)
# add_subdirectory(helper)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    main.cxx
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    monitoring
    # io_json
    # gpu
    # fft
    # execution_pool
    # helper_functions
    # OpenCL::OpenCL
)
