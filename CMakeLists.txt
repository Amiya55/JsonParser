cmake_minimum_required(VERSION 3.28)

project(JsonParser VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(${PROJECT_NAME}
    ${SRC}
)

target_include_directories(JsonParser PUBLIC include/)