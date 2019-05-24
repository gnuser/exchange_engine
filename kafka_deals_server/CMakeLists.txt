cmake_minimum_required(VERSION 2.8)

project(kafka_deals_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE "Debug")
include_directories(./include)

add_executable(kafka_deals_server
			src/db_mysql.cpp
			src/common.cpp
			src/consumer_handler.cpp
			src/consumer.cpp
			src/main.cpp)
target_link_libraries(kafka_deals_server pthread mysqlclient rdkafka++ )

