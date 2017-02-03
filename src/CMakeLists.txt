project(libScreen_Capture)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows/GetMonitors.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/windows/DXFrameProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows/DXFrameProcessor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/../include/windows/GDIHelpers.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/windows/GDIFrameProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows/GDIFrameProcessor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/../include/windows/GDIMouseProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows/GDIMouseProcessor.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows/ThreadRunner.cpp
    )

    set(SCREEN_CAPTURE_PLATFORM_INC     
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/windows
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/windows
    )

elseif(APPLE)

find_library(corefoundation_lib CoreFoundation)
find_library(cocoa_lib Cocoa)
set(SCREEN_CAPTURE_PLATFORM_LIBS ${corefoundation_lib} ${cocoa_lib})

    set(SCREEN_CAPTURE_PLATFORM_SRC
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/ios/NSMouseCapture.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/ios/NSMouseCapture.m
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/ios/NSMouseProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/ios/NSMouseProcessor.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/ios/CGFrameProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/ios/CGFrameProcessor.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/ios/GetMonitors.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/ios/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/ios
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/ios
    )
else()
  set(SCREEN_CAPTURE_PLATFORM_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/../include/linux/X11MouseProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/linux/X11MouseProcessor.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/linux/X11FrameProcessor.h
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/linux/X11FrameProcessor.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/linux/GetMonitors.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/linux/ThreadRunner.cpp
    )
  
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	set(SCREEN_CAPTURE_PLATFORM_INC
       ${CMAKE_CURRENT_SOURCE_DIR}/../include/linux
       ${CMAKE_CURRENT_SOURCE_DIR}/../src/linux
		${X11_INCLUDE_DIR}
    )
	find_package(Threads REQUIRED)
	set(SCREEN_CAPTURE_PLATFORM_LIBS
		${X11_LIBRARIES}
		${X11_Xfixes_LIB}
		${X11_XTest_LIB}
		${CMAKE_THREAD_LIBS_INIT}
	)
	
endif()

set(SCREEN_CAPTURE_COMMON_SRC 
   ${CMAKE_CURRENT_SOURCE_DIR}/../include/ScreenCapture.h
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/ScreenCapture.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../include/Internal/SCCommon.h
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/Internal/SCCommon.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../include/Internal/ThreadManager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/Internal/ThreadManager.cpp
)
set(SCREEN_CAPTURE_LIB
	${PROJECT_NAME}
	CACHE INTERNAL "${PROJECT_NAME}: Include Libraries" FORCE
)

set(SCREEN_CAPTURE_INCLUDE_DIRS 
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../include/internal
	${CMAKE_CURRENT_SOURCE_DIR}/../src/internal
	${SCREEN_CAPTURE_PLATFORM_INC}
	CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
 )

include_directories(${SCREEN_CAPTURE_INCLUDE_DIRS})
add_library(${PROJECT_NAME} ${SCREEN_CAPTURE_COMMON_SRC} ${SCREEN_CAPTURE_PLATFORM_SRC})

target_link_libraries(${PROJECT_NAME} ${SCREEN_CAPTURE_PLATFORM_LIBS})


