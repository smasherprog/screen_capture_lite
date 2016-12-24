project(Screen_Capture_Example)

include_directories(${Screen_Capture_SOURCE_DIR}/include)


add_executable(${PROJECT_NAME} tiny_jpeg.h Screen_Capture_Example.cpp)
target_link_libraries(${PROJECT_NAME} Screen_Capturelib)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)