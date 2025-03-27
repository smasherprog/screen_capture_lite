project(screen_capture_lite)

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
		windows/GetWindows.cpp
		windows/GetMonitors.cpp 
		windows/DXFrameProcessor.cpp 
		../include/windows/DXFrameProcessor.h
		windows/GDIFrameProcessor.cpp 
		../include/windows/GDIFrameProcessor.h
		windows/GDIMouseProcessor.cpp
		../include/windows/GDIMouseProcessor.h
		windows/ThreadRunner.cpp
		../include/windows/GDIHelpers.h
    )
    set(SCREEN_CAPTURE_PLATFORM_INC     
       ../include/windows 
    )
	add_definitions(-DNOMINMAX)
	link_libraries(dxgi.lib d3d11.lib)
elseif(APPLE)

    set(SCREEN_CAPTURE_PLATFORM_SRC
        ios/GetWindows.cpp 
        ios/NSMouseCapture.m
        ../include/ios/NSMouseCapture.h 
        ios/NSFrameProcessor.cpp
        ios/NSFrameProcessor.mm
        ../include/ios/NSFrameProcessorm.h
        ../include/ios/NSFrameProcessor.h
        ios/NSMouseProcessor.cpp
        ../include/ios/NSMouseProcessor.h
        ios/CGFrameProcessor.cpp
        ../include/ios/CGFrameProcessor.h
        ios/GetMonitors.cpp
        ios/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC
       ../include/ios 
    )
else()
  set(SCREEN_CAPTURE_PLATFORM_SRC 
       ../include/linux/X11MouseProcessor.h 
       linux/X11MouseProcessor.cpp 
       ../include/linux/X11FrameProcessor.h 
       linux/X11FrameProcessor.cpp
       linux/GetMonitors.cpp
       linux/GetWindows.cpp
       linux/ThreadRunner.cpp
    )
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	set(SCREEN_CAPTURE_PLATFORM_INC
       ../include/linux 
		${X11_INCLUDE_DIR}
    )
endif()


include_directories(
	../include 
	${SCREEN_CAPTURE_PLATFORM_INC}
)

set(libsrc 
	../include/ScreenCapture.h 
		../include/internal/SCCommon.h 
		../include/internal/ThreadManager.h
		ScreenCapture.c
		ScreenCapture.cpp
		SCCommon.cpp
		ThreadManager.cpp
		${SCREEN_CAPTURE_PLATFORM_SRC}
)

if(NOT ${BUILD_SHARED_LIBS})
    message("Building STATIC Library")
    add_library(${PROJECT_NAME}_static STATIC ${libsrc})
		target_link_libraries(${PROJECT_NAME}_static Dwmapi)
else()
  message("Building SHARED Library")
  
    add_library(${PROJECT_NAME}_shared SHARED ${libsrc} ../include/ScreenCapture_C_API.h)

	add_definitions(-DSC_LITE_DLL)
	 if(WIN32) 
		target_link_libraries(${PROJECT_NAME}_shared Dwmapi)
		if (!MINGW)
			install (FILES $<TARGET_PDB_FILE:${PROJECT_NAME}_shared> DESTINATION bin OPTIONAL)
		endif()
	elseif(APPLE)
		find_package(Threads REQUIRED)
        find_library(corefoundation_lib CoreFoundation REQUIRED)
        find_library(cocoa_lib Cocoa REQUIRED)
        find_library(coremedia_lib CoreMedia REQUIRED)
        find_library(avfoundation_lib AVFoundation REQUIRED)
        find_library(coregraphics_lib CoreGraphics REQUIRED)
        find_library(corevideo_lib CoreVideo REQUIRED)
       
        target_link_libraries(
           ${PROJECT_NAME}_shared
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
		target_link_libraries(
            ${PROJECT_NAME}_shared
			${X11_LIBRARIES}
			${X11_Xfixes_LIB}
			${X11_XTest_LIB}
			${X11_Xinerama_LIB}
			${CMAKE_THREAD_LIBS_INIT}
		)	 
	endif()
endif()  
