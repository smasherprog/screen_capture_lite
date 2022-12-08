project(screen_capture_example_opengl_cpp)

if(WIN32)
	add_definitions(-DNOMINMAX)
endif()

if(NOT ${BUILD_SHARED_LIBS})
  if(WIN32)
	set(${PROJECT_NAME}_PLATFORM_LIBS Dwmapi)
  elseif(APPLE)
    find_package(Threads REQUIRED)
    find_library(corefoundation_lib CoreFoundation REQUIRED)
    find_library(cocoa_lib Cocoa REQUIRED)
    find_library(coremedia_lib CoreMedia REQUIRED)
    find_library(avfoundation_lib AVFoundation REQUIRED)
    find_library(coregraphics_lib CoreGraphics REQUIRED)
    find_library(corevideo_lib CoreVideo REQUIRED)
   
	set(${PROJECT_NAME}_PLATFORM_LIBS
        ${CMAKE_THREAD_LIBS_INIT}
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib}  
        ${corevideo_lib}
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
endif()

include_directories(
	glfw/deps
	glfw/include
	../include 
) 

set(GLAD_GL glfw/deps/glad/gl.h
            glfw/deps/glad_gl.c)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(USE_MSVC_RUNTIME_LIBRARY_DLL OFF) 

if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(glfw) 

add_executable(${PROJECT_NAME}_${TARGET_SUFFIX} 
	Screen_Capture_Example_OpenGL.cpp
	 ${GLAD_GL}
)

target_link_libraries(${PROJECT_NAME}_${TARGET_SUFFIX} screen_capture_lite_${TARGET_SUFFIX} ${${PROJECT_NAME}_PLATFORM_LIBS} glfw)  

install (TARGETS ${PROJECT_NAME}_${TARGET_SUFFIX} screen_capture_lite_${TARGET_SUFFIX}
	RUNTIME DESTINATION Examples
) 
