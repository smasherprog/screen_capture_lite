project(Screen_Capturelib)

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
        ${Screen_Capture_SOURCE_DIR}/src/windows/GetMonitors.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/DXFrameProcessor.cpp
		${Screen_Capture_SOURCE_DIR}/include/windows/GDIHelpers.h
        ${Screen_Capture_SOURCE_DIR}/include/windows/GDIFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/GDIFrameProcessor.cpp
		${Screen_Capture_SOURCE_DIR}/include/windows/GDIMouseProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/GDIMouseProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/src/windows/ThreadRunner.cpp
    )

    set(SCREEN_CAPTURE_PLATFORM_INC     
        ${Screen_Capture_SOURCE_DIR}/include/windows
        ${Screen_Capture_SOURCE_DIR}/src/windows
    )

elseif(APPLE)
    set(SCREEN_CAPTURE_PLATFORM_SRC
        ${Screen_Capture_SOURCE_DIR}/include/ios/NSMouseCapture.h
        ${Screen_Capture_SOURCE_DIR}/src/ios/NSMouseCapture.m
        ${Screen_Capture_SOURCE_DIR}/include/ios/NSMouseProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/ios/NSMouseProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/include/ios/CGFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/ios/CGFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/src/ios/GetMonitors.cpp
        ${Screen_Capture_SOURCE_DIR}/src/ios/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC
        ${Screen_Capture_SOURCE_DIR}/include/ios
        ${Screen_Capture_SOURCE_DIR}/src/ios
    )
else()
  set(SCREEN_CAPTURE_PLATFORM_SRC
		${Screen_Capture_SOURCE_DIR}/include/linux/X11MouseProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/linux/X11MouseProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/include/linux/X11FrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/linux/X11FrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/src/linux/GetMonitors.cpp
        ${Screen_Capture_SOURCE_DIR}/src/linux/ThreadRunner.cpp
    )
  
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	set(SCREEN_CAPTURE_PLATFORM_INC
        ${Screen_Capture_SOURCE_DIR}/include/linux
        ${Screen_Capture_SOURCE_DIR}/src/linux
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
    ${Screen_Capture_SOURCE_DIR}/include/ScreenCapture.h
    ${Screen_Capture_SOURCE_DIR}/src/ScreenCapture.cpp
    ${Screen_Capture_SOURCE_DIR}/include/Internal/SCCommon.h
    ${Screen_Capture_SOURCE_DIR}/src/Internal/SCCommon.cpp
    ${Screen_Capture_SOURCE_DIR}/include/Internal/ThreadManager.h
    ${Screen_Capture_SOURCE_DIR}/src/Internal/ThreadManager.cpp
)
set(SCREEN_CAPTURE_COMMON_INC
    ${Screen_Capture_SOURCE_DIR}/include
    ${Screen_Capture_SOURCE_DIR}/src
    ${Screen_Capture_SOURCE_DIR}/include/Internal
    ${Screen_Capture_SOURCE_DIR}/src/Internal
)

include_directories(${SCREEN_CAPTURE_COMMON_INC} ${SCREEN_CAPTURE_PLATFORM_INC})

add_library(${PROJECT_NAME} ${SCREEN_CAPTURE_COMMON_SRC} ${SCREEN_CAPTURE_PLATFORM_SRC})
target_link_libraries(${PROJECT_NAME} ${SCREEN_CAPTURE_PLATFORM_LIBS})

install(TARGETS ${PROJECT_NAME} 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

