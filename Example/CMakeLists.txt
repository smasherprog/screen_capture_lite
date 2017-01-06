project(Screen_Capture_Example)


if(WIN32)
    set(SYSTEM_LIBS)
elseif(APPLE)

find_library(corefoundation_lib CoreFoundation)
find_library(cocoa_lib Cocoa)
    set(SYSTEM_LIBS ${corefoundation_lib} ${cocoa_lib})

elseif(UNIX)
    set(SYSTEM_LIBS)
endif()



include_directories(${Screen_Capture_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} tiny_jpeg.h Screen_Capture_Example.cpp)
target_link_libraries(${PROJECT_NAME} Screen_Capturelib)
target_link_libraries(${PROJECT_NAME} ${SYSTEM_LIBS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)