cmake_minimum_required(VERSION 3.8)
project(screen_capture_lite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(BUILD_SHARED_LIBS "Build shared library" OFF) 
option(BUILD_EXAMPLE "Build example" ON)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
		src/windows/GetWindows.cpp
		src/windows/GetMonitors.cpp 
		src/windows/DXFrameProcessor.cpp 
		src/windows/GDIFrameProcessor.cpp 
		src/windows/GDIMouseProcessor.cpp
		src/windows/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC     
       include/windows 
    )
elseif(APPLE)
	find_library(corefoundation_lib CoreFoundation)
	find_library(cocoa_lib Cocoa)
    set(SCREEN_CAPTURE_PLATFORM_SRC
        src/ios/GetWindows.cpp 
        src/ios/NSMouseCapture.m 
        src/ios/NSMouseProcessor.cpp 
        src/ios/CGFrameProcessor.cpp
        src/ios/GetMonitors.cpp
        src/ios/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC
       include/ios 
    )
else()
  set(SCREEN_CAPTURE_PLATFORM_SRC 
       src/linux/X11MouseProcessor.cpp 
       src/linux/X11FrameProcessor.cpp
       src/linux/GetMonitors.cpp
       src/linux/GetWindows.cpp
       src/linux/ThreadRunner.cpp
    )
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	set(SCREEN_CAPTURE_PLATFORM_INC
       include/linux 
		${X11_INCLUDE_DIR}
    )
endif()


include_directories(
	include 
	${SCREEN_CAPTURE_PLATFORM_INC} 
)
add_library(${PROJECT_NAME} 
	include/ScreenCapture.h 
	include/SCCommon.h 
	include/ThreadManager.h 
	src/ScreenCapture.cpp 
	src/SCCommon.cpp 
	src/ThreadManager.cpp
	${SCREEN_CAPTURE_PLATFORM_SRC}
 )
 if(${BUILD_SHARED_LIBS})	
	set_target_properties(${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL SC_LITE_DLL)
	 if(WIN32) 
		target_link_libraries(${PROJECT_NAME} ${COMMON_LIBRARIES}  Dwmapi)
		install (FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
	elseif(APPLE)
		find_package(Threads REQUIRED)
		find_library(corefoundation_lib CoreFoundation)
		find_library(cocoa_lib Cocoa)
		target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${corefoundation_lib} ${cocoa_lib})
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
		target_link_libraries(${PROJECT_NAME} ${COMMON_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} dl)
	endif()
endif()

install (TARGETS ${PROJECT_NAME}  
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
)

install (FILES 
	include/ScreenCapture.h 
	DESTINATION include
)

if(BUILD_EXAMPLE)
  add_subdirectory(Example)
endif()
