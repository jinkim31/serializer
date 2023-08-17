cmake_minimum_required(VERSION 3.24)
project(serializer)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/json)
include_directories(lib/json/include)

add_library(serializer STATIC src/serializer.cpp)
target_link_libraries(serializer PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(serializer PUBLIC src)

add_executable(test test/main.cpp)
target_link_libraries(test PRIVATE serializer)
