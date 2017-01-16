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

include_directories(${Screen_Capture_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${Screen_Capture_EXAMPLE_SRC})
target_link_libraries(${PROJECT_NAME} Screen_Capturelib)
target_link_libraries(${PROJECT_NAME} ${SYSTEM_LIBS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
