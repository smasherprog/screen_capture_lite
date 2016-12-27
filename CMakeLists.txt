cmake_minimum_required(VERSION 3.0)
project(Screen_Capture)

option(BUILD_EXAMPLE "Build example" On)

 add_subdirectory(lib)

if(BUILD_EXAMPLE)
  add_subdirectory(Example)
endif()