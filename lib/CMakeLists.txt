project(Screen_Capturelib)

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
        ${Screen_Capture_SOURCE_DIR}/src/windows/GetMonitors.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/DXFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/GDIFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/GDIFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXCommon.h
		${Screen_Capture_SOURCE_DIR}/src/windows/DXCommon.cpp
        ${Screen_Capture_SOURCE_DIR}/src/windows/ThreadRunner.cpp
    )

    set(SCREEN_CAPTURE_PLATFORM_INC     
        ${Screen_Capture_SOURCE_DIR}/include/windows
        ${Screen_Capture_SOURCE_DIR}/src/windows
    )
elseif(APPLE)
    set(TARGET_BUILD_PLATFORM apple)
elseif(UNIX)
    set(TARGET_BUILD_PLATFORM unix)
endif()

set(SCREEN_CAPTURE_COMMON_SRC 
    ${Screen_Capture_SOURCE_DIR}/include/ScreenCapture.h
    ${Screen_Capture_SOURCE_DIR}/src/ScreenCapture.cpp
    ${Screen_Capture_SOURCE_DIR}/include/ThreadRunner.h
    ${Screen_Capture_SOURCE_DIR}/include/ThreadManager.h
    ${Screen_Capture_SOURCE_DIR}/src/ThreadManager.cpp
)
set(SCREEN_CAPTURE_COMMON_INC
    ${Screen_Capture_SOURCE_DIR}/include
    ${Screen_Capture_SOURCE_DIR}/src
)

include_directories(${SCREEN_CAPTURE_COMMON_INC} ${SCREEN_CAPTURE_PLATFORM_INC})

add_library(${PROJECT_NAME} ${SCREEN_CAPTURE_COMMON_SRC} ${SCREEN_CAPTURE_PLATFORM_SRC})

install(TARGETS ${PROJECT_NAME} 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

