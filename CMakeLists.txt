cmake_minimum_required(VERSION 3.15)

project(gpu_temperature LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)


# find_package(OpenCL REQUIRED)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.87.0 COMPONENTS system) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    # add_executable(gpu_temperature file1.cxx file2.cxx) 
    # target_link_libraries(gpu_temperature ${Boost_LIBRARIES})
endif()


include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    include
    src
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

add_subdirectory(src)
