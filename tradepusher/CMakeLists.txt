cmake_minimum_required(VERSION 2.8)

project(tradepusher)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE "Debug")

include_directories(./include)

aux_source_directory(./src DIR_SRCS)

set(link_lib mysqlclient boost_system boost_program_options rdkafka rdkafka++)

add_executable(tradepusher ${DIR_SRCS})

target_link_libraries(tradepusher ${link_lib})
