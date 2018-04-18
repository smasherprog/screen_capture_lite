project(screen_capture_example)

if(WIN32)
	set(${PROJECT_NAME}_PLATFORM_LIBS Dwmapi)
elseif(APPLE)
	find_library(corefoundation_lib CoreFoundation)
	find_library(cocoa_lib Cocoa)
    find_library(coremedia_lib CoreMedia)
    find_library(avfoundation_lib AVFoundation)
    find_library(coregraphics_lib CoreGraphics) 

	set(${PROJECT_NAME}_PLATFORM_LIBS
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib} 
    )

else()
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	find_package(Threads REQUIRED)
	set(${PROJECT_NAME}_PLATFORM_LIBS
		${X11_LIBRARIES}
		${X11_Xfixes_LIB}
		${X11_XTest_LIB}
		${X11_Xinerama_LIB}
		${CMAKE_THREAD_LIBS_INIT}
	)
endif()

add_executable(${PROJECT_NAME}  
	lodepng.cpp
	Screen_Capture_Example.cpp
)
target_link_libraries(${PROJECT_NAME} screen_capture_lite ${${PROJECT_NAME}_PLATFORM_LIBS}) 

