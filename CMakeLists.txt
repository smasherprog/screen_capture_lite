cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Screen_Capture)

option(BUILD_EXAMPLE "Build example" On)

 add_subdirectory(lib)

if(BUILD_EXAMPLE)
  add_subdirectory(Example)
endif()