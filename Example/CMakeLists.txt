project(Screen_Capture_Example)


if(WIN32)
    
elseif(APPLE)

else()
    

endif()


set(Screen_Capture_EXAMPLE_SRC 
	tiny_jpeg.h 
	lodepng.h 
	lodepng.cpp
	Screen_Capture_Example.cpp
)

include_directories(${Screen_Capture_INCLUDE_DIRS})
message(STATUS "info")
message(STATUS ${Screen_Capture_INCLUDE_DIRS})
message(STATUS ${Screen_Capture_LIB})
add_executable(${PROJECT_NAME} ${Screen_Capture_EXAMPLE_SRC})
target_link_libraries(${PROJECT_NAME} ${Screen_Capture_LIB})
target_link_libraries(${PROJECT_NAME} ${SYSTEM_LIBS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
