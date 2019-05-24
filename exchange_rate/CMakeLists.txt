cmake_minimum_required(VERSION 2.8)

project(rate_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE "Debug")

include_directories(./include)

aux_source_directory(./src DIR_SRCS)

set(link_lib curl pthread event boost_system boost_program_options rdkafka rdkafka++)

add_executable(rate_server ${DIR_SRCS})

target_link_libraries(rate_server ${link_lib})
