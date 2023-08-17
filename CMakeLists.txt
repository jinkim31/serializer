cmake_minimum_required(VERSION 3.24)
project(sync)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/json)
include_directories(lib/json/include)

add_library(sync STATIC src/serializer.cpp)
target_link_libraries(sync PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(sync PUBLIC src)

add_executable(test test/main.cpp)
target_link_libraries(test PRIVATE sync)
