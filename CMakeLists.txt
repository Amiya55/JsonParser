cmake_minimum_required(VERSION 3.28)

project(JsonParser VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(${PROJECT_NAME}
    ${SRC}
)

target_include_directories(JsonParser PUBLIC include/)