cmake_minimum_required(VERSION 3.1)
project(Screen_Capture)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLE "Build example" On)

add_subdirectory(src)

if(BUILD_EXAMPLE)
  add_subdirectory(Example)
endif()
